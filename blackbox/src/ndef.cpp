// SPDX-License-Identifier: AGPL-3.0-or-later
#include "lab.h"
#include <cstdio>
#include <cstring>
namespace lab {
Decode capability(const uint8_t cc[4], Tag &tag) {
    tag = {};
    if (cc[0] != 0xe1 || cc[1] != 0x10 || !cc[2] || (cc[3] != 0 && cc[3] != 15)) return Decode::Unsupported;
    tag.capacity = 8U * cc[2];
    tag.readOnly = cc[3] == 15;
    return Decode::OK;
}
namespace {
Decode record(Record &out, unsigned tnf, const uint8_t *type, unsigned types, const uint8_t *p, size_t n) {
    const char *prefix = "";
    if (tnf == 1 && types == 1 && type[0] == 'T') {
        if (!n || (p[0] & 0x40) || size_t(1 + (p[0] & 63)) > n) return Decode::Invalid;
        if (p[0] & 0x80) {
            strcpy(out.kind, "UTF16");
            return Decode::OK;
        }
        size_t skip = 1 + (p[0] & 63);
        p += skip;
        n -= skip;
        strcpy(out.kind, "Text");
    } else if (tnf == 1 && types == 1 && type[0] == 'U') {
        static const char *const prefixes[] = {"", "http://www.", "https://www.", "http://", "https://"};
        if (!n) return Decode::Invalid;
        if (*p >= 5) {
            strcpy(out.kind, "Other URI");
            return Decode::OK;
        }
        prefix = prefixes[*p++];
        --n;
        strcpy(out.kind, "URI");
    } else {
        strcpy(out.kind, "Other");
        return Decode::OK;
    }
    strcpy(out.text, prefix);
    size_t at = strlen(prefix);
    for (size_t i = 0; i < n; ++i) {
        if (at + 1 == sizeof(out.text)) {
            out.clipped = true;
            break;
        }
        out.text[at++] = p[i] >= 32 && p[i] <= 126 ? char(p[i]) : '?';
    }
    out.text[at] = 0;
    return Decode::OK;
}
} // namespace
Decode ndef(const uint8_t *p, size_t n, Tag &tag) {
    tag.count = 0;
    if (!n || n > maxMessage) return n ? Decode::Unsupported : Decode::Invalid;
    size_t i = 0;
    unsigned count = 0;
    while (i < n) {
        if (count == maxRecords) return Decode::Unsupported;
        if (n - i < 3) return Decode::Invalid;
        unsigned flags = p[i++], types = p[i++], tnf = flags & 7;
        if (bool(flags & 0x80) != (count == 0) || tnf == 7) return Decode::Invalid;
        if ((flags & 0x20) || tnf == 6) return Decode::Unsupported;
        uint32_t length = 0;
        unsigned lenBytes = flags & 0x10 ? 1 : 4;
        if (n - i < lenBytes) return Decode::Invalid;
        while (lenBytes--) length = (length << 8) | p[i++];
        unsigned ids = 0;
        if (flags & 8) {
            if (i == n) return Decode::Invalid;
            ids = p[i++];
        }
        if (n - i < types + ids || length > n - i - types - ids) return Decode::Invalid;
        if ((tnf == 0 && (types || length || (flags & 8))) || (tnf == 5 && types) ||
            (tnf >= 1 && tnf <= 4 && !types))
            return Decode::Invalid;
        auto &r = tag.records[count];
        r = Record{};
        auto result = record(r, tnf, p + i, types, p + i + types + ids, length);
        if (result != Decode::OK) return result;
        i += types + ids + length;
        ++count;
        if (flags & 0x40) {
            if (i != n) return Decode::Invalid;
            tag.count = count;
            return Decode::OK;
        }
    }
    return Decode::Invalid;
}
Decode type2(const uint8_t *p, size_t n, Tag &tag) {
    tag.count = 0;
    size_t i = 0;
    while (i < n && p[i] == 0) ++i;
    if (i == n) return n >= tag.capacity ? Decode::Invalid : Decode::More;
    if (p[i] == 0xfe) return Decode::OK;
    if (p[i++] != 3) return Decode::Unsupported; // No traversal into reserved or proprietary TLVs.
    if (i == n) return Decode::More;
    unsigned size = p[i++];
    if (size == 255) return Decode::Unsupported;
    if (i + size > tag.capacity) return Decode::Invalid;
    if (size > maxMessage) return Decode::Unsupported;
    if (size > n - i) return Decode::More;
    return size ? ndef(p + i, size, tag) : Decode::OK;
}
Phase demoTag(Tag &tag) {
    const uint8_t cc[] = {0xe1, 0x10, 18, 15};
    const uint8_t sample[] = {3,   29,   0x91, 1,  11,  'T', 2,   'e', 'n', 'L', 'a', 'b', ' ', 'n', 'o', 't',
                              'e', 0x51, 1,    10, 'U', 4,   'l', 'a', 'b', '.', 'l', 'o', 'c', 'a', 'l'};
    capability(cc, tag);
    return type2(sample, sizeof(sample), tag) == Decode::OK ? Phase::Complete : Phase::Error;
}
} // namespace lab
