// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_stubs.h"
#include "preview_canvas.h"
#include "reports.h"
#include "storage.h"
#include "workspace.h"
#include "workspace_view.h"
#include <cassert>
#include <string>
using namespace ws;
namespace {
const Profile profiles[] = {
    {"display", "LCD", Bus::DisplaySPI, 1, {35}, nullptr, nullptr}
};
std::string lines[6];
unsigned screen;
Destination opened{};
class LockedCard : public RamVolume {
    Err mount() override { return Err::Locked; }
} locked;
void step(const Lifecycle &app, unsigned n = 1) {
    while (n--) app.input(Key::Step);
}
void pick(const Lifecycle &app) { assert(app.special(Key::Hold)); }
void back(const Lifecycle &app) { assert(app.special(Key::Back)); }
void type(const Lifecycle &app, const char *s) {
    while (*s) {
        bench::fixture::ascii = *s++;
        app.input(Key::Character);
    }
}
bool shows(const char *s) {
    for (auto &line : lines)
        if (line.find(s) != std::string::npos) return true;
    return false;
}
template <class Fn> void shot(Fn draw) {
    printf("<g transform='translate(%u %u)'>", 4 + 244 * (screen % 2), 20 + 141 * (screen / 2));
    capture = true;
    invalidate();
    draw();
    capture = false;
    ++screen;
    puts("</g>");
}
} // namespace
Modules modules(profiles, 1);
unsigned moduleCount() { return 1; }
void airRow(int x, int y, const char *s, bool a) {
    lines[(y - 23) / 15] = s;
    airText(x, y, s, a);
}
namespace ws {
Volume &card() { return locked; }
const char *settingsState = "Defaults";
void applySettings() {}
void launch(Destination d) { opened = d; }
} // namespace ws
const Lifecycle stub{[] {}, [] {}, [](Key) {}, [] {}};
const Lifecycle systemApp = stub, settingsApp = stub, hardwareLabApp = stub, terminalApp = stub,
                modulesApp = stub, nfcApp = stub, infraredApp = stub, signalApp = stub, wifiApp = stub,
                bleApp = stub, channelApp = stub;
int main() {
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='984' height='1176' viewBox='0 0 492 588' "
        "font-family='monospace' font-size='9'><rect width='492' height='588' fill='#212728'/>"
        "<text x='4' y='13' fill='white'>BLACKBOX 0.8.0 / HOST PREVIEW / SYNTHETIC INPUT</text>"
    );
    auto &w = workspaceApp;
    shot([&] { w.enter(); });
    pick(w);
    type(w, "voltag");
    shot([&] { type(w, "e"); });
    assert(shows("Voltage divider"));
    pick(w);
    step(w);
    pick(w);
    assert(opened.app == labId && opened.kind == Kind::Formula);
    w.leave();
    auto &f = filesApp;
    f.enter();
    shot([&] { pick(f); });
    assert(store.owner == Client::Files && shows("DEMO.CSV"));
    pick(f);
    shot([&] { pick(f); });
    assert(shows("1.1 source") && shows("2.1 SIM"));
    back(f);
    step(f, 1);
    pick(f);
    assert(bench::fixture::edit); // New directory
    pick(f);
    assert(shows("NEW"));
    pick(f);
    pick(f);
    assert(shows("/BLACKBOX/NEW"));
    back(f);
    // Delete confirmation defaults to cancellation.
    pick(f);
    step(f, 4);
    pick(f);
    pick(f);
    assert(shows("NEW"));
    f.leave();
    assert(store.owner == Client::None);
    f.enter();
    step(f);
    shot([&] { pick(f); });
    assert(shows("SD bench lock"));
    f.leave();
    w.enter();
    step(w, 3);
    pick(w);
    pick(w);
    pick(w); // Select Battery group.
    back(w);
    step(w);
    pick(w);
    assert(recorder.running);
    w.leave();
    recorder.observe(Millivolts, Origin::Device, 3810);
    bench::fixture::clock = 3000;
    w.enter();
    step(w, 3);
    pick(w);
    step(w);
    pick(w);
    assert(!recorder.running);
    step(w, 5);
    pick(w);
    assert(store.owner == Client::Report);
    step(w, 6); // Recorder index 2, preview session
    shot([&] { pick(w); });
    assert(shows("Uptime start"));
    shot([&] { pick(w); });
    assert(shows("Cancel") && shows("Save to SIM"));
    pick(w);
    assert(!store.busy); // cancel export
    pick(w);
    step(w);
    pick(w);
    assert(store.busy);
    while (store.busy) w.input(Key::None);
    assert(shows("Saved SIM"));
    pick(w);
    step(w, 8);
    pick(w);
    pick(w);
    assert(recorder.exists); // cancel discard
    step(w, 8);
    pick(w);
    step(w);
    pick(w);
    assert(!recorder.exists);
    back(w);
    assert(store.owner == Client::None);
    step(w, 4);
    pick(w);
    step(w, 9);
    shot([&] { pick(w); });
    assert(shows("/BLACKBOX"));
    w.leave();
    Screen sample("CSV", "Host fixture", "Quoted field");
    const char first[] = "\"a,b\"";
    CsvCursor csv = csvPage(sample, CsvCursor{}, reinterpret_cast<const uint8_t *>(first), sizeof(first) - 1);
    assert(strstr(sample.rows[1], "a,b") && csv.quotePending);
    const char next[] = "\"c\",d\r\ne,f";
    csv = csvPage(sample, csv, reinterpret_cast<const uint8_t *>(next), sizeof(next) - 1);
    assert(
        strstr(sample.rows[1], "\"c") && strstr(sample.rows[2], "1.2 d") && strstr(sample.rows[3], "2.1 e")
    );
    assert(csv.row == 2 && csv.column == 2);
    assert(screen == 8 && !bench::fixture::edit);
    puts("</svg>");
}
