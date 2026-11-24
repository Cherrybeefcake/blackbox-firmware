// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab_fixture.h"
#include "platform.h"
#include "preview_canvas.h"
#include <cassert>
#include <initializer_list>
void show(bool ir, unsigned page, bool demo, unsigned position) {
    auto &app = ir ? infraredApp : nfcApp;
    auto step = [&](unsigned n) {
        while (n--) app.input(Key::Step);
    };
    auto choose = [&]() { assert(app.special(Key::Hold)); };
    capture = false;
    app.enter();
    if (demo) {
        step(3);
        choose();
        if (!ir) choose();
        step((page + 2) % 5);
    } else step(page);
    // Draw only the final interaction on each canvas.
    printf("<g transform='translate(%u %u)'>", 3 + 244 * (position % 2), 20 + 140 * (position / 2));
    if (page == 2) {
        capture = true;
        choose();
    } else {
        choose();
        capture = !demo;
        choose();
        if (demo) {
            capture = true;
            fixture::clock += 110;
            app.input(Key::None);
        }
    }
    puts("</g>");
    capture = false;
    assert(app.special(Key::Back));
    assert(!app.special(Key::Back));
    app.leave();
}
int main() {
    modules.identify(true);
    // Leave and Back must each release a still-running hardware session.
    nfcApp.enter();
    assert(nfcApp.special(Key::Open));
    assert(nfcApp.special(Key::Open));
    assert(modules.slot(lab::capSlot).owner == Owner::Application);
    assert(nfcApp.special(Key::Back));
    assert(modules.slot(lab::capSlot).owner == Owner::None);
    nfcApp.leave();
    infraredApp.enter();
    assert(infraredApp.special(Key::Hold));
    assert(infraredApp.special(Key::Hold));
    assert(modules.slot(lab::irSlot).owner == Owner::Application);
    infraredApp.leave();
    assert(modules.slot(lab::irSlot).owner == Owner::None);
    for (auto slot : {lab::capSlot, lab::groveSlot, lab::irSlot}) fixture::inventory[slot].start = nullptr;
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 492 440' width='984' height='880' "
        "font-family='monospace' font-size='9'><rect width='492' height='440' fill='#212728'/><text x='4' "
        "y='13' fill='white'>HOST PREVIEW / SYNTHETIC FIXTURES</text>"
    );
    show(false, 0, true, 0);
    show(false, 1, true, 1);
    show(false, 0, false, 2);
    show(true, 0, true, 3);
    show(true, 1, true, 4);
    show(true, 2, false, 5);
    puts("</svg>");
}
