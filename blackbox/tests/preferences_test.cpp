// SPDX-License-Identifier: AGPL-3.0-or-later
#include "Preferences.h"
#include "workspace.h"
#include <cassert>
namespace fakeNvs {
unsigned size, writes;
bool fail;
unsigned char bytes[100];
} // namespace fakeNvs
namespace bench {
unsigned baudIndex, i2cHz;
}
void applyBrightness() {}
int main() {
    using namespace ws;
    loadSettings();
    serviceSettings(10000);
    assert(!fakeNvs::writes);
    prefs.theme = 1;
    favorite(4);
    serviceSettings(11000);
    recent(9);
    serviceSettings(12000);
    serviceSettings(14000);
    assert(!fakeNvs::writes);
    serviceSettings(14500);
    assert(fakeNvs::writes == 1);
    prefs = Prefs{};
    loadSettings();
    assert(prefs.theme == 1 && prefs.favorites[0] == 4 && prefs.recents[0] == 9);
    serviceSettings(50000);
    assert(fakeNvs::writes == 1);
    fakeNvs::fail = true;
    bench::baudIndex = 3;
    serviceSettings(51000);
    serviceSettings(54000);
    assert(fakeNvs::writes == 2 && strstr(settingsState, "failed"));
    fakeNvs::fail = false;
    serviceSettings(55000);
    assert(fakeNvs::writes == 2);
    serviceSettings(84000);
    assert(fakeNvs::writes == 3);
    loadSettings();
    assert(bench::baudIndex == 3);
    fakeNvs::bytes[0] = 0;
    loadSettings();
    assert(prefs.theme == 0 && prefs.favorites[0] == noId && strstr(settingsState, "Invalid"));
    prefs = Prefs{};
    applySettings();
    serviceSettings(120000);
    assert(fakeNvs::writes == 4);
    loadSettings();
    assert(!strcmp(settingsState, "Loaded"));
}
