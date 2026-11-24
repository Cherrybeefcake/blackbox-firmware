// SPDX-License-Identifier: AGPL-3.0-or-later
// GPIO-level SPI fixture for the bench-gated receiver build.
#include "subghz.h"
#include <cassert>
#include <vector>
using namespace rf;
static unsigned us, writes;
static int mode[48], level[48], bits, byteIn, header;
static bool missing, wrongId;
static std::vector<int> commands;
unsigned micros() { return us++; }
unsigned millis() { return us / 1000; }
void delay(unsigned ms) { us += ms * 1000; }
void delayMicroseconds(unsigned n) { us += n; }
void pinMode(int p, int m) { mode[p] = m; }
void digitalWrite(int p, int v) {
    ++writes;
    if (p == 5 && v == 0) { bits = byteIn = header = 0; }
    if (p == 40 && v && !level[5]) {
        assert(level[6] && level[12]);
        byteIn = (byteIn << 1) | level[14];
        if (++bits == 8) {
            header = byteIn;
            commands.push_back(header);
        }
    }
    level[p] = v;
}
int digitalRead(int p) {
    assert(p == 39);
    if (!level[40]) return missing;
    int value = header == 0xf1 ? (wrongId ? 0xff : 0x14) : header == 0xf5 ? 13 : header == 0xf4 ? 0xd4 : 0;
    return (value >> ((16 - bits) % 8)) & 1;
}
static const Profile inventory[] = {
    {},
    {},
    {},
    {},
    {},
    {"rf", "U219", Bus::ExpansionSPI, 9, {40, 14, 39, 5, 13, 15, 3, 6, 12}, initChip, powerDown},
    {"sd", "SD", Bus::ExpansionSPI, 1, {12}, [] { return State::Available; }, [] {}}
};
Modules modules(inventory, 7);
int main() {
    assert(physical.open() == State::Unavailable && !writes);
    modules.identify(true);
    assert(modules.acquire(6, Owner::Manager) == State::Active);
    assert(physical.open() == State::Conflict && !writes);
    modules.release(6, Owner::Manager);
    missing = true;
    assert(physical.open() == State::Unavailable && !commands.size() && diagnostics.readyTimeouts == 1);
    assert(modules.slot(moduleIndex).owner == Owner::None);
    missing = false;
    wrongId = true;
    assert(physical.open() == State::Unavailable && diagnostics.revision == 0xff);
    wrongId = false;
    Session s;
    s.begin(physical, true, 433900, 50, 0);
    assert(s.backend && diagnostics.part == 0 && diagnostics.revision == 0x14);
    s.tick(100);
    assert(s.count == 1 && s.at(0).dbm == -96);
    missing = true;
    s.tick(200);
    assert(!s.backend && s.result == Result::Fault && modules.slot(moduleIndex).owner == Owner::None);
    for (int pin : inventory[5].pins) assert(mode[pin] == 0);
    for (int c : commands) {
        bool allowed = false;
        for (int a :
             {0xf0,
              0xf1,
              0xf4,
              0xf5,
              0x39,
              0x30,
              0x36,
              0x34,
              0x00,
              0x02,
              0x08,
              0x12,
              0x10,
              0x18,
              0x0d,
              0x0e,
              0x0f})
            if (c == a) allowed = true;
        assert(allowed);
    }
}
