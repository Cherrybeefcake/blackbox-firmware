// SPDX-License-Identifier: AGPL-3.0-or-later
// Drive production transports through deterministic ST register and RMT fixtures.
#include "lab.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <cassert>
#include <cstring>
#include <driver/rmt.h>
#include <vector>
namespace {
unsigned clockMs, op, cursor;
int writes[48]{}, pins[48]{}, spiStops, i2cStops, rmtStops, resets;
bool identified = true, clockStable = true, irBusy, irFail, waiting, restricted;
std::vector<uint8_t> bytes, fifo, frame;
uint8_t value(unsigned offset) {
    switch (op) {
        case 0x7f: return identified ? 0x29 : 0xff;
        case 0x71: return clockStable ? 0x10 : 0;
        case 0x5a: return !waiting && !offset ? 0x10 : 0;
        case 0x5e: return offset ? 0 : frame.size();
        case 0x9f: return frame.at(offset);
        case 0x42: return 0;
        default: return 0;
    }
}
void complete() {
    if (op == 0x80) fifo = bytes;
    if (op == 0xc6) frame = {4, 0};
    if (op == 0xc5) {
        assert((fifo == std::vector<uint8_t>{0x93, 0x20}));
        frame = {1, 2, 3, 4, 4};
    }
    if (op == 0xc4) {
        if (fifo[0] == 0x93) {
            assert(fifo[1] == 0x70);
            frame = {0, 0, 0};
        } else {
            assert(fifo.size() == 2 && fifo[0] == 0x30);
            frame.assign(18, 0);
            if (!fifo[1]) {
                frame[12] = 0xe1;
                frame[13] = 0x10;
                frame[14] = 18;
                frame[15] = restricted ? 0xf0 : 0xf;
            } else {
                assert(!restricted && fifo[1] == 4);
                uint8_t ndef[] = {3, 6, 0xd1, 1, 2, 'U', 4, 'x'};
                memcpy(frame.data(), ndef, sizeof ndef);
            }
        }
    }
}
} // namespace
TestSPI SPI;
TestWire Wire;
void digitalWrite(int p, int v) { writes[p] = v; }
void pinMode(int p, int v) { pins[p] = v; }
unsigned millis() { return clockMs; }
void delay(unsigned ms) { clockMs += ms; }
void TestSPI::begin(int c, int i, int o, int s) { assert(c == 40 && i == 39 && o == 14 && s == 6); }
void TestSPI::beginTransaction(SPISettings) {
    cursor = 0;
    bytes.clear();
    assert(writes[5] && writes[12] && writes[3]);
}
uint8_t TestSPI::transfer(uint8_t v) {
    assert(!writes[6]);
    if (!cursor++) {
        op = v;
        return 0;
    }
    bytes.push_back(v);
    return value(cursor - 2);
}
void TestSPI::endTransaction() {
    assert(writes[6]);
    complete();
}
void TestSPI::end() { ++spiStops; }
bool TestWire::begin(int d, int c, int hz) {
    assert(d == 2 && c == 1 && hz == 100000);
    return true;
}
void TestWire::setTimeOut(unsigned n) { assert(n <= 5); }
void TestWire::beginTransmission(uint8_t a) {
    assert(a == 0x50);
    bytes.clear();
    cursor = 0;
}
void TestWire::write(uint8_t byte) { op = byte; }
void TestWire::write(const uint8_t *p, size_t n) { bytes.assign(p, p + n); }
int TestWire::endTransmission(bool) {
    complete();
    return 0;
}
size_t TestWire::requestFrom(uint8_t a, size_t n) {
    assert(a == 0x50);
    return n;
}
int TestWire::read() { return value(cursor++); }
void TestWire::end() { ++i2cStops; }
int rmt_driver_install(int ch, int, int) {
    assert(!ch);
    return irBusy ? -1 : 0;
}
int rmt_config(const rmt_config_t *c) {
    assert(
        c->pin == 44 && c->clk_div == 80 && c->tx_config.carrier_en &&
        c->tx_config.carrier_freq_hz == 38000 && c->tx_config.carrier_duty_percent == 33
    );
    return irFail ? -1 : 0;
}
int rmt_write_items(int ch, const rmt_item32_t *p, int n, bool wait) {
    assert(
        !ch && !wait && n == 1 && p->level0 == 1 && p->duration0 == 1000 && !p->level1 && p->duration1 == 1000
    );
    return 0;
}
int rmt_wait_tx_done(int, int wait) {
    assert(!wait);
    return 0;
}
void rmt_tx_stop(int) { ++rmtStops; }
void rmt_driver_uninstall(int) {}
void gpio_reset_pin(int pin) {
    assert(pin == 44);
    ++resets;
}
int main() {
    using namespace lab;
    for (bool cap : {true, false}) {
        assert((cap ? capStart() : groveStart()) == State::Available);
        Tag tag;
        for (int i = 0; i < 4; ++i) assert(pollTag(tag) == Phase::Running);
        assert(
            pollTag(tag) == Phase::Complete && tag.count == 1 && !strcmp(tag.records[0].text, "https://x")
        );
        readerStop();
    }
    assert(spiStops == 1 && i2cStops == 1 && !writes[3] && pins[6] == INPUT);
    restricted = true;
    assert(groveStart() == State::Available);
    Tag tag;
    for (int i = 0; i < 3; ++i) assert(pollTag(tag) == Phase::Running);
    assert(pollTag(tag) == Phase::Unsupported && !tag.count);
    readerStop();
    identified = false;
    assert(capStart() == State::Unavailable);
    readerStop();
    identified = true;
    clockStable = false;
    unsigned began = millis();
    assert(groveStart() == State::Error && millis() - began == 15);
    readerStop();
    irBusy = true;
    assert(emitterStart() == State::Error);
    emitterStop();
    assert(!resets);
    irBusy = false;
    irFail = true;
    assert(emitterStart() == State::Error);
    emitterStop();
    assert(resets == 1);
    irFail = false;
    assert(emitterStart() == State::Available && burstFinished());
    emitterStop();
    assert(resets == 2 && rmtStops == 2 && !burstFinished());
}
