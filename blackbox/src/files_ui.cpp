// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "storage.h"
#include "workspace.h"
#include "workspace_view.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <strings.h>
namespace {
using namespace ws;
enum Page { Sources, Browse, Actions, Naming, Delete, Recovery, Viewer, Info, Transfer, Notice };
enum Op { NewDir, Rename, Copy };
Page page;
Op op;
unsigned cursor, count, selected, sourceChoice;
bool demo, hexView;
Entry rows[listLimit];
char dir[pathSize], path[pathSize], name[13], message[40];
const char *const browseActions[] = {
    "New directory",
    "Change sort",
    "Storage information",
    "Parent directory",
    "Unmount",
    "Recover interrupted file"
};
const char *const fileActions[] = {
    "Open / view", "Hex viewer", "Rename", "Copy file", "Delete", "Back to list"
};
const char *const sources[] = {"Mount RAM simulation", "Mount microSD", "Dashboard"};
Pager pager;
CsvCursor csvCur, csvNext, csvHistory[8];
bool csvView;
Err listing;
const char *type(const Entry &e) {
    if (e.dir) return "DIR";
    const char *ext = strrchr(e.name, '.');
    if (ext && !strcasecmp(ext, ".CSV")) return "CSV";
    if (ext && (!strcasecmp(ext, ".TXT") || !strcasecmp(ext, ".LOG") || !strcasecmp(ext, ".MD")))
        return "TXT";
    return "BIN";
}
void notice(const char *msg) {
    snprintf(message, sizeof message, "%s", msg);
    page = Notice;
    cursor = 0;
}
void browse() {
    page = Browse;
    cursor = 0;
    listing = store.list(dir, rows, count, prefs.sizeSort);
    if (listing != Err::Ok && listing != Err::Limit) notice(describe(listing));
}
void enter() {
    invalidate();
    page = Sources;
    cursor = sourceChoice = prefs.simulation ? 0 : 1;
    strcpy(dir, root);
}
void leave() {
    bench::textMode(false);
    if (store.owner == Client::Files) store.unmount();
    memset(rows, 0, sizeof rows);
}
void draw() {
    const char *title = page == Viewer ? (hexView ? "Hex viewer" : type(rows[selected])) : "Files";
    const char *hint = page == Naming   ? "Type | DEL erase | G0 tap cancel"
                       : page == Viewer ? "Next: G0 / right   Enter / DEL: back"
                                        : "G0 next / hold select    DEL back";
    char status[48];
    snprintf(
        status,
        sizeof status,
        "%s",
        page == Sources ? "Choose storage"
        : store.owner != Client::Files ? (demo ? "SIM | Unmounted" : "SD | Unmounted")
        : demo          ? "SIM | RAM workspace"
                        : "SD | BLACKBOX workspace"
    );
    Screen s(title, hint, status);
    char line[48];
    if (page == Sources) {
        for (unsigned i = 0; i < 3; ++i) s.item(i, cursor, sources[i]);
    } else if (page == Browse) {
        snprintf(
            line,
            sizeof line,
            "%s %s %.20s",
            demo ? "SIM" : "SD",
            prefs.sizeSort ? "size" : "name",
            strlen(dir) > 20 ? dir + strlen(dir) - 20 : dir
        );
        s.row(
            0,
            store.interrupted       ? "Interrupted write | Recover below"
            : listing == Err::Limit ? "Limited directory listing"
                                    : line,
            true
        );
        for (unsigned i = 0; i < count; ++i) {
            snprintf(
                line, sizeof line, "%-12s %3s %lu B", rows[i].name, type(rows[i]), (unsigned long)rows[i].size
            );
            s.item(i, cursor, line);
        }
        for (unsigned i = 0; i < (store.interrupted ? 6u : 5u); ++i)
            s.item(count + i, cursor, browseActions[i]);
    } else if (page == Actions) {
        s.row(0, rows[selected].name, true);
        for (unsigned i = 0; i < 6; ++i) s.item(i, cursor, fileActions[i]);
    } else if (page == Naming) {
        s.row(
            1,
            op == NewDir   ? "Create folder"
            : op == Rename ? "Rename selected item"
                           : "Copy to sibling file"
        );
        s.row(2, name, true);
        s.row(3, "Name: 1-8 letters/digits, .ext");
        s.row(4, "Enter / hold G0 applies this name");
    } else if (page == Recovery) {
        s.row(0, "Recover interrupted file?", true);
        s.item(0, cursor, "Cancel");
        s.item(1, cursor, "Keep as PARTnnnn.TMP");
    } else if (page == Delete) {
        s.row(0, "Delete selected item?", true);
        s.item(0, cursor, "Cancel");
        s.item(1, cursor, rows[selected].name);
        s.row(4, "Directory must be empty.");
    } else if (page == Info) {
        snprintf(line, sizeof line, "Capacity %llu B", (unsigned long long)store.capacity());
        s.row(1, line);
        snprintf(line, sizeof line, "Free %llu B", (unsigned long long)store.freeSpace());
        s.row(2, line);
        s.row(3, demo ? "Volatile data | no SPI lease" : "Module Manager | SPI lease");
        s.row(4, "Owner: Files   /BLACKBOX");
        s.row(5, "Select to return");
    } else if (page == Viewer) {
        uint8_t data[chunk]{};
        int n = store.read(path, pager.pos, data, hexView ? 40 : chunk);
        if (n < 0) {
            notice(describe(store.last));
            draw();
            return;
        }
        snprintf(
            line,
            sizeof line,
            "%s | %s @%lu",
            demo ? "SIM" : "SD",
            rows[selected].name,
            (unsigned long)pager.pos
        );
        s.row(0, line, true);
        unsigned used = 0;
        if (csvView && n) {
            csvNext = csvPage(s, csvCur, data, n);
            used = csvNext.pos - csvCur.pos;
        } else if (!n) s.row(1, "Empty file");
        else if (hexView) {
            for (unsigned y = 1; y <= 5 && used < unsigned(n); ++y) {
                unsigned x = snprintf(line, sizeof line, "%08lX ", (unsigned long)(pager.pos + used));
                for (unsigned j = 0; j < 8 && used < unsigned(n); ++j)
                    x += snprintf(line + x, sizeof(line) - x, " %02X", data[used++]);
                s.row(y, line);
            }
        } else
            used = textPage(s, reinterpret_cast<const char *>(data), n, !strcmp(type(rows[selected]), "CSV"));
        pager.next = (used && pager.pos < rows[selected].size && used < rows[selected].size - pager.pos)
                         ? pager.pos + used
                         : pager.pos;
    } else if (page == Transfer) {
        snprintf(line, sizeof line, "%lu / %lu B", (unsigned long)store.done, (unsigned long)store.length);
        s.row(1, "Copy in progress");
        s.row(2, line);
        s.row(4, "DEL / hold to cancel");
    } else {
        s.row(1, message, true);
        s.row(3, "Select to continue");
    }
    s.show();
}
void startName(Op which) {
    page = Naming;
    op = which;
    snprintf(name, sizeof name, "%s", op == NewDir ? "NEW" : op == Rename ? rows[selected].name : "COPY.BIN");
    bench::textMode(true);
}
bool select(Key k) {
    bool back = k == Key::Back;
    if (page == Sources) {
        if (back || cursor == 2) return false;
        sourceChoice = cursor;
        demo = !cursor;
        Err result = store.mount(demo ? static_cast<Volume &>(ram) : card(), Client::Files);
        if (result != Err::Ok) notice(describe(result));
        else {
            strcpy(dir, root);
            browse();
        }
    } else if (page == Browse) {
        if (back && !strcmp(dir, root)) {
            leave();
            enter();
        } else if (back || cursor == count + 3) {
            parent(dir);
            browse();
        } else if (cursor < count) {
            selected = cursor;
            if (!child(dir, rows[cursor].name, path)) notice("Path too long");
            else {
                page = Actions;
                cursor = 0;
            }
        } else if (cursor == count) startName(NewDir);
        else if (cursor == count + 1) {
            prefs.sizeSort = !prefs.sizeSort;
            browse();
        } else if (cursor == count + 2) page = Info;
        else if (cursor == count + 5) {
            page = Recovery;
            cursor = 0;
        } else {
            leave();
            enter();
        }
    } else if (page == Actions) {
        if (back || cursor == 5) browse();
        else if (cursor < 2) {
            if (rows[selected].dir) {
                if (!cursor) {
                    strcpy(dir, path);
                    browse();
                } else notice("Select a file");
            } else {
                hexView = cursor == 1 || !strcmp(type(rows[selected]), "BIN");
                page = Viewer;
                pager = {};
                csvCur = {};
                csvView = !hexView && !strcmp(type(rows[selected]), "CSV");
            }
        } else if (cursor == 2) startName(Rename);
        else if (cursor == 3 && !rows[selected].dir) startName(Copy);
        else if (cursor == 4) {
            page = Delete;
            cursor = 0;
        } else notice("Copy supports files");
    } else if (page == Naming) {
        if (back && *name) {
            name[strlen(name) - 1] = 0;
        } else {
            bench::textMode(false);
            if (back) browse();
            else {
                char target[pathSize];
                Err e = Err::Invalid;
                if (child(dir, name, target))
                    e = op == NewDir   ? store.makeDir(target)
                        : op == Rename ? store.rename(path, target)
                                       : store.copy(path, target);
                if (e != Err::Ok) notice(describe(e));
                else if (store.busy) page = Transfer;
                else browse();
            }
        }
    } else if (page == Recovery) {
        if (!back && cursor == 1) {
            char kept[13];
            Err e = store.recover(true, kept);
            notice(e == Err::Ok ? kept : describe(e));
        } else browse();
    } else if (page == Delete) {
        if (!back && cursor == 1) {
            Err e = store.erase(path, true);
            if (e != Err::Ok) notice(describe(e));
            else browse();
        } else browse();
    } else if (page == Transfer) {
        store.cancel();
        notice(describe(store.last));
    } else if (store.owner != Client::Files) {
        page = Sources;
        cursor = sourceChoice;
    } else browse();
    draw();
    return true;
}
void input(Key k) {
    if (page == Transfer && k == Key::None) {
        Err e = store.tick();
        if (e != Err::Ok) notice(describe(e));
        else if (!store.busy) browse();
        draw();
        return;
    }
    if (k == Key::None) return;
    if (page == Naming) {
        if (k == Key::Character && strlen(name) < 12) {
            unsigned n = strlen(name);
            name[n] = bench::typed();
            name[n + 1] = 0;
        } else if (k == Key::Step) {
            bench::textMode(false);
            browse();
        }
    } else if (page == Viewer) {
        if (k == Key::Left || k == Key::Up) {
            if (csvView) csvCur = pager.depth ? csvHistory[pager.depth - 1] : CsvCursor{};
            pager.back();
        } else if (pager.next > pager.pos) {
            if (csvView) {
                if (pager.depth == 8) memmove(csvHistory, csvHistory + 1, 7 * sizeof(CsvCursor));
                csvHistory[pager.depth < 8 ? pager.depth : 7] = csvCur;
                csvCur = csvNext;
            }
            pager.forward();
        }
    } else if (page == Info || page == Notice) {
        select(Key::Open);
        return;
    } else if (!move(
                   k,
                   cursor,
                   page == Sources                        ? 3
                   : page == Browse                       ? count + (store.interrupted ? 6 : 5)
                   : (page == Delete || page == Recovery) ? 2
                   : page == Actions                      ? 6
                                                          : 0
               ))
        return;
    draw();
}
void refresh() {
    if (store.owner == Client::Files && !store.ready()) {
        notice("Storage removed");
        draw();
    }
}
} // namespace
const Lifecycle filesApp{
    [] {
        enter();
        draw();
    },
    refresh,
    input,
    leave,
    select
};
