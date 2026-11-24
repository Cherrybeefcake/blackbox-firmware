// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <cstdio>
#include "wireless_view.h"
static bool capture;
void airText(int x, int y, const char *s, bool accent) {
    if (!capture) return;
    printf("<text x='%d' y='%d' fill='%s'>", x, y + 7, accent ? "#10cf81" : "white");
    for (; *s; ++s) {
        if (*s == '<') fputs("&lt;", stdout);
        else if (*s == '&') fputs("&amp;", stdout);
        else putchar(*s);
    }
    puts("</text>");
}
void airRect(int x, int y, int w, int h, bool accent) {
    if (capture)
        printf(
            "<rect x='%d' y='%d' width='%d' height='%d' fill='%s'/>", x, y, w, h, accent ? "#10cf81" : "#999"
        );
}
void airFrame(const char *title, const char *footer) {
    if (!capture) return;
    puts("<rect width='240' height='135'/><path d='M4 19h232M4 113h232' stroke='#10cf81'/>");
    airText(4, 3, "BLACKBOX", true);
    airText(80, 5, title);
    airText(4, 120, footer);
}
