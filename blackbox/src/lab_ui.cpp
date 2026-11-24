// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab.h"
#include "platform.h"
#include "wireless_view.h"
#include <cstdio>
#include <cstring>
namespace {
using namespace lab;
Session session;
unsigned page, action, index;
bool ir, detail;
Source source;
const char *const nfcMenu[] = {"Tag Inspector", "Public NDEF", "Reader diagnostics", "Source", "Dashboard"};
const char *const irMenu[] = {"Carrier test", "Receiver fixture", "Emitter status", "Source", "Dashboard"};
const char *name() { return (ir ? irMenu : nfcMenu)[page]; }
const char *sourceName() {
    switch (source) {
        case Source::Cap: return "U219 SPI";
        case Source::Grove: return "U216 I2C";
        case Source::Emitter: return "ADV emitter";
        default: return "SIMULATED";
    }
}
void status() {
    if (source == Source::Simulation) {
        airText(4, 47, ir ? "Fixture: synthetic optical edges" : "Fixture: public Type 2 tag");
        airText(4, 62, "Fixture uses no physical bus");
        return;
    }
    unsigned slot = source == Source::Cap ? capSlot : source == Source::Grove ? groveSlot : irSlot;
    const Profile *p = modules.profile(slot);
    if (!p) return;
    char line[40];
    if (!ir && readerStatus(source).chip >= 0) {
        snprintf(line, sizeof line, "%s   chip 0x%02X", p->name, readerStatus(source).chip);
        airText(4, 44, line, true);
    } else airText(4, 44, p->name, true);
    snprintf(line, sizeof line, "%s | %s", busName(p->bus), ownerName(modules.slot(slot).owner));
    airText(4, 57, line);
    int blocked = modules.blocker(slot);
    airText(
        4,
        71,
        blocked >= 0 ? modules.profile(blocked)->name
        : !p->start  ? "Awaiting bench confirmation"
                     : stateName(modules.slot(slot).state)
    );
    if (!ir && readerStatus(source).standbyFailed) {
        airText(4, 85, "Field off unconfirmed");
        return;
    }
    airText(
        4,
        85,
        ir                      ? "GPIO44 TX | external RX needed"
        : source == Source::Cap ? "CS6 IRQ4 | shared cap power G3"
                                : "G2 SDA / G1 SCL | addr 0x50"
    );
}
void irView() {
    if (page == 1 && source != Source::Simulation) {
        airText(4, 50, "External receiver required");
        airText(4, 68, "Choose SIMULATED for RX fixture");
        return;
    }
    if (page == 1) {
        if (session.phase != Phase::Complete) {
            airText(4, 54, "Start synthetic edge measurement");
            return;
        }
        // Deterministic edge timestamps in microseconds, independent of the TX peripheral.
        constexpr unsigned edges[] = {10, 36, 62, 88, 114, 140, 166, 192, 218};
        char line[40];
        snprintf(line, sizeof line, "%u Hz | %u rising edges", 8000000 / (edges[8] - edges[0]), 9U);
        airText(4, 45, line);
        airText(4, 62, "Synthetic interval: 26 us");
        for (auto edge : edges) airRect(5 + edge, 82, 2, 9, true);
    } else {
        airText(4, 45, "Command: 38 kHz | 33% duty");
        airText(4, 61, "One 1 ms burst | envelope");
        airRect(5, 84, 18, 1, false);
        airRect(23, 75, 1, 10, true);
        airRect(24, 75, 86, 1, true);
        airRect(110, 75, 1, 10, true);
        airRect(111, 84, 110, 1, false);
        airText(4, 91, "Optical feedback: external only");
    }
}
void draw() {
    const char *menu = ir ? "Infrared Lab" : "NFC Center";
    airFrame(detail ? name() : menu, "G0 next/hold select   DEL back");
    char line[40];
    if (!detail) {
        for (unsigned i = 0; i < 5; ++i) {
            snprintf(
                line,
                sizeof line,
                "%c %s%s%s",
                i == page ? '>' : ' ',
                (ir ? irMenu : nfcMenu)[i],
                i == 3 ? "  " : "",
                i == 3 ? sourceName() : ""
            );
            airText(4, 25 + 16 * i, line, i == page);
        }
        return;
    }
    snprintf(line, sizeof line, "%s | %s", sourceName(), label(session.phase));
    airText(4, 25, line, true);
    if (page == 2) status();
    else if (session.phase == Phase::Unavailable) {
        auto profile = modules.profile(session.slot());
        airText(
            4,
            51,
            (!profile || !profile->start)      ? "Physical profile awaits bench check"
            : modules.identity() == Board::Adv ? "Check connection and chip identity"
                                               : "Cardputer ADV unconfirmed"
        );
        airText(4, 70, "Use Source to select SIMULATED");
    } else if (session.phase == Phase::Unsupported) {
        airText(4, 52, "Requires public Type 2 NDEF");
        airText(4, 68, "Unsupported tag or data layout");
    } else if (ir) irView();
    else if (session.phase != Phase::Complete) {
        airText(4, 52, "Start with one public NFC-A tag");
        airText(4, 68, "Type 2 | 3 second limit");
    } else if (page == 0) {
        airText(4, 44, "NFC-A / NFC Forum Type 2");
        snprintf(line, sizeof line, "Public data area: %u bytes", session.tag.capacity);
        airText(4, 59, line);
        snprintf(
            line,
            sizeof line,
            "%s | %u NDEF records",
            session.tag.readOnly ? "Read-only tag" : "Writable tag",
            session.tag.count
        );
        airText(4, 74, line);
    } else if (!session.tag.count) airText(4, 52, "No public NDEF records");
    else {
        const auto &r = session.tag.records[index % session.tag.count];
        snprintf(
            line,
            sizeof line,
            "%u/%u  %s%s",
            index % session.tag.count + 1,
            session.tag.count,
            r.kind,
            r.clipped ? "  (excerpt)" : ""
        );
        airText(4, 44, line, true);
        snprintf(line, sizeof line, "%.32s", r.text);
        airText(4, 60, line);
        if (strlen(r.text) > 32) airText(4, 74, r.text + 32);
        else if (!r.text[0]) airText(4, 74, "No displayable text");
    }
    const char *actions[] = {session.phase == Phase::Running ? "Stop" : "Start", "Next record", "Back"};
    snprintf(line, sizeof line, "> %s", actions[action]);
    airText(4, 103, line, true);
}
void leave() {
    session.finish(Phase::Idle);
    index = 0;
}
void enter(bool infrared) {
    leave();
    ir = infrared;
    source = ir ? Source::Emitter : Source::Cap;
    page = action = 0;
    detail = false;
    draw();
}
bool canStart() { return page != 2 && !(ir && page == 1 && source != Source::Simulation); }
bool nextRecord() { return !ir && page == 1 && session.tag.count > 1; }
void input(Key k) {
    auto phase = session.phase;
    session.tick(nowMs());
    if (phase != session.phase) draw();
    bool prev = k == Key::Left || k == Key::Up;
    if (!prev && k != Key::Right && k != Key::Down && k != Key::Step) return;
    if (!detail) page = (page + (prev ? 4 : 1)) % 5;
    else do {
            action = (action + (prev ? 2 : 1)) % 3;
        } while ((action == 1 && !nextRecord()) || (action == 0 && !canStart()));
    draw();
}
bool select(Key k) {
    if (k == Key::Back || (detail && action == 2)) {
        if (!detail) return false;
        if (session.phase == Phase::Running) leave();
        detail = false;
    } else if (!detail) {
        if (page == 4) return false;
        if (page == 3) {
            leave();
            if (ir) source = source == Source::Emitter ? Source::Simulation : Source::Emitter;
            else
                source = source == Source::Cap     ? Source::Grove
                         : source == Source::Grove ? Source::Simulation
                                                   : Source::Cap;
        } else {
            if (ir) leave();
            detail = true;
            action = canStart() ? 0 : 2;
        }
    } else if (action == 1) ++index;
    else if (session.phase == Phase::Running) session.finish(Phase::Stopped);
    else {
        index = 0;
        session.start(source, ir, nowMs());
    }
    draw();
    return true;
}
void idle() {}
} // namespace
const Lifecycle nfcApp{[] { enter(false); }, idle, input, leave, select};
const Lifecycle infraredApp{[] { enter(true); }, idle, input, leave, select};
