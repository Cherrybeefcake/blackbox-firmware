// SPDX-License-Identifier: AGPL-3.0-or-later
#include "storage_sd.h"
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
namespace ws {
namespace {
// Only the Module Manager callbacks own the second SPI controller and chip selects.
SPIClass &expansion() { static SPIClass bus(FSPI); return bus; }

class CardVolume final : public Volume {
public:
    Err mount() override { return claimCard(modules); }
    void close() override { modules.release(sdSlot, Owner::Application); }
    bool online() override { return SD.cardType() != CARD_NONE && SD.exists(root); }
    uint64_t capacity() override { return SD.totalBytes(); }
    uint64_t freeSpace() override {
        auto total = capacity(), used = SD.usedBytes();
        return used <= total ? total - used : 0;
    }
    bool stat(const char *p, Entry &e) override {
        File f = SD.open(p);
        if (!f) return false;
        e.dir = f.isDirectory();
        e.size = f.size();
        return true;
    }
    Err list(const char *p, Entry *rows, unsigned &count) override {
        File dir = SD.open(p);
        if (!dir || !dir.isDirectory()) return Err::Io;
        count = 0;
        // Bound directory work even when media contains unsupported names.
        for (unsigned inspected = 0; inspected < 64; ++inspected) {
            File f = dir.openNextFile();
            if (!f) return online() ? Err::Ok : Err::Missing;
            if (!validName(f.name())) continue;
            if (count == listLimit) return Err::Limit;
            auto &e = rows[count++];
            strcpy(e.name, f.name());
            e.dir = f.isDirectory();
            e.size = f.size();
        }
        return Err::Limit;
    }
    bool create(const char *p, bool dir) override {
        if (SD.exists(p)) return false;
        return dir ? SD.mkdir(p) : bool(SD.open(p, FILE_WRITE));
    }
    bool rename(const char *from, const char *to) override { return !SD.exists(to) && SD.rename(from, to); }
    bool erase(const char *p, bool dir) override { return dir ? SD.rmdir(p) : SD.remove(p); }
    int read(const char *p, uint32_t pos, uint8_t *buf, unsigned n) override {
        File f = SD.open(p);
        return f && !f.isDirectory() && f.seek(pos) ? int(f.read(buf, n)) : -1;
    }
    int append(const char *p, const uint8_t *buf, unsigned n) override {
        File f = SD.open(p, FILE_APPEND);
        return f ? int(f.write(buf, n)) : -1;
    }
};
} // namespace
Volume &card() {
    static CardVolume volume;
    return volume;
}
State sdStart() {
    for (int pin : {5, 6, 12}) {
        digitalWrite(pin, HIGH);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
    expansion().begin(40, 39, 14, 12);
    if (!SD.begin(12, expansion(), 4000000, "/sd", 4, false)) return State::Unavailable;
    if (!SD.exists(root) && !SD.mkdir(root)) return State::Error;
    File dir = SD.open(root);
    return dir && dir.isDirectory() ? State::Available : State::Error;
}
void sdStop() {
    SD.end();
    expansion().end();
    for (int pin : {5, 6, 12, 14, 39, 40}) pinMode(pin, INPUT);
}
} // namespace ws
