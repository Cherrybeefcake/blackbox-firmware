// SPDX-License-Identifier: AGPL-3.0-or-later
#include "device.h"
#include "modules.h"
#include <Arduino.h>
namespace {
Navigation nav;
bool startup = true;
uint32_t started, lastSample;
} // namespace
void setup() {
    initDisplay();
    initStatus(initInput());
    modules.identify(status.adv);
    for (unsigned i = 0; i < 4; ++i) modules.reserveSystem(i);
    started = millis();
}
void loop() {
    Key key = readInput();
    if (startup) {
        if (millis() - started >= 850) {
            startup = false;
            renderHome(nav.selected, -1);
        }
    } else {
        auto oldPage = nav.page;
        unsigned oldSelection = nav.selected;
        nav.handle(key, boardCapabilities(status.partition != 0));
        if (nav.page == Navigation::Home && (oldPage != nav.page || oldSelection != nav.selected))
            renderHome(nav.selected, oldPage == nav.page ? int(oldSelection) : -1);
        if (nav.page != oldPage && (nav.page == Navigation::Reserved || nav.page == Navigation::Missing))
            showNotice(*appAt(nav.selected), nav.page == Navigation::Missing);
        if (millis() - lastSample >= 1000) {
            lastSample = millis();
            sampleStatus();
            refreshBar();
            if (nav.page == Navigation::Running) appAt(nav.selected)->run->refresh();
        }
    }
    delay(8);
}
