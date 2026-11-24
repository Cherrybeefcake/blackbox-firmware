// SPDX-License-Identifier: AGPL-3.0-or-later
#include "subghz.h"
#include <Arduino.h>
namespace rf {
Diagnostics diagnostics;
namespace {
// Software SPI mode 0, MSB first. Bus is reserved exclusively by Modules.
constexpr int cs = 5, clk = 40, so = 39, si = 14, power = 3, sw = 13, nfc = 6, sd = 12;
void output(int pin, int level) {
    digitalWrite(pin, level);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, level);
}
uint8_t transfer(uint8_t data) {
    uint8_t in = 0;
    for (int bit = 7; bit >= 0; --bit) {
        digitalWrite(si, (data >> bit) & 1);
        delayMicroseconds(1);
        digitalWrite(clk, HIGH);
        in = (in << 1) | digitalRead(so);
        delayMicroseconds(1);
        digitalWrite(clk, LOW);
    }
    return in;
}
bool ready() {
    unsigned began = micros();
    while (digitalRead(so)) {
        if (micros() - began >= 2000) {
            ++diagnostics.readyTimeouts;
            return false;
        }
    }
    return true;
}
bool command(uint8_t header, int data = -1, uint8_t *out = nullptr) {
    digitalWrite(cs, LOW);
    if (!ready()) {
        digitalWrite(cs, HIGH);
        return false;
    }
    transfer(header);
    if (out) *out = transfer(0);
    else if (data >= 0) transfer(data);
    bool complete = header != 0x30 || ready();
    digitalWrite(cs, HIGH);
    return complete;
}
bool tune(unsigned khz) {
    if (!valid(khz)) return false;
    unsigned word = (uint64_t(khz) * 65536 + 13000) / 26000;
    return command(0x36) && command(0x0d, word >> 16) && command(0x0e, (word >> 8) & 255) &&
           command(0x0f, word & 255) && command(0x34);
}
bool read(int &value) {
    uint8_t state, raw;
    if (!command(0xf5, -1, &state) || (state & 31) != 13 || !command(0xf4, -1, &raw)) return false;
    value = int(int8_t(raw)) / 2 - 74;
    return true;
}
State open() {
    State state = modules.acquire(moduleIndex, Owner::Application);
    if (state != State::Active) ++diagnostics.failures;
    return state;
}
void close() { modules.release(moduleIndex, Owner::Application); }
} // namespace
unsigned clockMs() { return millis(); }
State initChip() {
    if (!confirmed) return State::Unavailable;
    diagnostics.identityRead = false;
    output(power, LOW);
    output(nfc, HIGH);
    output(sd, HIGH);
    output(cs, HIGH);
    output(clk, LOW);
    output(si, LOW);
    output(sw, HIGH);
    pinMode(so, INPUT);
    delay(5);
    digitalWrite(power, HIGH);
    delay(5);
    uint8_t part, rev;
    if (!command(0xf0, -1, &part) || !command(0xf1, -1, &rev)) return State::Unavailable;
    diagnostics.identityRead = true;
    diagnostics.part = part;
    diagnostics.revision = rev;
    if (part != 0 || rev != 0x14) return State::Unavailable;
    // Reset only a recognized chip, then wait for its crystal before configuring RX.
    if (!command(0x30)) return State::Error;
    delay(1);
    // GDO2 drives SW1 low. Asynchronous mode bypasses packet FIFO; GDO0 is high impedance.
    // 26 MHz crystal, 101.6 kHz RX bandwidth, calibrate on IDLE -> RX.
    return command(0x36) && command(0x00, 0x2f) && command(0x02, 0x2e) && command(0x08, 0x30) &&
                   command(0x12, 0) && command(0x10, 0xc9) && command(0x18, 0x18)
               ? State::Available
               : State::Error;
}
void powerDown() {
    if (!confirmed) return;
    if (diagnostics.identityRead && diagnostics.part == 0 && diagnostics.revision == 0x14) {
        if (command(0x36)) command(0x39); // IDLE, then sleep on CS release.
    }
    digitalWrite(power, LOW);
    for (int pin : {clk, si, so, cs, nfc, sd, sw, power}) pinMode(pin, INPUT);
}
const Backend physical{
    open,
    [](unsigned khz) {
        if (tune(khz)) return true;
        ++diagnostics.failures;
        return false;
    },
    [](int &value) {
        if (read(value)) return true;
        ++diagnostics.failures;
        return false;
    },
    close
};
} // namespace rf
