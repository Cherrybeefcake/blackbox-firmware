// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <stddef.h>
#include <stdint.h>
namespace ws {
// Registry positions are stable persisted app IDs. Append new IDs at the end.
constexpr uint8_t filesId = 10, labId = 11, deskId = 12, noId = 255, historySize = 6;
struct Prefs {
    uint8_t brightness = 2, contrast = 0, theme = 0, shortcut = deskId, simulation = 1, baud = 0, fastBus = 0,
            sizeSort = 0, accessorySim = 0;
    uint8_t favorites[historySize] = {noId, noId, noId, noId, noId, noId};
    uint8_t recents[historySize] = {noId, noId, noId, noId, noId, noId};
};
extern Prefs prefs;
constexpr unsigned packetSize = 3 + sizeof(Prefs) + 4;
void encode(const Prefs &, uint8_t (&packet)[packetSize]);
bool decode(Prefs &, const uint8_t *, size_t);
bool favorite(uint8_t);
void recent(uint8_t);
enum class Kind : uint8_t { App, Lab, Terminal, Formula, Pins, Reference, Workspace };
struct Destination {
    uint8_t app;
    Kind kind;
    uint8_t item;
    const char *name;
    const char *keywords;
};
unsigned destinations();
Destination destination(unsigned i);
bool matches(const Destination &, const char *query);
void launch(Destination);
void benchDestination(Kind, unsigned);
void workspaceDestination(unsigned);
void loadSettings();
void serviceSettings(uint32_t now);
void applySettings();
extern const char *settingsState;
} // namespace ws
