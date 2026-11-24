// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "modules.h"
#include <stddef.h>
namespace lab {
enum class Source { Cap, Grove, Emitter, Simulation };
enum class Phase { Idle, Running, Complete, Stopped, Timeout, Unavailable, Conflict, Error, Unsupported };
constexpr unsigned capSlot = 6, groveSlot = 9, irSlot = 10;
constexpr unsigned nfcLimitMs = 3000, irLimitMs = 20, carrierHz = 38000, markUs = 1000;
constexpr size_t maxMessage = 128, maxRecords = 4;
struct Record {
    char kind[10]{}, text[65]{};
    bool clipped = false;
};
struct Tag {
    unsigned capacity = 0, count = 0;
    bool readOnly = false;
    Record records[maxRecords]{};
};
enum class Decode { OK, More, Invalid, Unsupported };
Decode capability(const uint8_t cc[4], Tag &tag);
Decode ndef(const uint8_t *bytes, size_t length, Tag &tag);
Decode type2(const uint8_t *bytes, size_t available, Tag &tag);
Phase demoTag(Tag &);
struct ReaderStatus {
    int chip = -1;
    bool standbyFailed = false;
};
const ReaderStatus &readerStatus(Source);
Phase pollTag(Tag &);
State capStart();
State groveStart();
void readerStop();
State emitterStart();
void emitterStop();
bool burstFinished();
unsigned nowMs();
struct Session {
    Source source = Source::Cap;
    Phase phase = Phase::Idle;
    bool infrared = false, leased = false;
    unsigned began = 0;
    Tag tag{};
    unsigned slot() const;
    void start(Source choice, bool ir, unsigned now);
    void tick(unsigned now);
    void finish(Phase next);
};
const char *label(Phase);
} // namespace lab
