// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab.h"
namespace lab {
unsigned Session::slot() const {
    return source == Source::Cap ? capSlot : source == Source::Grove ? groveSlot : irSlot;
}
const char *label(Phase p) {
    const char *const names[] = {
        "Ready",
        "Running",
        "Complete",
        "Stopped",
        "Timed out",
        "Profile unavailable",
        "Resource conflict",
        "Read / test error",
        "Unsupported tag"
    };
    return names[unsigned(p)];
}
void Session::finish(Phase next) {
    if (leased) {
        modules.release(slot(), Owner::Application);
        if (!infrared && readerStatus(source).standbyFailed) next = Phase::Error;
    }
    leased = false;
    phase = next;
    if (next != Phase::Complete) tag = Tag{};
}
void Session::start(Source choice, bool ir, unsigned now) {
    finish(Phase::Idle);
    source = choice;
    infrared = ir;
    began = now;
    if (choice != Source::Simulation) {
        if (ir != (choice == Source::Emitter)) {
            finish(Phase::Error);
            return;
        }
        if (modules.blocker(slot()) >= 0) {
            finish(Phase::Conflict);
            return;
        }
        auto result = modules.acquire(slot(), Owner::Application);
        if (result != State::Active) {
            finish(
                result == State::Conflict      ? Phase::Conflict
                : result == State::Unavailable ? Phase::Unavailable
                                               : Phase::Error
            );
            return;
        }
        leased = true;
    }
    phase = Phase::Running;
}
void Session::tick(unsigned now) {
    if (phase != Phase::Running) return;
    auto elapsed = now - began;
    if (infrared) {
        // RMT stops the actual burst in hardware even if the UI misses a tick.
        if (source == Source::Simulation ? elapsed >= 2 : burstFinished()) finish(Phase::Complete);
        else if (elapsed >= irLimitMs) finish(Phase::Timeout);
    } else if (elapsed >= nfcLimitMs) finish(Phase::Timeout);
    else if (source == Source::Simulation) {
        if (elapsed >= 100) finish(demoTag(tag));
    } else {
        Phase next = pollTag(tag);
        if (next != Phase::Running) finish(next);
    }
}
} // namespace lab
