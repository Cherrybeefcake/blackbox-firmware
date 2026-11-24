// SPDX-License-Identifier: AGPL-3.0-or-later
#include "modules.h"
#include "platform.h"
#include <cassert>
static int starts, stops;
static State result = State::Available;
static State start() {
    ++starts;
    return result;
}
static void stop() { ++stops; }
static const Profile fixtures[] = {
    {"key", "Internal keyboard", Bus::InternalI2C,   2, {8, 9},   nullptr, nullptr},
    {"a",   "A",                 Bus::ExpansionSPI,  2, {14, 40}, start,   stop   },
    {"b",   "B",                 Bus::ExpansionSPI,  2, {1, 2},   start,   stop   },
    {"c",   "C",                 Bus::ExpansionUART, 2, {15, 14}, start,   stop   },
    {"d",   "D",                 Bus::GPIO,          2, {1, 1},   start,   stop   },
    {"e",   "E",                 Bus::GroveI2C,      2, {2, 1},   start,   stop   },
    {"f",   "F",                 Bus::GPIO,          1, {8},      start,   stop   },
    {"g",   "G",                 Bus::GPIO,          1, {27},     start,   stop   }
};
static Modules manager(fixtures, 8);
static void leave() { manager.releaseAll(Owner::Manager); }
static void noop() {}
static void input(Key) {}
const Lifecycle systemApp = {noop, noop, input, noop}, settingsApp = {noop, noop, input, noop},
                modulesApp = {noop, noop, input, leave};
const Lifecycle signalApp = systemApp, wifiApp = systemApp, bleApp = systemApp, channelApp = systemApp;
int main() {
    const Owner app = Owner::Application, ui = Owner::Manager;
    assert(manager.acquire(1, ui) == State::Unavailable && starts == 0);
    manager.identify(true);
    manager.reserveSystem(0);
    assert(manager.acquire(6, ui) == State::Conflict);
    assert(manager.acquire(4, ui) == State::Error && manager.acquire(7, ui) == State::Error);
    assert(manager.acquire(99, ui) == State::Error && starts == 0);
    assert(manager.acquire(1, app) == State::Active);
    assert(manager.acquire(2, ui) == State::Conflict && manager.acquire(3, ui) == State::Conflict);
    manager.release(1, ui);
    assert(manager.slot(1).owner == app && stops == 0);
    manager.release(1, app);
    assert(manager.slot(1).owner == Owner::None && stops == 1);
    result = State::Error;
    assert(manager.acquire(2, ui) == State::Error && stops == 2 && manager.slot(2).owner == Owner::None);
    result = State::Unavailable;
    assert(manager.acquire(5, ui) == State::Unavailable && stops == 3);
    result = State::Available;
    assert(manager.acquire(3, ui) == State::Active);
    assert(manager.acquire(2, app) == State::Active);
    Navigation nav;
    nav.selected = 7;
    nav.handle(Key::Open, Panel);
    assert(nav.page == Navigation::Running);
    nav.handle(Key::Back, Panel);
    assert(nav.page == Navigation::Home && nav.selected == 7);
    assert(
        manager.slot(3).owner == Owner::None && manager.slot(2).owner == app &&
        manager.slot(0).owner == Owner::System
    );
}
