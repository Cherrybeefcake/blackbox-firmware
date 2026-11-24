// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab.h"
#include "subghz.h"
#include <Wire.h>
#include <cassert>
#include <cstring>
#include <initializer_list>
#include "profile_stubs.h"
#include "bench_stubs.h"
int main() {
    using namespace lab;
    modules.identify(true);
    assert(
        moduleCount() == 14 && !strcmp(modules.profile(groveSlot)->id, "u216") &&
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
