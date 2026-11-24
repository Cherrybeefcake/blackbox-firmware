// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "device.h"
#include "storage.h"
#include "subghz.h"
#include "wireless.h"
#include "workspace.h"
#include "workspace_view.h"
#include <cstdio>
#include <cstring>
#ifdef ARDUINO
#include "blackbox_build.h"
#else
#define BLACKBOX_BUILD_ID "host-fixture"
#endif

namespace {
enum Page { Overview, Runtime, Ownership, Checks, Pages };
unsigned page, result;
bool tested;
bool validRegistry() {
    if (appCount() != ws::deskId + 1) return false;
    for (unsigned i = 0; i < appCount(); ++i) {
        const App *a = appAt(i);
        if (!a || !a->name || !*a->name || !a->tile || !*a->tile || !a->needs || !a->run || !a->run->enter ||
            !a->run->refresh || !a->run->input || !a->run->leave)
            return false;
        for (unsigned j = 0; j < i; ++j)
            if (!strcmp(a->name, appAt(j)->name)) return false;
    }
    return true;
}
// All four checks operate on firmware tables, local bytes or cached ownership.
unsigned selfTest() {
    uint8_t packet[ws::packetSize];
    ws::Prefs restored;
    ws::encode(ws::prefs, packet);
    unsigned bits = validRegistry() ? 1u : 0u;
    if (ws::decode(restored, packet, sizeof packet) && !memcmp(&restored, &ws::prefs, sizeof restored))
        bits |= 2;
    if (ws::validPath("/BLACKBOX/TEST.TXT") && !ws::validPath("/BLACKBOX/../OUT")) bits |= 4;
    bool clean = true;
    for (unsigned i = 0; i < moduleCount(); ++i)
        if (modules.slot(i).owner != Owner::None && modules.blocker(i) >= 0) clean = false;
    return clean ? bits | 8 : bits;
}
void draw() {
    const char *titles[] = {"System Info", "System Health", "Resource health", "Self-test"};
    ws::Screen screen(
        titles[page],
        "G0 next  Enter select  DEL back",
        page == Checks ? "Read-only software checks" : "Cardputer ADV | qualification pending"
    );
    char line[56];
    auto field = [&](unsigned row, const char *fmt, unsigned v) {
        snprintf(line, sizeof line, fmt, v);
        screen.row(row, line);
    };
    if (page == Overview) {
        screen.row(1, status.adv ? "ADV keyboard detected" : "ADV keyboard unconfirmed");
        snprintf(line, sizeof line, "BLACKBOX %s  %s", version, BLACKBOX_BUILD_ID);
        screen.row(2, line);
        if (status.mv) field(3, "Battery %u mV", status.mv);
        else screen.row(3, "Battery unavailable");
        field(4, "Data partition %u KiB", status.partition / 1024);
        field(5, "Free heap %u B | Enter for Health", status.heap);
    } else if (page == Runtime) {
        const auto m = bench::metrics();
        field(1, "Current free heap %u B", m.freeHeap);
        field(2, "Boot minimum heap %u B", m.minHeap);
        field(3, "Largest heap block %u B", m.largest);
        snprintf(line, sizeof line, "Preferences: %s", ws::settingsState);
        screen.row(4, line);
        screen.row(5, validRegistry() ? "Registry: 13 live applications" : "Registry validation failed");
    } else if (page == Ownership) {
        unsigned active = 0, blocked = 0;
        for (unsigned i = 0; i < moduleCount(); ++i) {
            if (modules.slot(i).owner != Owner::None) ++active;
            else if (modules.blocker(i) >= 0) ++blocked;
        }
        snprintf(line, sizeof line, "Slots owned %u | blocked %u", active, blocked);
        screen.row(1, line);
        snprintf(line, sizeof line, "Radio: %s", bench::metrics().radio);
        screen.row(2, line);
        screen.row(
            3, ws::store.owner != ws::Client::None ? "Workspace mounted" : "RAM sim ready | SD bench-locked"
        );
        snprintf(line, sizeof line, "Storage: %s", ws::describe(ws::store.last));
        screen.row(4, line);
        snprintf(
            line, sizeof line, "Lab errors %u | RF failures %u", bench::errors, rf::diagnostics.failures
        );
        screen.row(5, line);
    } else {
        const char *labels[] = {
            "Application registry", "RAM preference schema", "Workspace containment", "Resource ownership"
        };
        for (unsigned i = 0; i < 4; ++i) {
            snprintf(
                line,
                sizeof line,
                "%s  %s",
                labels[i],
                !tested              ? "--"
                : result & (1u << i) ? "PASS"
                                     : "FAIL"
            );
            screen.row(i + 1, line);
        }
        screen.row(5, tested ? "Enter: repeat   G0 tap: overview" : "Enter or hold G0 to run", true);
    }
    screen.show();
}
void enter() {
    page = Overview;
    tested = false;
    ws::invalidate();
    draw();
}
void input(Key k) {
    if (ws::move(k, page, Pages)) draw();
}
bool select(Key k) {
    if (k == Key::Back) {
        if (page == Overview) return false;
        page = Overview;
    } else if (page == Checks) {
        result = selfTest();
        tested = true;
    } else ++page;
    draw();
    return true;
}
void leave() {}
} // namespace
const Lifecycle systemApp{enter, draw, input, leave, select};
