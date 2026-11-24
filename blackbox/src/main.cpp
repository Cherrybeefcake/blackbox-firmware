// SPDX-License-Identifier: AGPL-3.0-or-later
// Board wiring and keyboard matrix mapping follow Bruce 1.17's Cardputer interface.
#include <Arduino.h>
#include <Adafruit_TCA8418.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <esp_partition.h>

namespace {
constexpr char version[] = "0.1.0";
constexpr uint16_t emerald = 0x066F;
constexpr uint16_t muted = 0xA554;
constexpr int batteryPin = 10;
enum class Screen { Startup, Home, System };
TFT_eSPI display;
Adafruit_TCA8418 keys;
Screen screen = Screen::Startup;
bool advDetected;
uint32_t screenSince, lastReading;
const esp_partition_t *storage;
char previous[5][32] = {};

void label(int x, int y, const char *text, uint16_t color, int font = 1) {
    display.setTextColor(color, TFT_BLACK);
    display.drawString(text, x, y, font);
}

void row(int index, const char *value) {
    if (strcmp(previous[index], value) == 0) return;
    snprintf(previous[index], sizeof(previous[index]), "%s", value);
    display.setTextPadding(146);
    label(92, 27 + index * 17, value, TFT_WHITE);
    display.setTextPadding(0);
}

void refresh() {
    char value[32];
    row(0, advDetected ? "Cardputer ADV" : "ADV unconfirmed");
    unsigned mv = analogReadMilliVolts(batteryPin) * 2;
    if (mv >= 2500 && mv <= 4500) snprintf(value, sizeof(value), "%u.%03u V", mv / 1000, mv % 1000);
    else snprintf(value, sizeof(value), "Unavailable");
    row(1, value);
    snprintf(value, sizeof(value), "%u KiB free", unsigned(ESP.getFreeHeap() / 1024));
    row(2, value);
    if (storage) snprintf(value, sizeof(value), "%u KiB reserved", unsigned(storage->size / 1024));
    else snprintf(value, sizeof(value), "Partition missing");
    row(3, value);
    row(4, version);
    lastReading = millis();
}

void show(Screen next) {
    screen = next;
    screenSince = millis();
    display.fillScreen(TFT_BLACK);
    label(6, 3, "BLACKBOX", emerald, 2);
    label(167, 8, next == Screen::System ? "SYSTEM INFO" : next == Screen::Home ? "HOME" : "STARTUP", muted);
    display.drawFastHLine(5, 21, 230, emerald);
    display.drawFastHLine(5, 112, 230, emerald);
    if (next == Screen::System) {
        const char *labels[] = {"DEVICE", "BATTERY", "HEAP", "FLASH DATA", "FIRMWARE"};
        memset(previous, 0, sizeof(previous));
        for (int i = 0; i < 5; ++i) label(7, 27 + i * 17, labels[i], muted);
        refresh();
        label(7, 121, "DEL / G0   Home", emerald);
    } else if (next == Screen::Home) {
        label(9, 37, "System information", TFT_WHITE, 2);
        label(9, 68, advDetected ? "Keyboard ready" : "Keyboard unavailable: use G0", muted);
        label(7, 121, "ENTER / G0   Open", emerald);
    } else {
        label(9, 42, "BLACKBOX OS", TFT_WHITE, 2);
        label(9, 72, "Device diagnostics", muted);
        label(7, 121, version, emerald);
    }
}

void navigate(bool open) {
    if (open && screen == Screen::Home) show(Screen::System);
    else if (!open && screen == Screen::System) show(Screen::Home);
}
} // namespace

void setup() {
    pinMode(0, INPUT_PULLUP);
    pinMode(batteryPin, INPUT);
    display.init();
    display.setRotation(1);
    display.invertDisplay(true);
    pinMode(38, OUTPUT);
    digitalWrite(38, HIGH);
    show(Screen::Startup);
    advDetected = Wire1.begin(8, 9) && keys.begin(0x34, &Wire1);
    if (advDetected) {
        keys.matrix(7, 8);
        keys.flush();
    }
    storage = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs");
}

void loop() {
    static bool sampled = false, stable = false;
    static uint32_t edge;
    bool down = digitalRead(0) == LOW;
    if (down != sampled) { sampled = down; edge = millis(); }
    if (sampled != stable && millis() - edge >= 30) {
        stable = sampled;
        if (stable) navigate(screen == Screen::Home);
    }
    if (advDetected && keys.available()) {
        int event = keys.getEvent();
        // ADV mapping: matrix row 6, column 6 = Enter; row 6, column 4 = Delete.
        if (event == (0x80 | 67)) navigate(true);
        if (event == (0x80 | 65)) navigate(false);
    }
    if (screen == Screen::Startup && millis() - screenSince >= 850) show(Screen::Home);
    if (screen == Screen::System && millis() - lastReading >= 1000) refresh();
    delay(8);
}
