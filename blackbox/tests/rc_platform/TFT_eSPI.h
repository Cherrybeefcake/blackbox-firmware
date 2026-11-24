// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
namespace panel {
extern bool capture;
extern unsigned paints;
inline const char *color(uint16_t c) {
    return c == 0 ? "#000" : c == 0xffff ? "#fff" : c == 0xA554 ? "#a0aaa5" : "#10cf81";
}
} // namespace panel
constexpr uint16_t TFT_WHITE = 0xffff, TFT_BLACK = 0;
struct TFT_eSPI {
    uint16_t ink = 0xffff;
    void init() {}
    void setRotation(int r) { assert(r == 1); }
    void invertDisplay(bool) {}
    void setTextColor(uint16_t c, uint16_t) { ink = c; }
    void setTextPadding(int) {}
    void fillRect(int x, int y, int w, int h, uint16_t c) {
        assert(x >= 0 && y >= 0 && w >= 0 && h >= 0 && x + w <= 240 && y + h <= 135);
        ++panel::paints;
        if (panel::capture)
            printf("<rect x='%d' y='%d' width='%d' height='%d' fill='%s'/>\n", x, y, w, h, panel::color(c));
    }
    void drawRect(int x, int y, int w, int h, uint16_t c) {
        if (panel::capture)
            printf(
                "<rect x='%d' y='%d' width='%d' height='%d' fill='none' stroke='%s'/>\n",
                x,
                y,
                w - 1,
                h - 1,
                panel::color(c)
            );
    }
    void drawFastHLine(int x, int y, int w, uint16_t c) { fillRect(x, y, w, 1, c); }
    void fillScreen(uint16_t c) { fillRect(0, 0, 240, 135, c); }
    void drawString(const char *s, int x, int y, int font) {
        assert(x >= 0 && y >= 0 && x < 240 && y < 135);
        ++panel::paints;
        if (!panel::capture) return;
        printf(
            "<text x='%d' y='%d' font-size='%d' fill='%s'>",
            x,
            y + (font == 2 ? 13 : 8),
            font == 2 ? 16 : 9,
            panel::color(ink)
        );
        for (; *s; ++s) {
            if (*s == '<') fputs("&lt;", stdout);
            else if (*s == '&') fputs("&amp;", stdout);
            else putchar(*s);
        }
        puts("</text>");
    }
};
