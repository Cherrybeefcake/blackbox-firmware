# Cardputer ADV hardware baseline

Source configuration, pending bench verification. The ESP32-S3 target uses 8 MiB flash, qio_qspi memory mode and a 135 × 240 ST7789 panel rotated to landscape. The keyboard controller at address 0x34 on internal I²C selects ADV behavior after settings load. Failure selects the classic keyboard path.

| Resource | Configured assignment or constraint |
|---|---|
| Internal keyboard | SDA 8, SCL 9, IRQ 11. Reserve these for the system bus. |
| Grove I²C | SDA 2, SCL 1. Address discovery initiates bus transactions. |
| SD SPI | SCK 40, MISO 39, MOSI 14, CS 12. Check all three bus signals when sharing. |
| ADV GPS / expansion | GPS RX 15 overlaps a Cap preset signal. NFC CS 6 and IRQ 4 overlap another expansion preset. A saved preset identifies intended wiring only. |

Existing UART, I²C, SPI and continuity diagnostics can transmit or drive pins. “Multimeter” implements continuity. The logic analyzer source exists with menu and serial registration disabled.

[Board definition](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/boards/m5stack-cardputer/m5stack-cardputer.ini) · [Board detection and defaults](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/boards/m5stack-cardputer/interface.cpp#L96-L238) · [Wire Tools](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/menu_items/WireToolsMenu.cpp#L8-L22)
