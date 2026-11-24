// SPDX-License-Identifier: AGPL-3.0-or-later
#include "platform.h"
static const App apps[] = {
    {"System Info",        "System",    {126, 66, 90, 90, 66, 126, 24, 60},   Panel,      &systemApp  },
    {"Settings",           "Settings",  {8, 126, 8, 0, 32, 126, 32, 0},       Panel,      &settingsApp},
    {"Wi-Fi Explorer",     "Wi-Fi",     {0, 126, 129, 60, 66, 24, 0, 24},     Wifi,       &wifiApp    },
    {"Bluetooth Explorer", "Bluetooth", {16, 24, 84, 56, 56, 84, 24, 16},     Bluetooth,  &bleApp     },
    {"Sub-GHz analysis",   "Sub-GHz",   {16, 84, 84, 56, 16, 16, 16, 56},     Panel,      &signalApp     },
    {"NFC Center",         "NFC",       {126, 66, 98, 82, 74, 70, 66, 126},   Panel,      &nfcApp     },
    {"Infrared Lab",       "Infrared",  {0, 66, 36, 24, 24, 36, 66, 0},       Panel,      &infraredApp     },
    {"Module Manager",     "Hardware",  {36, 126, 66, 126, 66, 126, 36, 0},   Panel,      &modulesApp },
    {"Channel Analyzer",   "Analyzer",  {0, 8, 8, 20, 84, 162, 2, 0},         Wifi,       &channelApp },
    {"Terminal",           "Terminal",  {126, 66, 98, 82, 98, 78, 66, 126},   SerialPort, nullptr     },
    {"Files",              "Files",     {0, 112, 136, 254, 130, 130, 254, 0}, Storage,    nullptr     },
    {"Lab tools",          "Lab",       {60, 24, 24, 36, 66, 126, 126, 0},    Panel,      nullptr     }
};
unsigned appCount() { return sizeof(apps) / sizeof(apps[0]); }
const App *appAt(unsigned index) { return index < appCount() ? &apps[index] : nullptr; }
