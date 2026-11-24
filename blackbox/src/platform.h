// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stdint.h>
enum class Key { None, Left, Right, Up, Down, Open, Back, Step, Hold };
enum Hardware : uint16_t {
    Panel = 1,
    ADC = 2,
    Flash = 4,
    Wifi = 8,
    Bluetooth = 16,
    Radio = 32,
    Nfc = 64,
    Ir = 128,
    Pins = 256,
    SerialPort = 512,
    Storage = 1024
};
struct Lifecycle {
    void (*enter)();
    void (*refresh)();
    void (*input)(Key);
    void (*leave)();
};
struct App {
    const char *name;
    const char *tile;
    uint16_t icon[8];
    uint16_t needs;
    const Lifecycle *run;
};
extern const Lifecycle systemApp, settingsApp;
unsigned appCount();
const App *appAt(unsigned index);
struct Navigation {
    unsigned selected = 0;
    enum Page { Home, Running, Reserved, Missing } page = Home;
    void handle(Key key, uint16_t hardware);
};
constexpr unsigned columns = 3, pageSize = 6;
