// SPDX-License-Identifier: AGPL-3.0-or-later
#include "bench_offline.h"
#include "platform.h"
#include "workspace.h"
#include <cstring>
#include <strings.h>
namespace ws {
namespace {
const Destination tools[] = {
    {11, Kind::Lab,       0, "I2C Inspector",         "grove bus ack address"          },
    {11, Kind::Lab,       1, "SPI Diagnostics",       "bus selects loopback"           },
    {11, Kind::Lab,       2, "GPIO Monitor",          "edges logic levels"             },
    {11, Kind::Lab,       3, "Calculators",           "electronics math"               },
    {11, Kind::Lab,       4, "Pinout Library",        "expansion wiring"               },
    {11, Kind::Lab,       5, "Electronics Reference", "circuits"                       },
    {11, Kind::Lab,       6, "System Profiler",       "memory heap uptime cpu"         },
    {9,  Kind::Terminal,  0, "UART Monitor",          "serial terminal"                },
    {9,  Kind::Terminal,  1, "ASCII Viewer",          "serial text"                    },
    {9,  Kind::Terminal,  2, "Hex Viewer",            "serial bytes"                   },
    {9,  Kind::Terminal,  3, "Serial Config",         "uart baud"                      },
    {9,  Kind::Terminal,  4, "Session Stats",         "uart counter"                   },
    {4,  Kind::Lab,       0, "Spectrum Analyzer",     "sub-ghz sweep"                  },
    {4,  Kind::Lab,       1, "Frequency Monitor",     "rf energy"                      },
    {4,  Kind::Lab,       2, "RSSI Meter",            "sub-ghz signal"                 },
    {4,  Kind::Lab,       3, "Activity History",      "sub-ghz energy"                 },
    {4,  Kind::Lab,       4, "Module Diagnostics",    "cc1101"                         },
    {5,  Kind::Lab,       0, "Tag Inspector",         "nfc public"                     },
    {5,  Kind::Lab,       1, "Public NDEF",           "nfc record"                     },
    {5,  Kind::Lab,       2, "Reader Diagnostics",    "nfc"                            },
    {6,  Kind::Lab,       0, "Carrier Test",          "infrared emitter"               },
    {6,  Kind::Lab,       1, "Receiver Fixture",      "infrared simulation"            },
    {6,  Kind::Lab,       2, "Emitter Status",        "infrared"                       },
    {12, Kind::Workspace, 1, "Favorites",             "bookmarks shortcuts"            },
    {12, Kind::Workspace, 2, "Recent Apps",           "history"                        },
    {12, Kind::Workspace, 3, "Session Recorder",      "summary preview export txt csv" },
    {12, Kind::Workspace, 4, "Offline Help",          "manual recovery troubleshooting"},
};
const char *const keys[] = {
    "battery voltage firmware",
    "display brightness contrast",
    "wifi signal meter access points",
    "ble advertisements",
    "radio spectrum",
    "ndef tag",
    "optical",
    "resource ownership peripheral",
    "wifi channel graph",
    "uart serial",
    "storage sd hex csv text",
    "gpio pinout",
    "search favorites recent session report manual"
};
constexpr unsigned toolCount = sizeof(tools) / sizeof(*tools);
bool contains(const char *s, const char *q) {
    size_t n = strlen(q);
    if (!n) return true;
    for (; *s; ++s)
        if (!strncasecmp(s, q, n)) return true;
    return false;
}
} // namespace
unsigned destinations() {
    return appCount() + toolCount + unsigned(bench::Math::Count) + bench::articleCount(true) +
           bench::articleCount(false);
}
Destination destination(unsigned i) {
    if (i < appCount())
        return {uint8_t(i), Kind::App, 0, appAt(i)->name, i < sizeof(keys) / sizeof(*keys) ? keys[i] : ""};
    i -= appCount();
    if (i < toolCount) return tools[i];
    i -= toolCount;
    if (i < unsigned(bench::Math::Count))
        return {
            labId, Kind::Formula, uint8_t(i), bench::formula(bench::Math(i)).name, "electronics calculator"
        };
    i -= unsigned(bench::Math::Count);
    bool pins = i < bench::articleCount(true);
    if (!pins) i -= bench::articleCount(true);
    if (!pins && i >= bench::articleCount(false)) return {noId, Kind::App, 0, "", ""};
    return {
        labId,
        pins ? Kind::Pins : Kind::Reference,
        uint8_t(i),
        bench::article(pins, i).rows[0],
        pins ? "pinout wiring" : "reference"
    };
}
bool matches(const Destination &d, const char *q) {
    return q && (contains(d.name, q) || contains(d.keywords, q));
}
} // namespace ws
