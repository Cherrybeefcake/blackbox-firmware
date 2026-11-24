// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab.h"
#include "subghz.h"
#include <Wire.h>
#include <cassert>
#include <cstring>
#include <initializer_list>
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
int main() {
    using namespace lab;
    modules.identify(true);
    assert(
        moduleCount() == 11 && !strcmp(modules.profile(groveSlot)->id, "u216") &&
        !strcmp(modules.profile(capSlot)->id, "u219-nfc") && !strcmp(modules.profile(irSlot)->id, "adv-ir")
    );
    for (unsigned slot : {capSlot, groveSlot, irSlot}) {
        assert(!modules.profile(slot)->start && modules.slot(slot).owner == Owner::None);
        assert(
            modules.acquire(slot, Owner::Application) == State::Unavailable &&
            modules.slot(slot).owner == Owner::None
        );
    }
    modules.reserveSystem(8);
    assert(modules.blocker(capSlot) == 8 && modules.blocker(rf::moduleIndex) == 8);
    modules.reserveSystem(4);
    assert(modules.blocker(groveSlot) == 4 && modules.blocker(irSlot) < 0);
}
