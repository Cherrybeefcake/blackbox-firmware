// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "lab.h"
#include "subghz.h"
#include <Wire.h>
#include <cassert>
TestWire Wire;
bool TestWire::begin(int, int, int) {
    assert(false);
    return false;
}
void TestWire::setTimeOut(unsigned) {}
void TestWire::beginTransmission(uint8_t) {}
int TestWire::endTransmission(bool) { return 2; }
void TestWire::end() {}
namespace lab {
State capStart() {
    assert(false);
    return State::Error;
}
State groveStart() {
    assert(false);
    return State::Error;
}
State emitterStart() {
    assert(false);
    return State::Error;
}
void readerStop() { assert(false); }
void emitterStop() { assert(false); }
} // namespace lab
namespace rf {
State initChip() {
    assert(false);
    return State::Error;
}
void powerDown() { assert(false); }
} // namespace rf
