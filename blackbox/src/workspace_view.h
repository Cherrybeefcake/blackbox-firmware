// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "platform.h"
#include <stddef.h>
namespace ws {
void invalidate();
bool move(Key, unsigned &cursor, unsigned count);
struct Screen {
    const char *title, *hint;
    char rows[6][40]{};
    bool accents[6]{};
    Screen(const char *t, const char *h, const char *status);
    void row(unsigned y, const char *s, bool accent = false);
    void item(unsigned index, unsigned cursor, const char *s);
    void show() const;
};
// Convert bounded input into five screen lines, returning the exact byte offset consumed.
unsigned textPage(Screen &, const char *data, unsigned n, bool csv = false);
struct CsvCursor {
    uint32_t pos = 0, row = 1, column = 1;
    bool quoted = false, quotePending = false, started = false, skipLF = false;
};
CsvCursor csvPage(Screen &, CsvCursor, const uint8_t *, unsigned);
struct Pager {
    unsigned pos = 0, next = 0, previous[8]{}, depth = 0;
    void forward();
    void back();
};
} // namespace ws
