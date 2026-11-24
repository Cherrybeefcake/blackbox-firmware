// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stdint.h>
enum class Key { None, Left, Right, Up, Down, Open, Back, Step, Hold, Character, Search, Shortcut, Home };
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
    bool (*special)(Key);
    constexpr Lifecycle(void (*e)(), void (*r)(), void (*i)(Key), void (*l)(), bool (*s)(Key) = nullptr)
        : enter(e), refresh(r), input(i), leave(l), special(s) {}
};
struct App {
    const char *name;
    const char *tile;
    uint16_t icon[8];
    uint16_t needs;
    const Lifecycle *run;
};
extern const Lifecycle hardwareLabApp, terminalApp, nfcApp, infraredApp, signalApp, wifiApp, bleApp, channelApp;
extern const Lifecycle filesApp, workspaceApp;
extern const Lifecycle systemApp, settingsApp, modulesApp;
unsigned appCount();
const App *appAt(unsigned index);
struct Navigation {
    unsigned selected = 0;
    enum Page { Home, Running, Reserved, Missing } page = Home;
    void handle(Key key, uint16_t hardware);
    void home();

private:
    const Lifecycle *active = nullptr;
};
constexpr unsigned columns = 3, pageSize = 6;
