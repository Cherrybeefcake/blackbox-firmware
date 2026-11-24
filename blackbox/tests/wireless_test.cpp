// SPDX-License-Identifier: AGPL-3.0-or-later
#include "wireless.h"
#include <cassert>
using namespace air;
static int starts, stops;
static bool initOK = true, stopOK = true;
static bool start(Owner) {
    ++starts;
    return initOK;
}
static bool stop(Owner) {
    ++stops;
    return stopOK;
}
int main() {
    Results r;
    Record a{};
    a.id[0] = 1;
    a.rssi = -60;
    a.channel = 11;
    strcpy(a.name, "Lab");
    r.add(a);
    a.rssi = -75;
    a.name[0] = 0;
    r.add(a);
    assert(r.count == 1 && r.rows[0].rssi == -75 && !strcmp(r.rows[0].name, "Lab"));
    a.type = 1;
    r.add(a);
    assert(r.count == 2);
    for (int i = 2; i < 33; ++i) {
        a.id[0] = i;
        a.channel = 1;
        a.rssi = -40;
        r.add(a);
    }
    assert(r.count == 32 && r.limited);
    a.id[0] = 99;
    a.rssi = -30;
    a.channel = 6;
    r.add(a);
    r.sort(false);
    assert(r.rows[0].id[0] == 99);
    r.sort(true);
    assert(r.rows[0].channel == 1 && r.rows[31].channel >= 6);
    Samples s;
    for (int i = 0; i < 35; ++i) s.add(-90 + i);
    int low, high, avg;
    s.stats(low, high, avg);
    assert(s.count == 32 && s.at(0) == -87 && low == -87 && high == -56 && avg == -71);
    Session session;
    Driver d = {start, stop};
    assert(session.begin(Owner::Explorer, 10, 12, d));
    assert(!session.begin(Owner::Bluetooth, 11, 12, d));
    session.end(Owner::Bluetooth, Phase::Cancelled, d);
    assert(stops == 0);
    session.end(Owner::Explorer, Phase::Cancelled, d);
    assert(stops == 1 && session.phase == Phase::Cancelled);
    assert(session.begin(Owner::Bluetooth, 0xfffffff0, 32, d));
    session.tick(15, d);
    assert(stops == 1);
    session.tick(16, d);
    assert(session.phase == Phase::Timeout && stops == 2);
    initOK = false;
    assert(!session.begin(Owner::Meter, 0, 10, d));
    assert(session.phase == Phase::Error && stops == 3);
    initOK = true;
    assert(session.begin(Owner::Channels, 0, 10, d));
    stopOK = false;
    session.end(Owner::Channels, Phase::Done, d);
    assert(session.fault && !session.begin(Owner::Explorer, 0, 10, d) && starts == 4);
}
