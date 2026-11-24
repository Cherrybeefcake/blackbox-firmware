// SPDX-License-Identifier: AGPL-3.0-or-later
#include "workspace_view.h"
#include "wireless_view.h"
#include <cstdio>
#include <cstring>
namespace ws {
namespace {
char lastTitle[24], lastHint[40], old[6][40];
bool colors[6];
} // namespace
void invalidate() { *lastTitle = 0; }
bool move(Key k, unsigned &i, unsigned count) {
    if (!count) return false;
    if (k == Key::Up || k == Key::Left) i = (i + count - 1) % count;
    else if (k == Key::Down || k == Key::Right || k == Key::Step) i = (i + 1) % count;
    else return false;
    return true;
}
Screen::Screen(const char *t, const char *h, const char *s) : title(t), hint(h) { row(0, s, true); }
void Screen::row(unsigned y, const char *s, bool a) {
    if (y >= 6) return;
    snprintf(rows[y], sizeof rows[y], "%.38s", s);
    accents[y] = a;
}
void Screen::item(unsigned idx, unsigned sel, const char *s) {
    if (idx / 5 != sel / 5) return;
    unsigned y = 1 + idx % 5;
    snprintf(rows[y], sizeof rows[y], "%c %.36s", idx == sel ? '>' : ' ', s);
    accents[y] = idx == sel;
}
void Screen::show() const {
    bool fresh = strcmp(lastTitle, title) || strcmp(lastHint, hint);
    if (fresh) {
        airFrame(title, hint);
        snprintf(lastTitle, sizeof lastTitle, "%s", title);
        snprintf(lastHint, sizeof lastHint, "%s", hint);
    }
    for (unsigned i = 0; i < 6; ++i) {
        if (fresh || strcmp(old[i], rows[i]) || colors[i] != accents[i]) {
            airRow(4, 23 + i * 15, rows[i], accents[i]);
            strcpy(old[i], rows[i]);
            colors[i] = accents[i];
        }
    }
}
unsigned textPage(Screen &screen, const char *data, unsigned n, bool csv) {
    unsigned pos = 0;
    for (unsigned y = 1; y <= 5 && pos < n; ++y) {
        char line[39]{};
        unsigned x = 0;
        while (pos < n && x < 38) {
            unsigned char c = data[pos++];
            if (c == '\n') break;
            if (c == '\r') continue;
            line[x++] = c == '\t' ? ' ' : c >= 32 && c < 127 ? c : '.';
        }
        screen.row(y, line, csv && y == 1);
    }
    return pos;
}
CsvCursor csvPage(Screen &screen, CsvCursor cur, const uint8_t *data, unsigned size) {
    unsigned i = 0;
    for (unsigned y = 1; y <= 5 && i < size; ++y) {
        if (cur.skipLF) {
            cur.skipLF = false;
            if (data[i] == '\n' && ++i == size) break;
        }
        char line[39];
        unsigned x = snprintf(
            line,
            sizeof line,
            "%lu.%lu %s",
            (unsigned long)cur.row,
            (unsigned long)cur.column,
            cur.started ? "+ " : ""
        );
        while (i < size && x < sizeof(line) - 1) {
            uint8_t c = data[i++];
            if (cur.quotePending) {
                cur.quotePending = false;
                if (c == '"') {
                    line[x++] = '"';
                    continue;
                }
                cur.quoted = false;
            }
            if (cur.quoted && c == '"') {
                cur.quotePending = true;
                continue;
            }
            if (!cur.started && c == '"') {
                cur.started = cur.quoted = true;
                continue;
            }
            if (!cur.quoted && (c == ',' || c == '\n' || c == '\r')) {
                if (c == ',') ++cur.column;
                else {
                    ++cur.row;
                    cur.column = 1;
                    cur.skipLF = c == '\r';
                }
                cur.started = false;
                break;
            }
            cur.started = true;
            line[x++] = c >= 32 && c < 127 ? char(c) : ' ';
        }
        line[x] = 0;
        screen.row(y, line);
    }
    cur.pos += i;
    return cur;
}
void Pager::forward() {
    if (next <= pos) return;
    if (depth == 8) {
        memmove(previous, previous + 1, 7 * sizeof(unsigned));
        depth = 7;
    }
    previous[depth++] = pos;
    pos = next;
}
void Pager::back() {
    pos = depth ? previous[--depth] : 0;
    next = pos;
}
} // namespace ws
