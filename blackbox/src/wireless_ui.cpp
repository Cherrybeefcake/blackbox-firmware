// SPDX-License-Identifier: AGPL-3.0-or-later
#include "device.h"
#include "wireless.h"
#include "wireless_view.h"
#include <algorithm>
#include <cstdio>
namespace {
using namespace air;
enum View { Menu, List, Detail, Graph, Confirm, Meter, Busy, Message };
View view;
Owner app;
unsigned cursor, selected, channel;
bool byChannel;
const char *message;
const char *title() {
    return app == Owner::Bluetooth ? "BLUETOOTH" : app == Owner::Channels ? "CHANNELS" : "WI-FI";
}
const char *name(const Record &r) {
    return r.name[0] ? r.name : app == Owner::Bluetooth ? "Unnamed" : "Hidden SSID";
}
void line(unsigned i, const char *s) { airText(4, 25 + 16 * i, s); }
void draw() {
    airFrame(title(), "Tap G0: next  Hold: select  DEL: back");
    char buf[40];
    switch (view) {
        case Menu: {
            const char *actions[] = {"Start passive scan", "Dashboard"};
            for (unsigned i = 0; i < 2; ++i) {
                snprintf(buf, sizeof buf, "%c %s", cursor == i ? '>' : ' ', actions[i]);
                line(i, buf);
            }
            break;
        }
        case Busy:
            line(0, "Listening passively...");
            line(2, "ENTER or hold G0: Cancel");
            snprintf(buf, sizeof buf, "Lowest free heap: %u KiB", minimumHeap / 1024);
            line(4, buf);
            break;
        case List: {
            unsigned n = results.count;
            unsigned first = cursor / 5 * 5;
            unsigned count = n + (app == Owner::Bluetooth ? 2 : 3);
            for (unsigned i = first; i < std::min(first + 5, count); ++i) {
                if (i < n) {
                    auto &r = results.rows[i];
                    snprintf(
                        buf, sizeof buf, "%c %-16.16s %4d dBm", i == cursor ? '>' : ' ', name(r), r.rssi
                    );
                    airRect(
                        195, 28 + 16 * (i - first), std::max(1, std::min(39, (r.rssi + 105) / 2)), 4, true
                    );
                } else {
                    const char *action = i == n           ? "Rescan"
                                         : i == count - 1 ? "Dashboard"
                                         : byChannel      ? "Sort by RSSI"
                                                          : "Sort by channel";
                    snprintf(buf, sizeof buf, "%c %s", i == cursor ? '>' : ' ', action);
                }
                line(i - first, buf);
            }
            break;
        }
        case Detail: {
            auto &r = results.rows[selected];
            line(0, name(r));
            snprintf(
                buf,
                sizeof buf,
                "%d dBm   %02X:%02X:%02X:%02X:%02X:%02X",
                r.rssi,
                r.id[0],
                r.id[1],
                r.id[2],
                r.id[3],
                r.id[4],
                r.id[5]
            );
            line(1, buf);
            if (app == Owner::Bluetooth) {
                snprintf(buf, sizeof buf, "Service: %s", r.service[0] ? r.service : "Not advertised");
                line(2, buf);
                line(4, "> Back to list");
            } else {
                snprintf(buf, sizeof buf, "Ch %u   %s", r.channel, securityName(r.security));
                line(2, buf);
                line(3, cursor == 0 ? "> Lab signal meter" : "  Lab signal meter");
                line(4, cursor == 1 ? "> Back to list" : "  Back to list");
            }
            break;
        }
        case Confirm:
            line(0, "Monitor only an authorized lab AP");
            line(1, name(results.rows[selected]));
            line(3, cursor == 0 ? "> Back" : "  Back");
            line(4, cursor == 1 ? "> Start 30 observations" : "  Start 30 observations");
            break;
        case Graph: {
            unsigned counts[15]{};
            int strongest[15];
            std::fill(strongest, strongest + 15, -128);
            for (unsigned i = 0; i < results.count; ++i) {
                auto &r = results.rows[i];
                if (r.channel <= 14) {
                    ++counts[r.channel];
                    strongest[r.channel] = std::max(strongest[r.channel], int(r.rssi));
                }
            }
            line(0, results.limited ? "Top 32 APs; not airtime" : "Observed APs; not utilization");
            for (unsigned c = 1; c <= 14; ++c) {
                int x = 5 + 16 * (c - 1), h = counts[c] ? 2 + counts[c] * 35 / 32 : 1;
                airRect(x, 76 - h, 10, h, c == channel);
            }
            if (counts[channel])
                snprintf(
                    buf, sizeof buf, "Ch %u: %u APs, max %d dBm", channel, counts[channel], strongest[channel]
                );
            else snprintf(buf, sizeof buf, "Ch %u: no AP observed", channel);
            airText(5, 80, buf);
            airText(
                5,
                99,
                cursor == 0   ? "> Channel   Rescan   Home"
                : cursor == 1 ? "  Channel > Rescan   Home"
                              : "  Channel   Rescan > Home"
            );
            break;
        }
        case Meter: {
            line(0, name(results.rows[selected]));
            for (unsigned i = 0; i < samples.count; ++i) {
                int h = std::max(1, std::min(40, (samples.at(i) + 110) / 2));
                airRect(5 + i * 7, 83 - h, 4, h, true);
            }
            if (samples.count) {
                int lo, hi, avg;
                samples.stats(lo, hi, avg);
                snprintf(buf, sizeof buf, "Min %d Max %d Avg %d dBm", lo, hi, avg);
                line(4, buf);
            } else line(3, "No matching beacon observed");
            airText(4, 104, session.owner == Owner::Meter ? "ENTER: stop" : "Session ended - ENTER: details");
            break;
        }
        case Message:
            line(0, message);
            line(3, cursor == 0 ? "> Rescan" : "  Rescan");
            line(4, cursor == 1 ? "> Dashboard" : "  Dashboard");
            break;
    }
}
void enter(Owner who) {
    app = who;
    view = Menu;
    cursor = 0;
    selected = 0;
    channel = 1;
    byChannel = false;
    draw();
}
void scan() {
    view = begin(app) ? Busy : Message;
    message = error;
    cursor = 0;
    draw();
}
void back() {
    if (view == Busy || view == Meter) cancel();
    if (view == Meter || view == Confirm) {
        view = Detail;
        cursor = 0;
    } else if (view == Detail) {
        view = List;
        cursor = selected;
    } else {
        view = Menu;
        cursor = 0;
    }
    draw();
}
bool open() {
    switch (view) {
        case Menu:
        case Message:
            if (cursor == 1) return false;
            scan();
            return true;
        case Busy: back(); return true;
        case List:
            if (cursor < results.count) {
                selected = cursor;
                view = Detail;
                cursor = 0;
            } else if (cursor == results.count) {
                scan();
                return true;
            } else if (app != Owner::Bluetooth && cursor == results.count + 1) {
                byChannel = !byChannel;
                results.sort(byChannel);
                cursor = 0;
            } else return false;
            break;
        case Detail:
            if (app == Owner::Bluetooth || cursor == 1) {
                back();
                return true;
            }
            view = Confirm;
            cursor = 0;
            break;
        case Confirm:
            if (!cursor) {
                back();
                return true;
            }
            if (begin(Owner::Meter, &results.rows[selected])) view = Meter;
            else {
                view = Message;
                message = error;
            }
            cursor = 0;
            break;
        case Meter:
            if (session.owner == Owner::Meter) {
                cancel();
                draw();
            } else back();
            return true;
        case Graph:
            if (cursor == 2) return false;
            if (cursor == 1) {
                scan();
                return true;
            }
            channel = channel % 14 + 1;
            break;
    }
    draw();
    return true;
}
bool special(Key k) {
    if (k == Key::Hold || k == Key::Open) return open();
    if (view == Menu) return false;
    back();
    return true;
}
void input(Key k) {
    if (k == Key::None) return;
    if (view == Busy) {
        if (k == Key::Step) back();
        return;
    }
    if (view == Meter) return;
    if (view == Graph && (k == Key::Right || k == Key::Left)) {
        channel = (channel - 1 + (k == Key::Right ? 1 : 13)) % 14 + 1;
        draw();
        return;
    }
    unsigned n = view == List    ? results.count + (app == Owner::Bluetooth ? 2 : 3)
                 : view == Graph ? 3
                 : view == Detail && app == Owner::Bluetooth ? 1
                                                             : 2;
    cursor = (cursor + (k == Key::Up || k == Key::Left ? n - 1 : 1)) % n;
    draw();
}
void refresh() {
    if (view == Meter && session.owner == Owner::None && session.phase == Phase::Done) {
        draw();
        return;
    }
    if (view != Busy && view != Meter) return;
    if (session.owner != Owner::None) {
        draw();
        return;
    }
    if (session.phase == Phase::Error || session.phase == Phase::Timeout) {
        view = Message;
        message = session.phase == Phase::Timeout ? "Scan timed out" : error;
    } else if (view == Busy) {
        if (!results.count) {
            view = Message;
            message = "No advertisements observed";
        } else view = app == Owner::Channels ? Graph : List;
        cursor = 0;
        byChannel = false;
    }
    draw();
}
void leave() { forget(); }
} // namespace
const Lifecycle wifiApp = {[]() { enter(Owner::Explorer); }, refresh, input, leave, special};
const Lifecycle channelApp = {[]() { enter(Owner::Channels); }, refresh, input, leave, special};
const Lifecycle bleApp = {[]() { enter(Owner::Bluetooth); }, refresh, input, leave, special};
