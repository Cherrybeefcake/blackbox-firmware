// SPDX-License-Identifier: AGPL-3.0-or-later
#include "device.h"
#include "lab.h"
#include "subghz.h"
#include "modules.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
namespace {
TFT_eSPI lcd;
constexpr uint16_t green = 0x066F, muted = 0xA554;
unsigned brightness = 2, focus = 0;
bool contrast = false;
uint16_t detail() { return contrast ? TFT_WHITE : muted; }
void text(int x, int y, const char *s, uint16_t color, int font = 1, int pad = 0) {
    lcd.setTextColor(color, TFT_BLACK);
    lcd.setTextPadding(pad);
    lcd.drawString(s, x, y, font);
    lcd.setTextPadding(0);
}
char oldBar[16];
void frame(const char *title, const char *hint) {
    lcd.fillScreen(TFT_BLACK);
    text(4, 1, "BLACKBOX", green, 2);
    text(80, 5, title, TFT_WHITE);
    lcd.drawFastHLine(4, 19, 232, green);
    lcd.drawFastHLine(4, 113, 232, green);
    text(4, 120, hint, detail());
    oldBar[0] = 0;
    refreshBar();
}
void tile(unsigned index, bool selected) {
    const App *app = appAt(index);
    if (!app) return;
    unsigned slot = index % pageSize;
    int x = 3 + 79 * (slot % columns), y = 22 + 45 * (slot / columns);
    lcd.drawRect(x, y, 76, 42, selected ? green : detail());
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            if (app->icon[r] & (128 >> c))
                lcd.fillRect(x + 5 + c * 2, y + 4 + r * 2, 2, 2, app->run ? green : detail());
    if (!app->run) text(x + 26, y + 8, "PLANNED", detail());
    text(x + 5, y + 26, app->tile, selected ? green : TFT_WHITE);
}
char previous[5][32];
void field(unsigned i, const char *value) {
    if (strcmp(previous[i], value) == 0) return;
    snprintf(previous[i], sizeof(previous[i]), "%s", value);
    text(92, 26 + i * 17, value, TFT_WHITE, 1, 144);
}
void systemRefresh() {
    char buf[32];
    field(0, status.adv ? "Cardputer ADV" : "ADV unconfirmed");
    if (status.mv) snprintf(buf, sizeof(buf), "%u.%03u V", status.mv / 1000, status.mv % 1000);
    field(1, status.mv ? buf : "Unavailable");
    snprintf(buf, sizeof(buf), "%u KiB free", status.heap / 1024);
    field(2, buf);
    snprintf(buf, sizeof(buf), "%u KiB reserved", status.partition / 1024);
    field(3, status.partition ? buf : "Partition missing");
    field(4, version);
}
void systemEnter() {
    frame("SYSTEM INFO", "DEL or hold G0: Home");
    const char *names[] = {"DEVICE", "BATTERY", "HEAP", "FLASH DATA", "FIRMWARE"};
    for (unsigned i = 0; i < 5; ++i) text(5, 26 + i * 17, names[i], detail());
    memset(previous, 0, sizeof(previous));
    systemRefresh();
}
void settingsRows() {
    const char *levels[] = {"Low", "Medium", "High"};
    text(5, 33, focus == 0 ? "> Brightness" : "  Brightness", detail(), 2, 112);
    text(122, 33, levels[brightness], green, 2, 110);
    text(5, 58, focus == 1 ? "> Contrast" : "  Contrast", detail(), 2, 112);
    text(122, 58, contrast ? "High" : "Standard", green, 2, 110);
}
void settingsEnter() {
    frame("SETTINGS", "ENTER: change   DEL / hold G0: Home");
    text(5, 88, "Up/down: field   G0: next preset", detail());
    settingsRows();
}
void settingsInput(Key k) {
    bool previousContrast = contrast;
    if (k == Key::Up || k == Key::Down) focus = 1 - focus;
    else if (k == Key::Step) {
        brightness = (brightness + 1) % 3;
        if (brightness == 0) contrast = !contrast;
    } else if (k == Key::Open || k == Key::Right || k == Key::Left) {
        if (focus == 0) brightness = (brightness + (k == Key::Left ? 2 : 1)) % 3;
        else contrast = !contrast;
    } else return;
    const int duty[] = {65, 150, 255};
    ledcWrite(0, duty[brightness]);
    if (contrast != previousContrast) settingsEnter();
    else settingsRows();
}
unsigned selectedModule;
bool moduleDetails;
const char *availability(unsigned index) {
    auto s = modules.slot(index);
    if (index == 1 && !status.adv) return "Unconfirmed";
    if (s.owner != Owner::None) return stateName(s.state);
    if (modules.blocker(index) >= 0) return "Conflict";
    return stateName(s.state);
}
void moduleView() {
    const Profile &p = *modules.profile(selectedModule);
    if (!moduleDetails) {
        frame("MODULES", "Up/down: select   Enter: details");
        unsigned first = selectedModule / 5 * 5;
        for (unsigned i = first; i < moduleCount() && i < first + 5; ++i) {
            int y = 25 + 17 * (i - first);
            text(3, y, i == selectedModule ? ">" : " ", green);
            text(13, y, modules.profile(i)->name, i == selectedModule ? green : TFT_WHITE);
            text(144, y, availability(i), detail());
        }
        return;
    }
    frame("DETAILS", "Left: list   DEL / hold G0: Home");
    text(5, 23, p.name, green, 2);
    char line[40];
    snprintf(line, sizeof(line), "%s | %s", busName(p.bus), availability(selectedModule));
    text(5, 43, line, TFT_WHITE);
    int len = snprintf(line, sizeof(line), "GPIO");
    for (unsigned i = 0; i < p.count; ++i) len += snprintf(line + len, sizeof(line) - len, " %u", p.pins[i]);
    text(5, 57, line, detail());
    int block = modules.blocker(selectedModule);
    if (block >= 0) snprintf(line, sizeof(line), "Blocked by %s", modules.profile(block)->name);
    else snprintf(line, sizeof(line), "Owner: %s", ownerName(modules.slot(selectedModule).owner));
    text(5, 71, line, TFT_WHITE);
    int shared = modules.blocker(selectedModule, false);
    if (block < 0 && shared >= 0) {
        snprintf(line, sizeof(line), "Shares with %s", modules.profile(shared)->name);
        text(5, 82, line, detail());
    }
    const char *action = "Profile only - driver disabled";
    if (modules.slot(selectedModule).owner == Owner::System) action = "Reserved by board firmware";
    if (selectedModule == rf::moduleIndex) action = "Open Sub-GHz for RX diagnostics";
    else if (selectedModule == lab::capSlot || selectedModule == lab::groveSlot) action = "Open NFC Center for reader status";
    else if (selectedModule == lab::irSlot) action = "Open Infrared Lab for emitter status";
    else if (selectedModule == 7 || selectedModule >= 11) action = "Open Hardware Lab to use";
    else if (p.start) action = status.adv ? "ENTER: check address (ACK only)" : "Disabled: ADV unconfirmed";
    text(5, 99, action, detail());
}
void modulesEnter() {
    selectedModule = 0;
    moduleDetails = false;
    moduleView();
}
void modulesInput(Key k) {
    if (k == Key::None) return;
    if (!moduleDetails) {
        if (k == Key::Down) selectedModule = (selectedModule + 1) % moduleCount();
        else if (k == Key::Up) selectedModule = (selectedModule + moduleCount() - 1) % moduleCount();
        else if (k == Key::Right || k == Key::Open || k == Key::Step) moduleDetails = true;
        else return;
    } else if (k == Key::Left || k == Key::Step) {
        moduleDetails = false;
        if (k == Key::Step) selectedModule = (selectedModule + 1) % moduleCount();
    } else if (k == Key::Open && selectedModule == 4 && modules.profile(selectedModule)->start) {
        if (modules.acquire(selectedModule, Owner::Manager) == State::Active)
            modules.release(selectedModule, Owner::Manager);
    } else return;
    moduleView();
}
void modulesLeave() { modules.releaseAll(Owner::Manager); }
void idle() {}
void ignore(Key) {}
} // namespace
const Lifecycle modulesApp = {modulesEnter, idle, modulesInput, modulesLeave};
const Lifecycle systemApp = {systemEnter, systemRefresh, ignore, idle};
const Lifecycle settingsApp = {settingsEnter, idle, settingsInput, idle};
void refreshBar() {
    char buf[16];
    if (status.mv) snprintf(buf, sizeof(buf), "%u.%02uV", status.mv / 1000, status.mv % 1000 / 10);
    else snprintf(buf, sizeof(buf), "BAT --");
    if (strcmp(oldBar, buf)) {
        strcpy(oldBar, buf);
        text(192, 5, buf, detail(), 1, 48);
    }
}
void initDisplay() {
    lcd.init();
    lcd.setRotation(1);
    lcd.invertDisplay(true);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(38, 0);
    ledcWrite(0, 255);
    frame("STARTUP", version);
    text(8, 45, "Device diagnostics", TFT_WHITE, 2);
}
void renderHome(unsigned selected, int previousIndex) {
    if (previousIndex < 0 || selected / pageSize != unsigned(previousIndex) / pageSize) {
        char title[24];
        snprintf(
            title,
            sizeof(title),
            "APPS %u/%u",
            selected / pageSize + 1,
            (appCount() + pageSize - 1) / pageSize
        );
        frame(title, "; , . /   ENTER    G0: next / hold");
        unsigned first = selected / pageSize * pageSize;
        for (unsigned i = first; i < first + pageSize; ++i) tile(i, i == selected);
    } else {
        tile(previousIndex, false);
        tile(selected, true);
    }
}
void showNotice(const App &app, bool missing) {
    frame("APP STATUS", "DEL or G0: Home");
    text(6, 36, app.name, TFT_WHITE, 2);
    text(6, 68, missing ? "Required hardware unavailable" : "Planned application", green);
}
void airFrame(const char *title, const char *footer) { frame(title, footer); }
void airText(int x, int y, const char *value, bool accent) { text(x, y, value, accent ? green : TFT_WHITE); }
void airRect(int x, int y, int width, int height, bool accent) {
    lcd.fillRect(x, y, width, height, accent ? green : muted);
}
void airRow(int x, int y, const char *value, bool accent) { text(x, y, value, accent ? green : TFT_WHITE, 1, 232); }
