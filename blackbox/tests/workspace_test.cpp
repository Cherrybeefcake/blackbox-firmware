// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
#include "reports.h"
#include "storage.h"
#include "workspace.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
using namespace ws;
namespace {
struct FaultVolume : RamVolume {
    bool shortWrite = false;
    unsigned closes = 0;
    int append(const char *p, const uint8_t *b, unsigned n) override {
        return RamVolume::append(p, b, (shortWrite && n) ? n - 1 : n);
    }
    void close() override { ++closes; }
};
void storage() {
    FaultVolume v;
    Store s;
    Entry entries[listLimit]{};
    unsigned n;
    Entry stat{};
    for (const char *p :
         {"",
          "/",
          "/BLACKBOX2",
          "/BLACKBOX/../OUT",
          "/BLACKBOX/a/",
          "/BLACKBOX//x",
          "/BLACKBOX/a\\b",
          "/BLACKBOX/bbstage.tmp",
          "/BLACKBOX/.tmp",
          "/BLACKBOX/x. ",
          "/BLACKBOX/foo~1",
          "/BLACKBOX/a.b.c",
          "/BLACKBOX/123456789"})
        assert(!validPath(p));
    char path[pathSize];
    assert(child(root, "DEMO.CSV", path) && validPath(path));
    parent(path);
    parent(path);
    assert(!strcmp(path, root));
    assert(!child(root, "../data", path));
    assert(s.mount(v, Client::Files) == Err::Ok && s.mount(v, Client::Report) == Err::Busy);
    assert(s.list(root, entries, n, false) == Err::Ok && n == 1 && !strcmp(entries[0].name, "DEMO.CSV"));
    assert(s.makeDir("/BLACKBOX/RUN") == Err::Ok);
    assert(s.makeDir("/BLACKBOX/run") == Err::Exists);
    assert(s.makeDir("/BLACKBOX/../xx") == Err::Invalid);
    assert(s.erase(root, true) == Err::Invalid);
    uint8_t payload[500], buf[chunk];
    for (unsigned i = 0; i < sizeof payload; ++i) payload[i] = i % 251;
    const char *a = "/BLACKBOX/RUN/NEW.BIN", *b = "/BLACKBOX/COPY.BIN";
    auto free = s.freeSpace();
    assert(s.write(a, payload, sizeof payload) == Err::Ok && !v.stat(a, stat));
    assert(s.tick() == Err::Ok && s.done == chunk && s.busy);
    assert(s.makeDir("/BLACKBOX/X") == Err::Busy);
    while (s.busy) assert(s.tick() == Err::Ok);
    assert(s.freeSpace() == free - sizeof payload);
    assert(s.copy(a, b) == Err::Ok && s.tick() == Err::Ok);
    s.cancel();
    assert(!s.busy && !v.stat(b, stat) && s.freeSpace() == free - sizeof payload);
    assert(s.copy(a, b) == Err::Ok);
    while (s.busy) assert(s.tick() == Err::Ok);
    assert(s.read(b, 256, buf, chunk) == int(chunk) && !memcmp(buf, payload + 256, chunk));
    assert(s.read(b, 0, buf, chunk + 1) < 0);
    assert(s.read("/BLACKBOX/../x", 0, buf, chunk) < 0);
    assert(s.copy(a, b) == Err::Exists && s.rename(b, a) == Err::Exists);
    assert(
        s.list(root, entries, n, true) == Err::Ok && n == 3 && entries[0].dir &&
        !strcmp(entries[1].name, "COPY.BIN")
    );
    assert(s.erase(b, false) == Err::Confirm && v.stat(b, stat));
    assert(s.erase(b, true) == Err::Ok && !v.stat(b, stat));
    assert(s.rename("/BLACKBOX/RUN", "/BLACKBOX/LOG") == Err::Ok);
    a = "/BLACKBOX/LOG/NEW.BIN";
    assert(s.read(a, 372, buf, chunk) == int(chunk) && !memcmp(buf, payload + 372, chunk));
    assert(s.write(b, payload, 9000) == Err::Full && !v.stat(b, stat));
    assert(s.owner == Client::None && s.mount(v, Client::Files) == Err::Ok);
    v.shortWrite = true;
    assert(s.copy(a, b) == Err::Ok && s.tick() == Err::Io && s.owner == Client::None && v.closes == 2);
    assert(!v.stat(b, stat) && v.stat(a, stat));
    v.shortWrite = false;
    assert(s.mount(v, Client::Report) == Err::Ok && s.copy(a, b) == Err::Ok && s.tick() == Err::Ok);
    v.present = false;
    assert(s.tick() == Err::Missing && s.owner == Client::None && !s.busy && v.closes == 3);
    assert(s.mount(v, Client::Files) == Err::Missing && !v.stat(b, stat));
    v.present = true;
    assert(s.mount(v, Client::Files) == Err::Ok && s.interrupted);
    assert(s.copy(a, b) == Err::Recovery && !v.stat(b, stat));
    char kept[13];
    assert(s.recover(false, kept) == Err::Confirm && s.recover(true, kept) == Err::Ok);
    assert(!s.interrupted && v.stat("/BLACKBOX/PART0000.TMP", stat) && stat.size == chunk);
    assert(s.copy(a, b) == Err::Ok && s.tick() == Err::Ok);
    s.unmount();
    assert(!s.busy && s.owner == Client::None && !v.stat(b, stat));
    assert(s.mount(v, Client::Report) == Err::Ok && s.unique(root, 42, true, path) == Err::Ok);
    std::string first = path;
    assert(
        s.write(path, nullptr, 0) == Err::Ok && s.tick() == Err::Ok &&
        s.unique(root, 42, true, path) == Err::Ok
    );
    assert(first != path && v.stat(first.c_str(), stat) && stat.size == 0);
}
void preferences() {
    prefs = Prefs{};
    for (auto id : {1, 3, 2, 3}) recent(id);
    recent(255);
    assert(
        prefs.recents[0] == 3 && prefs.recents[1] == 2 && prefs.recents[2] == 1 && prefs.recents[3] == noId
    );
    for (unsigned i = 0; i < historySize; ++i) assert(favorite(i));
    assert(!favorite(7) && !favorite(99));
    assert(favorite(1) && favorite(8));
    uint8_t buf[packetSize];
    encode(prefs, buf);
    Prefs out;
    assert(decode(out, buf, sizeof buf) && !memcmp(&prefs, &out, sizeof out));
    assert(!decode(out, buf, 4) && out.favorites[0] == noId);
    buf[4] ^= 4;
    assert(!decode(out, buf, sizeof buf));
    prefs.shortcut = 21;
    encode(prefs, buf);
    assert(!decode(out, buf, sizeof buf));
    prefs = Prefs{};
    prefs.favorites[2] = 3;
    encode(prefs, buf);
    assert(!decode(out, buf, sizeof buf));
    prefs.favorites[0] = prefs.favorites[1] = 3;
    encode(prefs, buf);
    assert(!decode(out, buf, sizeof buf));
    prefs = Prefs{};
    prefs.brightness = 254;
    encode(prefs, buf);
    assert(!decode(out, buf, sizeof buf));
    assert(out.shortcut == deskId && out.brightness == 2 && out.simulation == 1);
    prefs = Prefs{};
    for (unsigned i = 0; i < destinations(); ++i) {
        auto d = destination(i);
        assert(d.app < appCount() && *d.name && matches(d, ""));
        if (i < appCount()) assert(d.app == i && appAt(i)->run);
    }
    bool found = false, article = false, tool = false;
    for (unsigned i = 0; i < destinations(); ++i) {
        auto d = destination(i);
        found |= d.kind == Kind::Formula && matches(d, "VOLTAGE div");
        article |= d.kind == Kind::Pins && matches(d, "grove");
        tool |= d.app == 9 && d.kind == Kind::Terminal && matches(d, "baud");
        assert(!matches(d, "unknown-no-match"));
    }
    assert(found && article && tool && destination(destinations()).app == noId);
}
void reports() {
    Session s;
    Snapshot snap{400, 3790, 170000, 240, 4, 3, 8192, 8100, true, true};
    char out[reportLimit], cell[50];
    constexpr auto device = Origin::Device;
    assert(!makeReport(Report::Session, true, snap, s, out, sizeof out) && !s.start(0, 100));
    assert(s.start((1 << Battery) | (1 << SubGHz), 100) && !s.start(1, 120));
    s.observe(Millivolts, device, 3800);
    s.observe(Millivolts, device, 3900);
    s.observe(SimRssi, Origin::Simulation, -70);
    s.observe(Millivolts, device, NAN);
    s.observe(SimRssi, device, 12345);
    s.observe(FreeHeap, device, 77777);
    s.stop(300);
    s.observe(Millivolts, device, 1);
    assert(s.elapsed == 200);
    assert(makeReport(Report::Session, true, snap, s, out, sizeof out));
    assert(strstr(out, "0.8.0,uptime,100,200,") && strstr(out, "mV,DEVICE,2,3800,3900,3850,1"));
    assert(strstr(out, "dBm,SIM,1,-70,-70,-70,0") && !strstr(out, "77777") && !strstr(out, "12345"));
    assert(!strstr(out, "SSID") && !strstr(out, "BSSID") && !strstr(out, "UID") && !strstr(out, "UART"));
    assert(!makeReport(Report::Session, false, snap, s, out, 12));
    assert(makeReport(Report::Resources, false, snap, s, out, sizeof out) && strstr(out, "[B] SIM"));
    assert(makeReport(Report::Health, false, snap, s, out, sizeof out) && strstr(out, "3790"));
    assert(csvField(cell, sizeof cell, "a,\"b\"\nc") && !strcmp(cell, "\"a,\"\"b\"\"\nc\""));
    assert(!csvField(cell, 3, "xxxx"));
    s = Session{};
    assert(s.start((1 << Groups) - 1, UINT32_MAX - 100));
    for (unsigned i = 0; i < MetricCount; ++i)
        s.observe(
            Metric(i),
            i == SimRssi    ? Origin::Simulation
            : i >= Formula0 ? Origin::Calculation
                            : device,
            i
        );
    s.observe(Gpio1, Origin::Simulation, 1);
    s.stop(99);
    assert(s.elapsed == 200);
    for (bool csv : {false, true}) assert(makeReport(Report::Session, csv, snap, s, out, sizeof out));
}
} // namespace
const Lifecycle stub{[] {}, [] {}, [](Key) {}, [] {}};
const Lifecycle workspaceApp = stub, filesApp = stub, systemApp = stub, settingsApp = stub,
                hardwareLabApp = stub, terminalApp = stub, nfcApp = stub, infraredApp = stub,
                signalApp = stub, wifiApp = stub, bleApp = stub, channelApp = stub, modulesApp = stub;
int main() {
    storage();
    preferences();
    reports();
}
