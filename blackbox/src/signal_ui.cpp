// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
#include "subghz.h"
#include "wireless_view.h"
#include <cstdio>
namespace {
using namespace rf;
const char *const pages[] = {
    "Spectrum Analyzer",
    "Frequency Monitor",
    "RSSI Meter",
    "Activity History",
    "Module Diagnostics",
    "Source",
    "Dashboard"
};
const char *const states[] = {
    "Ready",
    "Measuring",
    "Stopped",
    "30s limit reached",
    "U219 unavailable",
    "Resource conflict",
    "Measurement error"
};
Session session;
unsigned selected, action, center, spacing;
bool detail, demo;
const char *source() { return demo ? "SIMULATED" : "U219"; }
void draw() {
    char line[48];
    airFrame(detail ? pages[selected] : "Signal Center", "G0: next   Hold: select   DEL: back");
    if (!detail) {
        unsigned first = selected < 5 ? 0 : 2;
        for (unsigned n = first; n < first + 5; ++n) {
            snprintf(
                line,
                sizeof line,
                "%c %s%s%s",
                selected == n ? '>' : ' ',
                pages[n],
                n == 5 ? "  " : "",
                n == 5 ? source() : ""
            );
            airText(5, 25 + (n - first) * 17, line, n == selected);
        }
        return;
    }
    snprintf(line, sizeof line, "%s | %s", source(), states[unsigned(session.result)]);
    airText(4, 24, line, true);
    if (selected == 4) {
        if (diagnostics.identityRead)
            snprintf(line, sizeof line, "U219 part %02X / rev %02X", diagnostics.part, diagnostics.revision);
        else snprintf(line, sizeof line, "U219 identity unconfirmed");
        airText(4, 40, line);
        snprintf(line, sizeof line, "SPI owner: %s", ownerName(modules.slot(moduleIndex).owner));
        airText(4, 54, line);
        int blocker = modules.blocker(moduleIndex);
        airText(
            4,
            68,
            blocker >= 0 ? modules.profile(blocker)->name
            : confirmed  ? "Bench profile enabled"
                         : "Profile awaits bench confirmation"
        );
        snprintf(
            line, sizeof line, "Failed %u  SPI timeouts %u", diagnostics.failures, diagnostics.readyTimeouts
        );
        airText(4, 84, line);
    } else if (!demo && (!confirmed || session.result == Result::Missing)) {
        airText(4, 45, confirmed ? "Check U219 seating and identity" : "U219 awaits physical confirmation");
        airText(4, 61, "Choose Source for simulated readings");
    } else {
        snprintf(line, sizeof line, "%u kHz   %s", center, selected == 0 ? "sweep" : "fixed");
        airText(4, 37, line);
        if (selected == 0) {
            snprintf(line, sizeof line, "%u kHz step | 102 kHz filter", spacing);
            airText(4, 49, line);
        }
        if (session.count) {
            int lo = session.at(0).dbm, hi = lo, sum = 0;
            unsigned n = selected == 0 ? session.seen : session.count;
            for (unsigned i = 0; i < n; ++i) {
                int v = selected == 0 ? session.current[i] : session.at(i).dbm;
                if (v < lo) lo = v;
                if (v > hi) hi = v;
                sum += v;
                auto height = [](int x) { return x < -115 ? 1 : x > -35 ? 25 : 1 + (x + 115) * 24 / 80; };
                int x = 5 + i * (selected == 0 ? 13 : 7);
                airRect(x, 85 - height(v), selected == 0 ? 8 : 4, height(v), true);
                if (selected == 0) airRect(x, 85 - height(session.hold[i]), 8, 1, false);
            }
            auto last = session.at(session.count - 1);
            if (selected == 2) {
                snprintf(line, sizeof line, "Latest %d dBm", last.dbm);
                airText(4, 49, line);
            }
            if (selected == 0)
                snprintf(line, sizeof line, "Peak %u kHz  %d dBm", session.peakKhz, session.peak);
            else if (selected == 2)
                snprintf(line, sizeof line, "Min %d  Max %d  Avg %d dBm", lo, hi, sum / int(n));
            else if (selected == 3)
                snprintf(
                    line, sizeof line, "t+%ums   Active ~%ums", last.ms - session.startMs, session.activityMs
                );
            else
                snprintf(
                    line, sizeof line, "%d dBm  %s (-70 dBm)", last.dbm, last.dbm >= -70 ? "Active" : "Quiet"
                );
            airText(4, 88, line);
        } else airText(4, 72, "No samples");
    }
    const char *options[] = {session.backend ? "Stop" : "Start", "Frequency +100 kHz", "Toggle span", "Back"};
    snprintf(line, sizeof line, "> %s", options[action]);
    airText(4, 102, line, true);
}
void enter() {
    session = Session{};
    detail = demo = false;
    selected = action = 0;
    center = 433900;
    spacing = 50;
    draw();
}
void leave() {
    session.stop();
    session = Session{};
}
void input(Key k) {
    if (k == Key::None) {
        auto before = session.result;
        session.tick(clockMs());
        if (before != session.result) draw();
        return;
    }
    bool prev = k == Key::Up || k == Key::Left;
    if (!prev && k != Key::Step && k != Key::Down && k != Key::Right) return;
    if (!detail) selected = (selected + (prev ? 6 : 1)) % 7;
    else {
        do {
            action = (action + (prev ? 3 : 1)) % 4;
        } while ((action == 1 && (selected == 4 || session.backend)) ||
                 (action == 2 && (selected != 0 || session.backend)));
    }
    draw();
}
bool special(Key k) {
    if (k == Key::Back || (detail && action == 3)) {
        if (!detail) return false;
        leave();
        detail = false;
    } else if (!detail) {
        if (selected == 6) return false;
        if (selected == 5) demo = !demo;
        else {
            detail = true;
            action = 0;
        }
    } else if (action == 0) {
        if (session.backend) session.stop();
        else session.begin(demo ? simulated : physical, selected == 0, center, spacing, clockMs());
    } else {
        session = Session{};
        if (action == 2) spacing = spacing == 50 ? 100 : 50;
        unsigned margin = selected == 0 ? 8 * spacing : 0;
        if (action == 1) center += 100;
        if (center > 435000 - margin || center < 433000 + margin) center = 433000 + margin;
    }
    // A frequency selected in fixed mode can be outside a sweep's valid span.
    if (detail && selected == 0 && (center < 433000 + 8 * spacing || center > 435000 - 8 * spacing))
        center = 433900;
    draw();
    return true;
}
void refresh() {
    if (detail && session.backend) draw();
}
} // namespace
const Lifecycle signalApp{enter, refresh, input, leave, special};
