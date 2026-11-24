// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_stubs.h"
#include "preview_canvas.h"
#include "profile_stubs.h"
using namespace bench;
void airRow(int x, int y, const char *s, bool a) { airText(x, y, s, a); }
void step(const Lifecycle &a, unsigned n) {
    while (n--) a.input(Key::Step);
}
void choose(const Lifecycle &a) { a.special(Key::Hold); }
int main() {
    modules.identify(true);
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' width='984' height='1160' viewBox='0 0 492 580' "
        "font-family='monospace' font-size='9'><rect width='492' height='580' fill='#212728'/><text x='4' "
        "y='13' fill='white'>HOST PREVIEW / SYNTHETIC READINGS</text>"
    );
    for (unsigned i = 0; i < 8; ++i) {
        auto &a = i == 0 ? terminalApp : hardwareLabApp;
        printf("<g transform='translate(%u %u)'>", 3 + 244 * (i % 2), 18 + 140 * (i / 2));
        a.enter();
        if (i == 0) {
            choose(a);
            choose(a);
            fixture::clock += 200;
            a.input(Key::None);
            a.special(Key::Back);
            step(a, 2);
        } else step(a, i - 1);
        capture = i == 0 || i >= 4;
        choose(a);
        if (i > 0 && i <= 3) {
            choose(a);
            for (unsigned n = 0; n < 112; ++n) {
                fixture::clock += 20;
                a.input(Key::None);
            }
            capture = true;
            step(a, 1);
        }
        capture = false;
        a.leave();
        puts("</g>");
    }
    puts("</svg>");
}
