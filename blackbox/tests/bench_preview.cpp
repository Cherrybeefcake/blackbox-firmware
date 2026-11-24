// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_stubs.h"
#include "preview_canvas.h"
#include "profile_stubs.h"
using namespace bench;
void airRow(int x, int y, const char *s, bool accent) { airText(x, y, s, accent); }
int main() {
    modules.identify(true);
    puts(
        "<svg xmlns='http://www.w3.org/2000/svg' width='984' height='1160' viewBox='0 0 492 580' "
        "font-family='monospace' font-size='9'><rect width='492' height='580' fill='#212728'/><text x='4' "
        "y='13' fill='white'>HARDWARE LAB / SYNTHETIC HOST FIXTURES</text>"
    );
    for (unsigned i = 0; i < 8; ++i) {
        auto &app = hardwareLabApp;
        printf("<g transform='translate(%u %u)'>", 3 + 244 * (i % 2), 18 + 140 * (i / 2));
        capture = i == 7;
        app.enter();
        capture = false;
        if (i != 7) {
            for (unsigned n = 0; n < i; ++n) app.input(Key::Step);
            capture = i >= Calculators;
            app.special(Key::Open);
            capture = false;
            if (i <= Gpio) {
                capture = i == Spi;
                app.special(Key::Open);
                capture = false;
                if (i == I2c) {
                    for (int n = 0; n < 111; ++n) app.input(Key::None);
                    capture = true;
                    app.input(Key::None);
                } else if (i != Spi) {
                    fixture::pending = "LAB READY\r\n";
                    fixture::level = 1;
                    fixture::clock += 22;
                    app.input(Key::None);
                    capture = true;
                    app.input(Key::Step);
                }
            }
        }
        capture = false;
        app.leave();
        puts("</g>");
    }
    puts("</svg>");
}
