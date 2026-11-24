// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stdint.h>
enum class Board { Unknown, Adv };
enum class Bus { GPIO, InternalI2C, GroveI2C, DisplaySPI, ExpansionSPI, ExpansionUART };
enum class State { NotChecked, Available, Unavailable, Conflict, Active, Error };
enum class Owner { None, System, Manager, Application };
struct Profile {
    const char *id, *name;
    Bus bus;
    uint8_t count, pins[8];
    State (*start)();
    void (*stop)();
};
struct Slot {
    State state = State::NotChecked;
    Owner owner = Owner::None;
};
class Modules {
public:
    Modules(const Profile *profiles, unsigned count) : profiles(profiles), count(count) {}
    void identify(bool keyboard) { board = keyboard ? Board::Adv : Board::Unknown; }
    Board identity() const { return board; }
    const Profile *profile(unsigned i) const { return valid(i) ? profiles + i : nullptr; }
    Slot slot(unsigned i) const { return valid(i) ? slots[i] : Slot{}; }
    void reserveSystem(unsigned i);
    int blocker(unsigned i, bool activeOnly = true) const;
    State acquire(unsigned i, Owner owner);
    void release(unsigned i, Owner owner);
    void releaseAll(Owner owner);

private:
    bool valid(unsigned i) const { return count <= 16 && i < count; }
    const Profile *profiles;
    unsigned count;
    Board board = Board::Unknown;
    Slot slots[16]{};
};
extern Modules modules;
unsigned moduleCount();
const char *busName(Bus);
const char *stateName(State);
const char *ownerName(Owner);
// Inventory capabilities represent configured drivers; external profiles do not advertise presence.
uint16_t boardCapabilities(bool hasPartition);
