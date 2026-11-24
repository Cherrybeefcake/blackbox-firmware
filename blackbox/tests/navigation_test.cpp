// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
#include <cassert>
#include <cstring>

static unsigned enters, leaves, inputs;
static void enter() { ++enters; }
static void leave() { ++leaves; }
static void input(Key) { ++inputs; }
static void refresh() {}
const Lifecycle systemApp = {enter, refresh, input, leave};
const Lifecycle settingsApp = {enter, refresh, input, leave};
const Lifecycle modulesApp = {enter, refresh, input, leave};

const Lifecycle workspaceApp=systemApp, filesApp=systemApp, terminalApp=systemApp, hardwareLabApp=systemApp, nfcApp=systemApp, infraredApp=systemApp, signalApp = systemApp, wifiApp = systemApp, bleApp = systemApp, channelApp = systemApp;
int main() {
    assert(appCount() == 13 && !appAt(appCount()));
    unsigned live = 0;
    for (unsigned i = 0; i < appCount(); ++i) {
        const App &a = *appAt(i);
        assert(a.needs && *a.tile && *a.name);
        for (unsigned j = 0; j < i; ++j) assert(strcmp(a.name, appAt(j)->name));
        if (a.run) {
            ++live;
            assert(a.run->enter && a.run->leave && a.run->refresh && a.run->input);
        }
    }
    assert(live == 13);
    Navigation n;
    auto press = [&](Key key) { n.handle(key, Panel); };
    press(Key::Left);
    press(Key::Up);
    assert(n.selected == 0);
    n.handle(Key::Open, 0);
    assert(n.page == Navigation::Missing && !enters);
    press(Key::Back);
    assert(n.page == Navigation::Home);
    press(Key::Right);
    press(Key::Hold);
    assert(n.page == Navigation::Running && enters == 1);
    press(Key::Down);
    assert(inputs == 1 && n.selected == 1);
    press(Key::Hold);
    assert(n.page == Navigation::Home && leaves == 1 && n.selected == 1);
    press(Key::Right);
    press(Key::Right);
    assert(n.selected == 2);
    n.selected = 10;
    press(Key::Open);
    assert(n.page == Navigation::Running && enters == 2);
    press(Key::Back);
    assert(n.page == Navigation::Home && n.selected == 10);
    n.selected = 2;
    for (int i = 0; i < 4; ++i) press(Key::Down);
    assert(n.selected == 11);
    press(Key::Step);
    assert(n.selected == 12);
    press(Key::Step);
    assert(n.selected == 0);
    press(Key::Open);
    press(Key::Back);
    assert(n.page == Navigation::Home && enters == 3 && leaves == 3);
    n.selected = 1099;
    press(Key::Open);
    assert(n.selected == 0 && n.page == Navigation::Home);
}
