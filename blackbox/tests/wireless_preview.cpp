// SPDX-License-Identifier: AGPL-3.0-or-later
// Host-only renderer. All results and graph samples are synthetic.
#include "platform.h"
#include "wireless.h"
#include "wireless_view.h"
#include <cassert>
#include <cstdio>
namespace air {
Results results;
Samples samples;
Session session;
const char *error = "Fixture error";
unsigned minimumHeap = 0;
bool begin(Owner who, const Record *) {
    session.owner = who;
    session.phase = Phase::Scanning;
    return true;
}
void cancel() {
    session.owner = Owner::None;
    session.phase = Phase::Cancelled;
}
void forget() {
    cancel();
    results = Results{};
    samples = Samples{};
}
const char *securityName(unsigned) { return "WPA2"; }
} // namespace air
static bool capture;
void airText(int x, int y, const char *s, bool accent) {
    if (!capture) return;
    printf("<text x='%d' y='%d' fill='%s'>", x, y + 7, accent ? "#10cf81" : "white");
    for (; *s; ++s) {
        if (*s == '<') fputs("&lt;", stdout);
        else if (*s == '&') fputs("&amp;", stdout);
        else putchar(*s);
    }
    puts("</text>");
}
void airRect(int x, int y, int w, int h, bool accent) {
    if (capture)
        printf(
            "<rect x='%d' y='%d' width='%d' height='%d' fill='%s'/>", x, y, w, h, accent ? "#10cf81" : "#999"
        );
}
void airFrame(const char *title, const char *footer) {
    if (!capture) return;
    puts("<rect width='240' height='135'/><path d='M4 19h232M4 113h232' stroke='#10cf81'/>");
    airText(4, 3, "BLACKBOX", true);
    airText(80, 5, title);
    airText(4, 120, footer);
}
void fixture(const Lifecycle &app, bool ble) {
    using namespace air;
    app.enter();
    assert(app.special(Key::Open));
    results = Results{};
    const char *names[] = {"Lab North", "Lab South", "Lab Sensor"};
    for (int i = 0; i < 3; ++i) {
        Record r{};
        r.id[0] = 2;
        r.id[5] = i + 1;
        r.channel = i == 2 ? 6 : 1;
        r.rssi = -47 - i * 9;
        snprintf(r.name, sizeof r.name, "%s", ble ? "Demo beacon" : names[i]);
        if (ble) strcpy(r.service, "Environment");
        results.add(r);
    }
    session.owner = Owner::None;
    session.phase = Phase::Done;
}
int main() {
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' width='972' height='604' viewBox='0 0 486 302' "
        "font-family='monospace' font-size='9'><rect width='486' height='302' fill='#24292b'/><text x='4' "
        "y='13' fill='white'>SIMULATED DISCOVERY DATA</text>"
    );
    for (int i = 0; i < 4; ++i) {
        printf("<g transform='translate(%d %d)'>", 2 + 242 * (i % 2), 20 + 140 * (i / 2));
        auto &app = i == 1 ? channelApp : i == 3 ? bleApp : wifiApp;
        fixture(app, i == 3);
        capture = i <= 1;
        app.refresh();
        if (i == 3) {
            capture = true;
            assert(app.special(Key::Hold));
        }
        if (i == 2) {
            assert(app.special(Key::Open));
            assert(app.special(Key::Open));
            app.input(Key::Step);
            assert(app.special(Key::Hold));
            for (int n = 0; n < 18; ++n) air::samples.add(-60 + (n % 5) * 4);
            capture = true;
            app.refresh();
        }
        capture = false;
        app.leave();
        puts("</g>");
    }
    puts("</svg>");
}
