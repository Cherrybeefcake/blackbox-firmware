// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "modules.h"
#include "storage.h"
namespace ws {
constexpr unsigned sdSlot = 8;
// Runtime settings cannot change the board profile's start callback.
inline Err claimCard(Modules &inventory) {
    auto p = inventory.profile(sdSlot);
    if (!p) return Err::Missing;
    if (inventory.blocker(sdSlot) >= 0 || inventory.slot(sdSlot).owner != Owner::None) return Err::Busy;
    if (!p->start) return Err::Locked;
    State state = inventory.acquire(sdSlot, Owner::Application);
    if (state == State::Active) return Err::Ok;
    return state == State::Conflict ? Err::Busy : state == State::Unavailable ? Err::Missing : Err::Io;
}
} // namespace ws
