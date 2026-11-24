// SPDX-License-Identifier: AGPL-3.0-or-later
#include "modules.h"
const char *busName(Bus b) {
    static const char *names[] = {
        "GPIO", "I2C1 system", "I2C0 Grove", "SPI display", "SPI expansion", "UART expansion"
    };
    return names[unsigned(b)];
}
const char *stateName(State s) {
    static const char *names[] = {"Not checked", "Available", "Unavailable", "Conflict", "Active", "Error"};
    return names[unsigned(s)];
}
const char *ownerName(Owner o) {
    static const char *names[] = {"Free", "System", "Manager", "Application"};
    return unsigned(o) >= unsigned(Owner::Application) ? "Application" : names[unsigned(o)];
}
void Modules::reserveSystem(unsigned i) {
    if (valid(i) && slots[i].owner == Owner::None && blocker(i) < 0) {
        slots[i].owner = Owner::System;
        slots[i].state = State::Active;
    }
}
int Modules::blocker(unsigned i, bool activeOnly) const {
    if (!valid(i)) return -1;
    const auto &p = profiles[i];
    for (unsigned j = 0; j < count; ++j) {
        if (i == j || (activeOnly && slots[j].owner == Owner::None)) continue;
        const auto &q = profiles[j];
        if (p.bus != Bus::GPIO && p.bus == q.bus) return j;
        for (unsigned a = 0; a < p.count && a < 9; ++a)
            for (unsigned b = 0; b < q.count && b < 9; ++b)
                if (p.pins[a] == q.pins[b]) return j;
    }
    return -1;
}
State Modules::acquire(unsigned i, Owner owner) {
    if (!valid(i) || owner == Owner::None || owner == Owner::System) return State::Error;
    auto &slot = slots[i];
    const auto &p = profiles[i];
    if (slot.owner != Owner::None) return State::Conflict;
    if (board != Board::Adv || !p.start || !p.stop) return slot.state = State::Unavailable;
    if (!p.count || p.count > 9) return slot.state = State::Error;
    for (unsigned n = 0; n < p.count; ++n) {
        // Only verified ADV expansion signals can be dynamically claimed.
        const unsigned pin = p.pins[n];
        if (!(pin == 1 || pin == 2 || pin == 3 || pin == 4 || pin == 5 || pin == 6 || pin == 8 || pin == 9 ||
              pin == 12 || pin == 13 || pin == 14 || pin == 15 || pin == 39 || pin == 40 || pin == 44))
            return slot.state = State::Error;
        for (unsigned k = 0; k < n; ++k)
            if (pin == p.pins[k]) return slot.state = State::Error;
    }
    if (blocker(i) >= 0) return slot.state = State::Conflict;
    slot.owner = owner;
    auto result = p.start();
    if (result == State::Available) return slot.state = State::Active;
    p.stop();
    slot.owner = Owner::None;
    return slot.state = result == State::Unavailable ? result : State::Error;
}
void Modules::release(unsigned i, Owner owner) {
    if (!valid(i) || owner == Owner::None || owner == Owner::System || slots[i].owner != owner) return;
    profiles[i].stop();
    slots[i].owner = Owner::None;
    slots[i].state = State::Available;
}
void Modules::releaseAll(Owner owner) {
    for (unsigned i = 0; i < count; ++i) release(i, owner);
}
