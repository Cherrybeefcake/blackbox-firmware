// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench.h"
#include "device.h"
#include "reports.h"
#include "storage.h"
#include "workspace.h"
#include "workspace_view.h"
#include <cstdio>
#include <cstring>
namespace {
using namespace ws;
enum View {
    Desk,
    Search,
    Favorites,
    Recents,
    Recorder,
    Help,
    Options,
    ManageFav,
    HelpText,
    Sources,
    Preview,
    Export,
    Discard,
    Reset,
    Saving,
    Message
};
View view, returnTo;
unsigned cursor, topic, hitCount, selection;
uint8_t hits[96];
bool editing, csv, sim;
uint16_t wanted = 0;
Report kind;
char query[25], info[40], output[reportLimit], filename[pathSize];
Pager pager;
const char *const desk[] = {
    "Quick Search", "Favorites", "Recent apps", "Session Recorder", "Offline Help", "Preferences", "Dashboard"
};
const char *const options[] = {
    "Theme", "Dashboard shortcut", "Lab / Files source", "RF / NFC / IR source", "Manage favorites", "Reset preferences", "Back"
};
const char *const help[][5] = {
    {"Controls and dashboard",
     "Use ; , . / to move. Enter opens.",      "DEL goes back. Tap G0 to advance;",
     "Hold to select. Hold 2s for Home.",      "Q searches. F opens your shortcut."    },
    {"System Info and Settings",
     "System has Health and a self-test.",     "Settings adjusts light and contrast.",
     "Workspace preferences set the theme",    "and shortcut. Changes save after idle."},
    {"Wi-Fi and Channel Analyzer",
     "Start a passive scan for nearby APs.",   "Choose an authorized AP for the meter.",
     "Channels show up to 32 observed APs.",   "AP counts do not measure airtime."     },
    {"Bluetooth Explorer",
     "Start a bounded passive BLE scan.",      "Select an advertisement for details.",
     "Exit releases radio ownership.",         "Discovery does not establish identity."},
    {"Sub-GHz analysis",
     "Choose the fixture to test RSSI views.", "Step size sets the sweep resolution.",
     "U219 RX needs confirmed ADV wiring.",    "Stop sessions before changing tools."  },
    {"NFC and Infrared",
     "Select a source and start explicitly.",  "NDEF and IR RX fixtures are simulated.",
     "Onboard IR is emitter only.",            "Check accessory revision and voltage." },
    {"Module Manager",
     "Select a module to inspect its pins.",   "Not checked means no presence test.",
     "Active shows an owner. Conflict means",  "another owner holds a shared resource."},
    {"Terminal and Hardware Lab",
     "UART is RX only. Set baud first.",       "Check ground and 3.3 V logic levels.",
     "Lab includes I2C, SPI, GPIO and math.",  "Pinout and references work offline."   },
    {"Simulation and availability",
     "SIM is generated data or RAM storage.",  "Unavailable may mean absent or locked.",
     "Error means a failed check or I/O.",     "Stop the active owner before retrying."},
    {"Files and storage",
     "RAM files last until reboot.",           "Manage short names under /BLACKBOX.",
     "Copy and export publish after writing.", "Confirm deletion. Unmount to remove."  },
    {"Recording and reports",
     "Select sources and Start in Recorder.",  "Run tools, then return and Stop.",
     "Preview TXT or CSV, choose storage,",    "then confirm export. Times use uptime."},
    {"Search and favorites",
     "Find apps, tools, formulas or topics.",  "Type, Enter, scroll, then select.",
     "Manage favorites in Preferences.",       "History saves only application IDs."   },
    {"Troubleshooting and recovery",
     "Back out of busy tools to free leases.", "Check source, power and card format.",
     "Reset preferences for display issues.",  "Recover with a trusted ADV image."     },
    {"Firmware recovery",
     "Back up files. Switch ADV power OFF.",   "Hold G0 while connecting USB-C data.",
     "Release G0 after power-up. Select a",    "trusted ADV image for manual recovery."}
};
void go(View v, unsigned i = 0) {
    bench::textMode(v == Search);
    editing = v == Search;
    view = v;
    cursor = i;
}
void message(const char *m, View back) {
    snprintf(info, sizeof info, "%s", m);
    returnTo = back;
    go(Message);
}
void findHits() {
    hitCount = 0;
    for (unsigned i = 0; i < destinations() && hitCount < 96; ++i)
        if (matches(destination(i), query)) hits[hitCount++] = i;
}
Snapshot snapshot() {
    auto m = bench::metrics();
    unsigned active = 0, conflicts = 0;
    for (unsigned i = 0; i < moduleCount(); ++i) {
        if (modules.slot(i).owner != Owner::None) ++active;
        else if (modules.blocker(i) >= 0) ++conflicts;
    }
    bool mounted = store.owner == Client::Report && store.ready();
    return {
        bench::now(),
        m.mv,
        m.freeHeap,
        m.mhz,
        active,
        conflicts,
        mounted ? store.capacity() : 0,
        mounted ? store.freeSpace() : 0,
        mounted,
        sim
    };
}
void preview(Report r) {
    if (r == Report::Session && (!recorder.exists || recorder.running)) {
        message("Stop a session to preview it", Recorder);
        return;
    }
    kind = r;
    if (!makeReport(kind, csv, snapshot(), recorder, output, sizeof output)) {
        message("Report exceeds buffer", Recorder);
        return;
    }
    pager = {};
    go(Preview);
}
void openDesk(unsigned i) {
    static const View pages[] = {Search, Favorites, Recents, Recorder, Help, Options};
    if (i >= sizeof pages / sizeof *pages) return;
    go(pages[i]);
    if (view == Search) {
        query[0] = 0;
        findHits();
    }
}
unsigned ids(const uint8_t *list) {
    unsigned n = 0;
    while (n < historySize && list[n] != noId) ++n;
    return n;
}
unsigned count() {
    switch (view) {
        case Desk: return 7;
        case Search: return hitCount + 2;
        case Favorites: return ids(prefs.favorites) + 2;
        case Recents: return ids(prefs.recents) + 1;
        case Recorder: return 10;
        case Help: return sizeof help / sizeof *help + 1;
        case Options: return 7;
        case ManageFav: return appCount() + 1;
        case Sources: return Groups + 1;
        case Export:
        case Discard:
        case Reset: return 2;
        default: return 0;
    }
}
void draw() {
    char line[60];
    const char *title = "Workspace", *state = version;
    const char *hint = "G0 next / hold select    DEL back";
    switch (view) {
        case Search:
            title = "Quick Search";
            state = *query ? query : "All apps and tools";
            hint = editing ? "Type | DEL erase | Enter/G0 results" : "G0 next / hold open    DEL edit";
            break;
        case Favorites:
            title = "Favorites";
            state = ids(prefs.favorites) ? "Pinned apps" : "No favorites yet";
            break;
        case Recents:
            title = "Recent apps";
            state = ids(prefs.recents) ? "Latest first" : "No app history";
            break;
        case Options:
            title = "Preferences";
            state = settingsState;
            break;
        case ManageFav:
            title = "Pin favorites";
            state = "Select to add or remove";
            break;
        case Help:
            title = "Offline Help";
            state = "Manual for Cardputer ADV";
            break;
        case HelpText:
            title = "Field guide";
            state = help[topic][0];
            break;
        case Recorder:
            title = "Session Recorder";
            snprintf(
                line,
                sizeof line,
                "%s | %s",
                recorder.running  ? "RECORDING"
                : recorder.exists ? "Stopped"
                                  : "Idle",
                store.owner == Client::Report ? (sim ? "SIM mounted" : "SD mounted") : "Unmounted"
            );
            state = line;
            break;
        case Sources:
            title = "Record sources";
            state = "Select summary groups";
            break;
        case Preview:
            title = csv ? "CSV preview" : "TXT preview";
            state = reportName(kind);
            hint = "G0 next page / hold export   DEL back";
            break;
        case Export:
            title = "Export report?";
            state = filename + sizeof(root);
            break;
        case Reset:
            title = "Reset settings?";
            state = "Restore defaults and clear history";
            break;
        case Discard:
            title = "Discard session?";
            state = "Erase the in-memory session";
            break;
        case Message:
            title = "Workspace";
            state = info;
            break;
        case Saving:
            title = "Writing report";
            state = sim ? "SIM workspace" : "SD workspace";
            break;
        default: break;
    }
    Screen s(title, hint, state);
    if (view == Desk)
        for (unsigned i = 0; i < count(); ++i) s.item(i, cursor, desk[i]);
    else if (view == Search) {
        for (unsigned i = 0; i < hitCount; ++i) s.item(i, cursor, destination(hits[i]).name);
        s.item(hitCount, cursor, "Clear / edit search");
        s.item(hitCount + 1, cursor, "Back");
    } else if (view == Favorites || view == Recents) {
        const uint8_t *list = view == Favorites ? prefs.favorites : prefs.recents;
        unsigned n = ids(list);
        for (unsigned i = 0; i < n; ++i) s.item(i, cursor, appAt(list[i])->name);
        if (view == Favorites) s.item(n++, cursor, "Manage favorites");
        s.item(n, cursor, "Back");
    } else if (view == Options) {
        for (unsigned i = 0; i < count(); ++i) {
            const char *val = i == 0   ? (prefs.theme ? "Mint" : "Emerald")
                              : i == 1 ? appAt(prefs.shortcut)->tile
                              : i == 2 ? (prefs.simulation ? "Simulation" : "Hardware")
                              : i == 3 ? (prefs.accessorySim ? "Simulation" : "Hardware")
                                       : "";
            snprintf(line, sizeof line, "%s %s", options[i], val);
            s.item(i, cursor, line);
        }
    } else if (view == ManageFav) {
        for (unsigned i = 0; i < appCount(); ++i) {
            bool pin = memchr(prefs.favorites, int(i), historySize);
            snprintf(line, sizeof line, "[%c] %s", pin ? 'x' : ' ', appAt(i)->name);
            s.item(i, cursor, line);
        }
        s.item(appCount(), cursor, "Back");
    } else if (view == Help) {
        for (unsigned i = 0; i + 1 < count(); ++i) s.item(i, cursor, help[i][0]);
        s.item(count() - 1, cursor, "Back");
    } else if (view == HelpText) {
        for (unsigned i = 1; i < 5; ++i) s.row(i, help[topic][i]);
    } else if (view == Recorder) {
        const char *items[] = {
            "Select summary sources",
            recorder.running ? "Stop recording" : "Start recording",
            "Preview session summary",
            "Preview device health",
            "Preview storage / resources",
            csv ? "Format: CSV" : "Format: TXT",
            store.owner == Client::Report ? "Unmount storage"
            : sim                         ? "Mount RAM simulation"
                                          : "Mount microSD",
            sim ? "Destination: SIM" : "Destination: SD",
            "Discard session",
            "Back"
        };
        for (unsigned i = 0; i < count(); ++i) s.item(i, cursor, items[i]);
    } else if (view == Sources) {
        for (unsigned i = 0; i < Groups; ++i) {
            snprintf(line, sizeof line, "[%c] %s", wanted & (1 << i) ? 'x' : ' ', groupName(i));
            s.item(i, cursor, line);
        }
        s.item(Groups, cursor, "Back to recorder");
    } else if (view == Preview) {
        unsigned remaining = strlen(output + pager.pos);
        unsigned used = textPage(s, output + pager.pos, remaining, csv);
        pager.next = used < remaining ? pager.pos + used : pager.pos;
    } else if (view == Saving) {
        snprintf(line, sizeof line, "%lu / %lu B", (unsigned long)store.done, (unsigned long)store.length);
        s.row(1, line);
        s.row(3, "DEL cancels the staged write");
    } else if (view == Message) s.row(2, "Select to return");
    else {
        s.item(0, cursor, "Cancel");
        s.item(
            1,
            cursor,
            view == Export  ? (sim ? "Save to SIM" : "Save to microSD")
            : view == Reset ? "Reset preferences"
                            : "Discard"
        );
    }
    s.show();
}
void leave() {
    bench::textMode(false);
    if (store.owner == Client::Report) store.unmount();
    *output = *query = 0;
}
void enter() {
    invalidate();
    sim = prefs.simulation;
    go(Desk);
    draw();
}
bool select(Key k) {
    bool back = k == Key::Back;
    if (view == Desk) {
        if (back || cursor == 6) return false;
        openDesk(cursor);
    } else if (view == Search) {
        if (editing && back && *query) {
            query[strlen(query) - 1] = 0;
            findHits();
            cursor = 0;
        } else if (editing && back) go(Desk);
        else if (editing) {
            editing = false;
            bench::textMode(false);
        } else if (back || cursor == hitCount) {
            if (!back) {
                *query = 0;
                findHits();
            }
            go(Search);
        } else if (cursor == hitCount + 1) go(Desk);
        else launch(destination(hits[cursor]));
    } else if (view == Favorites || view == Recents) {
        const uint8_t *list = view == Favorites ? prefs.favorites : prefs.recents;
        if (back || cursor + 1 == count()) go(Desk);
        else if (cursor == ids(list)) {
            selection = 1;
            go(ManageFav);
        } else launch(destination(list[cursor]));
    } else if (view == Options) {
        if (back || cursor == 6) go(Desk);
        else if (cursor == 0) {
            prefs.theme = !prefs.theme;
            invalidate();
        } else if (cursor == 1) prefs.shortcut = (prefs.shortcut + 1) % appCount();
        else if (cursor == 2) prefs.simulation = !prefs.simulation;
        else if (cursor == 3) prefs.accessorySim=!prefs.accessorySim;
        else if (cursor == 4) {
            selection = 0;
            go(ManageFav);
        } else go(Reset);
    } else if (view == ManageFav) {
        if (back || cursor == appCount()) go(selection ? Favorites : Options);
        else if (!favorite(cursor)) message("Six favorites already pinned", ManageFav);
    } else if (view == Reset) {
        if (!back && cursor == 1) {
            prefs = Prefs{};
            applySettings();
            invalidate();
        }
        go(Options);
    } else if (view == Help) {
        if (back || cursor + 1 == count()) go(Desk);
        else {
            topic = cursor;
            go(HelpText);
        }
    } else if (view == HelpText) go(Help, topic);
    else if (view == Recorder) {
        if (back || cursor == 9) {
            if (store.owner == Client::Report) store.unmount();
            go(Desk);
        } else if (cursor == 0) {
            if (recorder.exists) message("Discard session to change sources", Recorder);
            else go(Sources);
        } else if (cursor == 1) {
            if (recorder.running) recorder.stop(bench::now());
            else if (!recorder.start(wanted, bench::now()))
                message(
                    recorder.exists ? "Save / discard existing session" : "Select at least one source",
                    Recorder
                );
        } else if (cursor < 5) preview(Report(cursor - 2));
        else if (cursor == 5) csv = !csv;
        else if (cursor == 6) {
            if (store.owner == Client::Report) store.unmount();
            else {
                Err e = store.mount(sim ? static_cast<Volume &>(ram) : card(), Client::Report);
                if (e != Err::Ok) message(describe(e), Recorder);
            }
        } else if (cursor == 7) {
            if (store.owner == Client::Report) store.unmount();
            sim = !sim;
        } else if (recorder.exists) go(Discard);
    } else if (view == Sources) {
        if (back || cursor == Groups) go(Recorder);
        else wanted ^= 1u << cursor;
    } else if (view == Discard) {
        if (!back && cursor == 1) recorder = Session{};
        go(Recorder);
    } else if (view == Preview) {
        if (back) go(Recorder);
        else if (store.owner != Client::Report) message("Mount destination before export", Recorder);
        else {
            Err e = store.unique(root, bench::now() / 1000, csv, filename);
            if (e == Err::Ok) go(Export);
            else message(describe(e), Recorder);
        }
    } else if (view == Export) {
        if (back || !cursor) go(Preview);
        else {
            Err e = store.write(filename, reinterpret_cast<const uint8_t *>(output), strlen(output));
            if (e == Err::Ok) go(Saving);
            else message(describe(e), Recorder);
        }
    } else if (view == Saving) {
        store.cancel();
        message(describe(store.last), Recorder);
    } else go(returnTo);
    draw();
    return true;
}
void input(Key k) {
    if (view == Saving && k == Key::None) {
        Err e = store.tick();
        if (e != Err::Ok) message(describe(e), Recorder);
        else if (!store.busy) {
            char msg[40];
            snprintf(msg, sizeof msg, "Saved %s / %.12s", sim ? "SIM" : "SD", strrchr(filename, '/') + 1);
            message(msg, Recorder);
        }
    } else if (view == Search && editing && k == Key::Character) {
        unsigned n = strlen(query);
        if (n + 1 < sizeof query) {
            query[n] = bench::typed();
            query[n + 1] = 0;
        }
        cursor = 0;
        findHits();
    } else if (view == Search && editing && k == Key::Step) {
        editing = false;
        bench::textMode(false);
    } else if (view == Preview && k != Key::None) {
        if (k == Key::Up || k == Key::Left) pager.back();
        else if (pager.next == pager.pos) pager = {};
        else pager.forward();
    } else if (!move(k, cursor, count())) return;
    draw();
}
void refresh() {
    if (store.owner == Client::Report && !store.ready()) {
        message("Storage removed", Recorder);
        draw();
    }
}
} // namespace
namespace ws {
void workspaceDestination(unsigned i) {
    openDesk(i);
    draw();
}
} // namespace ws
const Lifecycle workspaceApp{enter, refresh, input, leave, select};
