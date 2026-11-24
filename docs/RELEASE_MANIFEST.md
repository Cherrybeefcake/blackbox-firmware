# BLACKBOX OS 0.9.0 RC1 release manifest

| Field | Distributed artifact |
|---|---|
| Target | M5Stack Cardputer ADV K132-Adv, ESP32-S3, 8 MiB flash |
| Configuration | `blackbox-cardputer-adv`, standard locked accessory configuration |
| Source revision | `8332ac614e068fb9dd33f70279ed32a50912a72a`, clean firmware inputs |
| Visible build ID | `8332ac614e06` |
| Baseline revision | `227debedea2ae7c42982957b77497f7561341225`, BLACKBOX OS 0.8.0 |
| Qualification | Host-tested and clean-built on macOS on 2026-11-24. Physical installation, recovery and hardware acceptance pending. |

The release branch includes a subsequent documentation and packaging commit. The embedded ID identifies the last change to the firmware sources, build hooks, configuration or partition CSV. The distributed binaries are the exact locally verified files. [Machine-readable manifest](../blackbox/release/rc1/flash-layout.json) · [Package checksums](../blackbox/release/rc1/SHA256SUMS)

| File | Required offset | Bytes | Whole-file SHA-256 |
|---|---:|---:|---|
| [bootloader.bin](../blackbox/release/rc1/bootloader.bin) | `0x0000` | 15,104 | `1776e4dd896a69d0a5c2e79957b0e2a88aa4129b1381d6478683515a1f6af343` |
| [partitions.bin](../blackbox/release/rc1/partitions.bin) | `0x8000` | 3,072 | `40dc554ae4f93b09fa057db852ff05b3e03d42167fe65a490496def94b3648ec` |
| [firmware.bin](../blackbox/release/rc1/firmware.bin) | `0x10000` | 1,339,584 | `76f9a0c6201c6b435e603dbc55cd051e65cd6729187bf5ad21303d69010a2a33` |

The build hook verifies the bootloader and partition offsets in the generated uploader arguments and the factory offset in PlatformIO's application environment. The offline check independently decodes the binary partition entries and MD5 record. Esptool 4.5.1 also validated the application's embedded checksum and image hash offline. The flash layout JSON SHA-256 is `8f2714bddf6240630311811226eb60ba7fe19df8faa115cff4c9cf815d02617b`.

| Partition | Offset | Capacity | Use |
|---|---:|---:|---|
| nvs | `0x9000` | `0x6000` | Existing preferences |
| factory | `0x10000` | `0x4e0000` | 5,111,808 bytes for the application |
| spiffs | `0x4f0000` | `0x300000` | Existing data partition |
| coredump | `0x7f0000` | `0x10000` | Legacy reservation, crash writer disabled |

### Resolved build environment

| Tool or compiled dependency | Version |
|---|---|
| Python / PlatformIO Core / Espressif32 platform | 3.9.6 / 6.2.0 / 6.10.0 |
| Arduino-ESP32 / ESP-IDF headers | 2.0.17, package `3.20017.241212+sha.dcc1105b` / 4.4.7 |
| Xtensa ESP32-S3 GCC / RISC-V tools package | Both `8.4.0+2021r2-patch5`; Xtensa compiled the application |
| esptool / SCons package | 4.5.1 (`1.40501.0`) / `4.41101.0` |
| TFT_eSPI / Adafruit TCA8418 / Adafruit BusIO | 2.5.43 / 1.0.2 / 1.17.2 |
| Arduino FS, LittleFS, Preferences, SD, SPI, SPIFFS and Wire | 2.0.0 each |
| Host tests | Apple Clang 21.0.0; C++11, regular and sanitized |

No target library or platform version changed from 0.8.0. The SDK library list describes the resolved build graph. Hardware availability is controlled by application profiles. [Memory and flash comparison](RELEASE_CANDIDATE.md) · [macOS installation](INSTALLATION.md) · [Component notices](../blackbox/release/rc1/NOTICES.md)
