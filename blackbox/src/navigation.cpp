// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
void Navigation::handle(Key key, uint16_t hardware) {
    const App *app = appAt(selected);
    if (!app) {
        page = Home;
        selected = 0;
        return;
    }
    if (page != Home) {
        if (key == Key::Back || key == Key::Hold || (page != Running && key == Key::Step)) {
            if (page == Running) app->run->leave();
            page = Home;
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
                app->run->enter();
            }
            break;
        default: break;
    }
}
