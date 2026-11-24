// SPDX-License-Identifier: AGPL-3.0-or-later
#include "device.h"
#include "modules.h"
#include "reports.h"
#include "wireless.h"
#include "workspace.h"
#include <Arduino.h>
namespace {
Navigation nav;
bool startup = true;
uint32_t started, lastSample;
ws::Destination pending{ws::noId, ws::Kind::App, 0, "", ""};
void openPending(uint16_t capabilities) {
    auto dest = pending;
    pending.app = ws::noId;
    if (!appAt(dest.app)) return;
    if (nav.page == Navigation::Running) appAt(nav.selected)->run->leave();
    nav.page = Navigation::Home;
    nav.selected = dest.app;
    nav.handle(Key::Open, capabilities);
    if (nav.page != Navigation::Running || dest.kind == ws::Kind::App) return;
    if (dest.app == ws::deskId) ws::workspaceDestination(dest.item);
    else if (dest.app == ws::labId || dest.kind == ws::Kind::Terminal)
        ws::benchDestination(dest.kind, dest.item);
    else {
        // These registered destinations only open detail pages. Start remains a separate input.
        auto run = appAt(dest.app)->run;
        for (unsigned i = 0; i < dest.item; ++i) run->input(Key::Step);
        run->special(Key::Open);
    }
}
void recordDevice() {
    using namespace ws;
    if (!recorder.running) return;
    if (millis() - recorder.started >= 86400000U) {
        recorder.stop(millis());
        return;
    }
    recorder.observe(Millivolts, Origin::Device, status.mv, !status.mv);
    recorder.observe(FreeHeap, Origin::Device, status.heap);
    if (recorder.wants(Profiler)) recorder.observe(LargestHeapBlock, Origin::Device, ESP.getMaxAllocHeap());
    recorder.observe(CpuMHz, Origin::Device, getCpuFrequencyMhz());
    if (recorder.wants(ModulesUsed)) {
        unsigned n = 0;
        for (unsigned i = 0; i < moduleCount(); ++i)
            if (modules.slot(i).owner != Owner::None) ++n;
        recorder.observe(ActiveModules, Origin::Device, n);
    }
}
} // namespace
namespace ws {
void launch(Destination d) { pending = d; }
} // namespace ws
void setup() {
    ws::loadSettings();
    initDisplay();
    initStatus(initInput());
    modules.identify(status.adv);
    for (unsigned i = 0; i < 4; ++i) modules.reserveSystem(i);
    started = millis();
}
void loop() {
    Key key = readInput();
    air::poll();
    if (startup) {
        if (millis() - started >= 850) {
            startup = false;
            renderHome(nav.selected, -1);
        }
    } else {
        auto oldPage = nav.page;
        unsigned oldSelection = nav.selected;
        uint16_t capabilities = boardCapabilities(status.partition != 0);
        if (nav.page == Navigation::Home && key == Key::Search)
            pending = {ws::deskId, ws::Kind::Workspace, 0, "", ""};
        else if (nav.page == Navigation::Home && key == Key::Shortcut)
            pending = ws::destination(ws::prefs.shortcut);
        else nav.handle(key, capabilities);
        bool jumping = pending.app != ws::noId;
        if (jumping) openPending(capabilities);
        if (nav.page == Navigation::Running && nav.selected != ws::deskId && (oldPage != nav.page || jumping))
            ws::recent(nav.selected);
        if (nav.page == Navigation::Home && (oldPage != nav.page || oldSelection != nav.selected))
            renderHome(nav.selected, oldPage == nav.page ? int(oldSelection) : -1);
        if (nav.page != oldPage && (nav.page == Navigation::Reserved || nav.page == Navigation::Missing))
            showNotice(*appAt(nav.selected), nav.page == Navigation::Missing);
        if (millis() - lastSample >= 1000) {
            lastSample = millis();
            sampleStatus();
            recordDevice();
            refreshBar();
            if (nav.page == Navigation::Running) appAt(nav.selected)->run->refresh();
        }
    }
    ws::serviceSettings(millis());
    delay(8);
}
