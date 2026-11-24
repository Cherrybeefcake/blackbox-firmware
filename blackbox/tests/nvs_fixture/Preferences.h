// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <cstring>
namespace fakeNvs {
extern unsigned size, writes;
extern bool fail;
extern unsigned char bytes[100];
} // namespace fakeNvs
struct Preferences {
    bool begin(const char *ns, bool) { return !strcmp(ns, "blackbox"); }
    bool isKey(const char *) { return fakeNvs::size != 0; }
    size_t getBytesLength(const char *) { return fakeNvs::size; }
    size_t getBytes(const char *, void *p, size_t n) {
        memcpy(p, fakeNvs::bytes, n);
        return n;
    }
    size_t putBytes(const char *, const void *p, size_t n) {
        ++fakeNvs::writes;
        if (fakeNvs::fail) return 0;
        memcpy(fakeNvs::bytes, p, n);
        fakeNvs::size = n;
        return n;
    }
    void end() {}
};
