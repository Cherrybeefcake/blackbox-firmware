// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
void Navigation::home() {
    if (active) active->leave();
    active = nullptr;
    page = Home;
}
void Navigation::handle(Key key, uint16_t hardware) {
    const App *app = appAt(selected);
    if (!app || (active && (page != Running || active != app->run)) || (page == Running && !active)) {
        home();
        selected = 0;
        return;
    }
    if (key == Key::Home) {
        home();
        return;
    }
    if (page != Home) {
        if (page == Running && app->run->special &&
            (key == Key::Back || key == Key::Hold || key == Key::Open)) {
            if (app->run->special(key)) return;
            home();
            return;
        }
        if (key == Key::Back || key == Key::Hold || (page != Running && key == Key::Step)) {
            home();
        } else if (page == Running) app->run->input(key);
        return;
    }
    switch (key) {
        case Key::Left:
            if (selected % columns) --selected;
            break;
        case Key::Right:
            if (selected % columns < columns - 1 && selected + 1 < appCount()) ++selected;
            break;
        case Key::Up:
            if (selected >= columns) selected -= columns;
            break;
        case Key::Down:
            if (selected + columns < appCount()) selected += columns;
            break;
        case Key::Step: selected = (selected + 1) % appCount(); break;
        case Key::Open:
        case Key::Hold:
            if (!app->run) page = Reserved;
            else if ((hardware & app->needs) != app->needs) page = Missing;
            else {
                page = Running;
                active = app->run;
                active->enter();
            }
            break;
        default: break;
    }
}
