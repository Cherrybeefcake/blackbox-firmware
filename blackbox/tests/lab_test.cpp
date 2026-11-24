// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab_fixture.h"
#include <cassert>
#include <climits>
#include <cstring>
#include <initializer_list>
using namespace lab;
int main() {
    Tag tag;
    assert(demoTag(tag) == Phase::Complete && tag.capacity == 144 && tag.readOnly && tag.count == 2);
    assert(
        strcmp(tag.records[0].text, "Lab note") == 0 && strcmp(tag.records[1].text, "https://lab.local") == 0
    );
    uint8_t invalidCC[] = {0xe1, 0x10, 18, 0x10};
    assert(capability(invalidCC, tag) == Decode::Unsupported && !tag.capacity && !tag.count);
    uint8_t cc[] = {0xe1, 0x10, 18, 0};
    assert(capability(cc, tag) == Decode::OK && !tag.readOnly);
    uint8_t badTlv[] = {3, 150};
    assert(type2(badTlv, 2, tag) == Decode::Invalid);
    uint8_t reserved[] = {1, 3, 0xa0, 0x0c, 0x34};
    assert(type2(reserved, sizeof reserved, tag) == Decode::Unsupported);
    uint8_t msg[] = {0xd1, 1, 5, 'T', 2, 'e', 'n', 'A', 'B'};
    for (unsigned n = 0; n < sizeof msg; ++n) assert(ndef(msg, n, tag) == Decode::Invalid && !tag.count);
    assert(ndef(msg, sizeof msg, tag) == Decode::OK && !strcmp(tag.records[0].text, "AB"));
    uint8_t longMsg[] = {0xc1, 1, 0, 0, 0, 2, 'U', 0, 'x'};
    assert(ndef(longMsg, sizeof longMsg, tag) == Decode::OK);
    longMsg[2] = 0xff;
    assert(ndef(longMsg, sizeof longMsg, tag) == Decode::Invalid);
    uint8_t tooMany[] = {0x90, 0, 0, 0x10, 0, 0, 0x10, 0, 0, 0x10, 0, 0, 0x50, 0, 0};
    assert(ndef(tooMany, sizeof tooMany, tag) == Decode::Unsupported && !tag.count);
    uint8_t longText[129] = {0xd1, 1, 124, 'T', 0};
    memset(longText + 5, 'x', 123);
    assert(
        ndef(longText, 128, tag) == Decode::OK && tag.records[0].clipped && strlen(tag.records[0].text) == 64
    );
    assert(ndef(longText, 129, tag) == Decode::Unsupported);
    msg[4] = 63;
    assert(ndef(msg, sizeof msg, tag) == Decode::Invalid);
    Session s;
    s.start(Source::Cap, false, 0);
    assert(s.phase == Phase::Unavailable && !fixture::starts);
    modules.identify(true);
    for (unsigned slot : {5U, 8U, 4U}) {
        assert(modules.acquire(slot, Owner::Manager) == State::Active);
        s.start(slot == 4 ? Source::Grove : Source::Cap, false, 0);
        assert(s.phase == Phase::Conflict && !s.leased);
        modules.release(slot, Owner::Manager);
    }
    fixture::initResult = State::Unavailable;
    s.start(Source::Cap, false, 0);
    assert(s.phase == Phase::Unavailable && modules.slot(capSlot).owner == Owner::None);
    fixture::initResult = State::Error;
    auto stops = fixture::stops;
    s.start(Source::Cap, false, 0);
    assert(s.phase == Phase::Error && fixture::stops == stops + 1);
    fixture::initResult = State::Available;
    s.start(Source::Cap, false, UINT_MAX - 20);
    s.tick(5);
    assert(s.leased && s.phase == Phase::Running);
    s.tick(nfcLimitMs - 21);
    assert(s.phase == Phase::Timeout && modules.slot(capSlot).owner == Owner::None);
    s.start(Source::Grove, false, 0);
    s.finish(Phase::Stopped);
    assert(!s.leased && !s.tag.count);
    s.start(Source::Emitter, true, 0);
    s.tick(irLimitMs - 1);
    assert(s.leased);
    s.tick(irLimitMs);
    assert(s.phase == Phase::Timeout && !s.leased);
    s.start(Source::Emitter, true, 0);
    fixture::finished = true;
    s.tick(3);
    assert(s.phase == Phase::Complete && !s.leased);
    auto starts = fixture::starts;
    s.start(Source::Simulation, false, 0);
    s.tick(100);
    assert(s.tag.count == 2 && s.phase == Phase::Complete && fixture::starts == starts);
    s.finish(Phase::Idle);
    assert(!s.tag.count && !s.tag.records[0].text[0]);
}
