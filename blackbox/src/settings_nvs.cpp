// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "workspace.h"
#include <Preferences.h>
#include <cstring>
#include "device.h"
namespace ws {
const char *settingsState = "Defaults";
namespace {
uint8_t saved[packetSize], pending[packetSize];
uint32_t changed, lastAttempt;
bool attempted;
void applyToolSettings() {
    bench::baudIndex = prefs.baud;
    bench::i2cHz = prefs.fastBus ? 400000 : 100000;
}
} // namespace
void loadSettings() {
    prefs = Prefs{};
    Preferences nvs;
    uint8_t buf[packetSize]{};
    if (nvs.begin("blackbox", true)) {
        if (nvs.isKey("prefs")) {
            bool ok = nvs.getBytesLength("prefs") == sizeof buf &&
                      nvs.getBytes("prefs", buf, sizeof buf) == sizeof buf && decode(prefs, buf, sizeof buf);
            settingsState = ok ? "Loaded" : "Invalid | defaults";
        }
        nvs.end();
    }
    applyToolSettings();
    encode(prefs, saved);
    memcpy(pending, saved, sizeof saved);
}
void applySettings() {
    saved[0] = 0; // Also replaces a previously malformed record after an explicit reset.
    applyBrightness();
    applyToolSettings();
}
void serviceSettings(uint32_t now) {
    prefs.baud = bench::baudIndex;
    prefs.fastBus = bench::i2cHz == 400000;
    uint8_t buf[packetSize];
    encode(prefs, buf);
    if (memcmp(buf, pending, sizeof buf)) {
        memcpy(pending, buf, sizeof buf);
        changed = now;
    }
    if (!memcmp(buf, saved, sizeof buf) || now - changed < 2500 || (attempted && now - lastAttempt < 30000))
        return;
    Preferences nvs;
    attempted = true;
    lastAttempt = now;
    bool retry = true;
    if (nvs.begin("blackbox", false)) {
        if (nvs.putBytes("prefs", buf, sizeof buf) == sizeof buf) {
            memcpy(saved, buf, sizeof buf);
            retry = false;
        }
        nvs.end();
    }
    settingsState = retry ? "Save failed | retry" : "Saved";
}
} // namespace ws
