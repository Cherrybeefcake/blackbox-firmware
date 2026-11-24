// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_stubs.h"
#include "lab.h"
#include "profile_stubs.h"
#include "storage_sd.h"
#include "subghz.h"
#include <Wire.h>
#include <cassert>
#include <cstring>
#include <initializer_list>
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
    assert(ws::claimCard(modules) == ws::Err::Locked && modules.slot(ws::sdSlot).owner == Owner::None);
    for (unsigned i : {rf::moduleIndex, capSlot}) {
        Profile profiles[9]{};
        profiles[ws::sdSlot] = *modules.profile(ws::sdSlot);
        profiles[i] = *modules.profile(i);
        for (unsigned j : {ws::sdSlot, i}) {
            profiles[j].start = [] { return State::Available; };
            profiles[j].stop = [] {};
        }
        Modules leases(profiles, 9);
        leases.identify(true);
        assert(leases.acquire(i, Owner::Manager) == State::Active && ws::claimCard(leases) == ws::Err::Busy);
        leases.release(i, Owner::Manager);
        assert(ws::claimCard(leases) == ws::Err::Ok && leases.acquire(i, Owner::Manager) == State::Conflict);
        leases.release(ws::sdSlot, Owner::Application);
        assert(leases.acquire(i, Owner::Manager) == State::Active);
    }
    modules.reserveSystem(8);
    assert(modules.blocker(capSlot) == 8 && modules.blocker(rf::moduleIndex) == 8);
    modules.reserveSystem(4);
    assert(modules.blocker(groveSlot) == 4 && modules.blocker(irSlot) < 0);
}
