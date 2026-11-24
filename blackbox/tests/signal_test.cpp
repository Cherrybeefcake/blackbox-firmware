// SPDX-License-Identifier: AGPL-3.0-or-later
#include "subghz.h"
#include "reports.h"
#include <cassert>
#include <climits>
#include <initializer_list>
using namespace rf;
static unsigned opens, closes;
static bool tuneOK = true, readOK = true;
static State next = State::Active;
static Backend fake{
    [] {
        ++opens;
        return next;
    },
    [](unsigned f) { return valid(f) && tuneOK; },
    [](int &r) {
        r = -60;
        return readOK;
    },
    [] { ++closes; }
};
int main() {
    Session s;
    assert(!valid(432999) && valid(433000) && valid(435000) && !valid(435001));
    s.begin(fake, true, 433000, 100, 0);
    assert(!opens && s.result == Result::Fault);
    s.begin(fake, true, 434000, UINT_MAX, 0);
    assert(!opens);
    for (State cause : {State::Unavailable, State::Conflict, State::Error}) {
        next = cause;
        s.begin(fake, false, 433900, 50, 0);
        assert(!s.backend && closes == 0);
        assert(
            s.result == (cause == State::Unavailable ? Result::Missing
                         : cause == State::Conflict  ? Result::Conflict
                                                     : Result::Fault)
        );
    }
    next = State::Active;
    tuneOK = false;
    s.begin(fake, false, 433900, 50, 0);
    assert(!s.backend && closes == 1 && s.result == Result::Fault);
    tuneOK = true;
    readOK = false;
    s.begin(fake, false, 433900, 50, 0);
    s.tick(100);
    assert(closes == 2 && !s.count && s.result == Result::Fault);
    readOK = true;
    unsigned start = UINT_MAX - 50;
    s.begin(fake, false, 433900, 50, start);
    s.tick(start + 99);
    assert(!s.count);
    for (unsigned i = 1; i <= 40; ++i) s.tick(start + i * 100);
    assert(s.count == capacity && s.at(0).ms == start + 900 && s.activityMs == 3900);
    s.tick(start + 30000);
    assert(s.result == Result::Timeout && closes == 3);
    s.begin(fake, false, 433900, 50, 0);
    s.stop();
    s.stop();
    assert(closes == 4 && s.result == Result::Stopped);
    assert(ws::recorder.start(1<<ws::SubGHz,0));
    s.begin(simulated, true, 433900, 50, 0);
    int expected = -200;
    unsigned peak = 0;
    for (unsigned i = 1; i <= 3 * bins; ++i) {
        unsigned f = s.frequency(), index = s.bin;
        s.tick(i * 100);
        assert(s.current[index] <= s.hold[index]);
        if (s.current[index] > expected) {
            expected = s.current[index];
            peak = f;
        }
    }
    assert(ws::recorder.stats[ws::SimRssi].count==3*bins && ws::recorder.stats[ws::SimRssi].high==expected);
    assert(s.peak == expected && s.peakKhz == peak && s.seen == bins && s.bin == 0);
}
