# BLACKBOX RC1 source and component notices

The independent application and build instructions are available at the firmware revision below. The repository retains the original Bruce attribution, including the origin of the ADV keyboard mapping. The PlatformIO source allowlist compiles the independent project. The top-level repository also contains upstream components with their own notices. [Corresponding release source](https://github.com/Cherrybeefcake/blackbox-firmware/tree/8332ac614e068fb9dd33f70279ed32a50912a72a) · [AGPL-3.0-or-later](notices/BLACKBOX-AGPL.txt) · [Repository attributions](../../../THIRD_PARTY.md) · [Build and relink instructions](../../README.md)

| Component | Source and notice |
|---|---|
| TFT_eSPI 2.5.43 | [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) · [Packaged notices](notices/TFT_eSPI.txt) |
| Adafruit TCA8418 1.0.2 | [Adafruit source](https://github.com/adafruit/Adafruit_TCA8418) · [BSD notice](notices/Adafruit-TCA8418.txt) |
| Adafruit BusIO 1.17.2 | [Adafruit source](https://github.com/adafruit/Adafruit_BusIO) · [MIT notice](notices/Adafruit-BusIO.txt) |
| Arduino-ESP32 2.0.17 and bundled Arduino libraries | [Framework distribution](https://github.com/espressif/arduino-esp32/tree/2.0.17) · [LGPL text](notices/Arduino-ESP32-LGPL.md) |
| ESP-IDF 4.4.7, SDK and prebuilt components | The distributed SDK includes components with individual notices. Consult the corresponding component directories and toolchain for their terms. [ESP-IDF source and submodules](https://github.com/espressif/esp-idf/tree/v4.4.7/components) · [SDK license](notices/ESP-IDF-Apache.txt) |

Package and library versions used to rebuild the application are recorded in [flash-layout.json](flash-layout.json). The application is distributed for physical qualification with the standard accessory locks enabled.
