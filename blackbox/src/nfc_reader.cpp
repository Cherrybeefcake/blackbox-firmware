// SPDX-License-Identifier: AGPL-3.0-or-later
// ST25R3916 register definitions and SPI command framing: ST DS12484, sections 4-5.
// Only NFC-A single-tag selection and Type 2 READ have callers in this transport.
#include "lab.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <cstring>
namespace lab {
namespace {
enum class Step { Request, Anticollision, Select, Capability, Data };
struct Reader {
    bool cap = false, bus = false, identified = false;
    uint8_t cascade = 0x93;
    Step step = Step::Request;
    unsigned sent = 0;
    uint8_t data[maxMessage + 16]{};
    size_t filled = 0;
} reader;
ReaderStatus statusByBus[2];
constexpr int cs = 6, power = 3;
constexpr uint8_t address = 0x50;
bool io(uint8_t op, const uint8_t *tx, size_t nt, uint8_t *rx, size_t nr) {
    if (reader.cap) {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
        digitalWrite(cs, LOW);
        SPI.transfer(op);
        for (size_t i = 0; i < nt; ++i) SPI.transfer(tx[i]);
        for (size_t i = 0; i < nr; ++i) rx[i] = SPI.transfer(0);
        digitalWrite(cs, HIGH);
        SPI.endTransaction();
        return true;
    }
    Wire.beginTransmission(address);
    Wire.write(op);
    if (nt) Wire.write(tx, nt);
    if (Wire.endTransmission(nr == 0) != 0) return false;
    if (nr && Wire.requestFrom(address, nr) != nr) return false;
    for (size_t i = 0; i < nr; ++i) rx[i] = Wire.read();
    return true;
}
bool cmd(uint8_t op) { return io(op, nullptr, 0, nullptr, 0); }
bool reg(uint8_t addr, uint8_t v) { return io(addr, &v, 1, nullptr, 0); }
bool fetch(uint8_t addr, uint8_t *p, size_t n = 1) { return io(0x40 | addr, nullptr, 0, p, n); }
bool request(Step step, const uint8_t *tx = nullptr, unsigned n = 0) {
    bool crc = step != Step::Request && step != Step::Anticollision;
    reader.step = step;
    if (!cmd(0xc2) || !reg(0x05, step == Step::Anticollision ? 1 : 0) || !reg(0x0a, crc ? 0 : 0x80) ||
        !cmd(0xdb))
        return false;
    if (n && (!reg(0x22, 0) || !reg(0x23, n * 8) || !io(0x80, tx, n, nullptr, 0))) return false;
    reader.sent = millis();
    return cmd(step == Step::Request ? 0xc6 : crc ? 0xc4 : 0xc5);
}
State begin(bool cap) {
    reader = Reader{};
    reader.cap = cap;
    statusByBus[cap].chip = -1;
    if (cap) {
        for (int pin : {5, 6, 12}) {
            digitalWrite(pin, HIGH);
            pinMode(pin, OUTPUT);
            digitalWrite(pin, HIGH);
        }
        pinMode(power, OUTPUT);
        digitalWrite(power, HIGH);
        delay(5);
        SPI.begin(40, 39, 14, cs);
        reader.bus = true;
    } else {
        reader.bus = Wire.begin(2, 1, 100000);
        if (!reader.bus) return State::Error;
        Wire.setTimeOut(5);
    }
    uint8_t identity;
    if (!fetch(0x3f, &identity) || (identity >> 3) != 5) return State::Unavailable;
    statusByBus[cap].chip = identity;
    reader.identified = true;
    if (!cmd(0xc1) || !reg(1, 0x80) || !reg(2, 0x80)) return State::Error;
    unsigned began = millis();
    uint8_t status;
    do {
        if (!fetch(0x31, &status)) return State::Error;
        if (status & 0x10) break;
        delay(1);
    } while (millis() - began < 15);
    if (!(status & 0x10) || !reg(3, 8) || !reg(2, 0xc8)) return State::Error;
    delay(6); // Field-on guard before first Type A poll.
    return request(Step::Request) ? State::Available : State::Error;
}
Phase anticollision() {
    uint8_t bytes[] = {reader.cascade, 0x20};
    return request(Step::Anticollision, bytes, 2) ? Phase::Running : Phase::Error;
}
Phase readPage(Step step, unsigned page) {
    uint8_t bytes[] = {0x30, uint8_t(page)};
    return request(step, bytes, 2) ? Phase::Running : Phase::Error;
}
Phase received(Tag &tag, const uint8_t *p) {
    switch (reader.step) {
        case Step::Request: return anticollision();
        case Step::Anticollision: {
            if ((p[0] ^ p[1] ^ p[2] ^ p[3]) != p[4]) return Phase::Error;
            uint8_t select[] = {reader.cascade, 0x70, p[0], p[1], p[2], p[3], p[4]};
            return request(Step::Select, select, sizeof select) ? Phase::Running : Phase::Error;
        }
        case Step::Select:
            if ((p[0] & 4) && reader.cascade < 0x97) {
                reader.cascade += 2;
                return anticollision();
            }
            if (p[0] != 0) return Phase::Unsupported;
            return readPage(Step::Capability, 0);
        case Step::Capability:
            if (capability(p + 12, tag) != Decode::OK) return Phase::Unsupported;
            break;
        case Step::Data: {
            memcpy(reader.data + reader.filled, p, 16);
            reader.filled += 16;
            Decode result = type2(reader.data, reader.filled, tag);
            if (result == Decode::OK) return Phase::Complete;
            if (result == Decode::Unsupported) return Phase::Unsupported;
            if (result != Decode::More) return Phase::Error;
            break;
        }
    }
    if (reader.filled + 16 > tag.capacity || reader.filled + 16 > sizeof reader.data)
        return Phase::Unsupported;
    return readPage(Step::Data, 4 + reader.filled / 4);
}
} // namespace
const ReaderStatus &readerStatus(Source s) { return statusByBus[s == Source::Cap]; }
State capStart() { return begin(true); }
State groveStart() { return begin(false); }
Phase pollTag(Tag &tag) {
    uint8_t irq[3]{};
    if (!reader.identified || !fetch(0x1a, irq, 3) || (irq[0] & 4) || (irq[2] & 0xf0)) return Phase::Error;
    if (!(irq[0] & 0x10)) {
        if (millis() - reader.sent < 40) return Phase::Running;
        if (reader.step != Step::Request) return Phase::Timeout;
        return request(Step::Request) ? Phase::Running : Phase::Error;
    }
    // CRC remains in FIFO for SELECT and READ. Hardware checks it before RX completion.
    const uint8_t sizes[] = {2, 5, 3, 18, 18};
    uint8_t status[2]{}, frame[18]{};
    unsigned size = sizes[unsigned(reader.step)];
    if (!fetch(0x1e, status, 2) || status[0] != size || status[1] || !io(0x9f, nullptr, 0, frame, size))
        return Phase::Error;
    return received(tag, frame);
}
void readerStop() {
    if (reader.identified) {
        cmd(0xc2);
        uint8_t control = 0xff;
        statusByBus[reader.cap].standbyFailed = !(reg(2, 0) && fetch(2, &control) && (control & 0xc8) == 0);
    }
    if (reader.cap) {
        digitalWrite(power, LOW);
        if (reader.bus) SPI.end();
        for (int pin : {3, 5, 6, 12, 14, 39, 40}) pinMode(pin, INPUT);
    } else if (reader.bus) Wire.end();
    reader = Reader{};
}
} // namespace lab
