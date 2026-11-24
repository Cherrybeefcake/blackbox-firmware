// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_stubs.h"
#include "device.h"
#include "lab.h"
#include "profile_stubs.h"
#include "reports.h"
#include "storage.h"
#include "subghz.h"
#include "wireless.h"
#include "workspace.h"
#include "workspace_view.h"
#include <cassert>
#include <cstdio>

namespace panel {
bool capture;
unsigned paints;
} // namespace panel
Status status{true, 3810, 182000, 3145728};
namespace rf {
Diagnostics diagnostics;
unsigned clockMs() { return bench::now(); }
const Backend physical{[] { return State::Unavailable; }, nullptr, nullptr, nullptr};
} // namespace rf
namespace lab {
const ReaderStatus &readerStatus(Source) {
    static ReaderStatus s;
    return s;
}
Phase pollTag(Tag &) {
    assert(false);
    return Phase::Error;
}
bool burstFinished() {
    assert(false);
    return false;
}
unsigned nowMs() { return bench::now(); }
} // namespace lab
namespace air {
Results results;
Session session;
Samples samples;
const char *error = "";
unsigned minimumHeap, starts, stops;
bool begin(Owner who, const Record *) {
    ++starts;
    session.owner = who;
    session.phase = Phase::Scanning;
    return true;
}
void cancel() {
    if (session.owner != Owner::None) ++stops;
    session.owner = Owner::None;
}
void forget() {
    cancel();
    results = Results{};
    samples = Samples{};
    session.phase = Phase::Idle;
}
const char *securityName(unsigned) { return "Synthetic"; }
} // namespace air
namespace ws {
const char *settingsState = "Fixture defaults";
void applySettings() {}
void launch(Destination) { assert(false); }
Volume &card() {
    assert(false);
    return ram;
}
} // namespace ws
namespace {
void key(Navigation &n, Key k) { n.handle(k, 0xffff); }
void launch(Navigation &n, unsigned id) {
    assert(n.page == Navigation::Home);
    n.selected = id;
    key(n, Key::Hold);
    assert(n.page == Navigation::Running);
}
void released() {
    assert(
        air::session.owner == air::Owner::None && ws::store.owner == ws::Client::None && !ws::store.busy &&
        !bench::fixture::edit
    );
    for (unsigned i = 0; i < moduleCount(); ++i)
        assert(modules.slot(i).owner == (i < 4 ? Owner::System : Owner::None));
}
template <class F> void shot(unsigned i, F f) {
    printf("<g transform='translate(%u %u)'>", 4 + 244 * (i % 2), 20 + 140 * (i / 2));
    panel::capture = true;
    ws::invalidate();
    f();
    panel::capture = false;
    puts("</g>");
}
} // namespace
int main() {
    modules.identify(true);
    for (unsigned i = 0; i < 4; ++i) modules.reserveSystem(i);
    Navigation n;
    for (unsigned pass = 0; pass < 100; ++pass) {
        for (unsigned i = 0; i < appCount(); ++i) {
            launch(n, i);
            appAt(i)->run->refresh();
            key(n, Key::Step);
            key(n, Key::Home);
            released();
        }
    }
    launch(n, 2);
    key(n, Key::Open);
    assert(air::starts == 1);
    n.selected = 999;
    key(n, Key::None);
    assert(n.page == Navigation::Home && air::stops == 1);
    released();
    launch(n, 10);
    key(n, Key::Open);
    assert(ws::store.owner == ws::Client::Files);
    key(n, Key::Home);
    released();
    const auto prefs = ws::prefs;
    const unsigned opens = bench::fixture::opens, radioStarts = air::starts;
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='984' height='888' viewBox='0 0 "
        "492 444' font-family='monospace'><rect width='492' height='444' fill='#202624'/><text x='4' y='13' "
        "font-size='9' fill='white'>BLACKBOX RC1 / HOST RENDER / SYNTHETIC INPUTS</text>"
    );
    shot(0, [] { initDisplay(); });
    shot(1, [] { renderHome(0, -1); });
    shot(2, [&] { launch(n, 0); });
    shot(3, [&] { key(n, Key::Open); });
    shot(4, [&] { key(n, Key::Step); });
    key(n, Key::Step);
    shot(5, [&] { key(n, Key::Hold); });
    unsigned draws = panel::paints;
    systemApp.refresh();
    assert(panel::paints == draws);
    assert(
        !memcmp(&ws::prefs, &prefs, sizeof prefs) && opens == bench::fixture::opens &&
        radioStarts == air::starts
    );
    assert(!ws::recorder.exists && ws::store.owner == ws::Client::None);
    key(n, Key::Back);
    key(n, Key::Back);
    released();
    assert(n.page == Navigation::Home);
    puts("</svg>");
}
