// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
#include "preview_canvas.h"
#include "subghz.h"
#include <cassert>
static const Profile profiles[6]{};
Modules modules(profiles, 6);
namespace rf {
Diagnostics diagnostics;
unsigned now;
unsigned clockMs() { return now; }
const Backend physical{[] { return State::Unavailable; }, nullptr, nullptr, nullptr};
} // namespace rf
int main() {
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' width='984' height='896' viewBox='0 0 492 448' "
        "font-family='monospace' font-size='9'><rect width='492' height='448' fill='#24292b'/><text x='4' "
        "y='13' fill='white'>HOST PREVIEW | SYNTHETIC RF DATA</text>"
    );
    for (unsigned screen = 0; screen < 6; ++screen) {
        auto &app = signalApp;
        capture = false;
        app.enter();
        if (screen < 5) {
            for (int i = 0; i < 5; ++i) app.input(Key::Step);
            assert(app.special(Key::Hold));
            for (unsigned i = 0; i < screen + 2; ++i) app.input(Key::Step);
        }
        assert(app.special(Key::Open));
        assert(app.special(Key::Hold));
        for (int i = 0; i < 40; ++i) {
            rf::now += 100;
            app.input(Key::None);
        }
        printf("<g transform='translate(%u %u)'>", 4 + 244 * (screen % 2), 20 + 142 * (screen / 2));
        capture = true;
        if (screen == 5) app.input(Key::Step);
        else app.refresh();
        puts("</g>");
        capture = false;
        assert(app.special(Key::Back));
        assert(!app.special(Key::Back));
        app.leave();
    }
    puts("</svg>");
}
