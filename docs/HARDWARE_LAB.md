# Hardware Lab · BLACKBOX OS 0.7.0

Open **Lab** on the second dashboard page. **Terminal** opens the same UART screen directly. Arrows or a G0 tap move focus. Enter or a G0 hold selects the focused action. Delete or the on-screen Back action exits a tool. Opening a screen leaves external pins idle. Live operations require a separate start action and the existing ADV keyboard fingerprint. The calculators, pin reference and profiler work independently of accessory presence.

![Production views rendered with synthetic host inputs](hardware-lab-preview.svg)

| Tool | Operation and limits |
|---|---|
| UART Terminal | EXT GPIO13 TX and GPIO15 RX, 8N1. Before opening the port, choose 9600, 19200, 38400, 57600 or 115200 baud. Compose a line of up to 35 printable characters. Enter submits it once with CRLF. Delete edits, and G0 cancels composition. The three-line receive window replaces control bytes with dots and clears on close. Partial sends report an error. RX counts consumed bytes. Sustained high-rate input can exceed the buffer. |
| I²C Inspector | One 100 kHz Grove scan from 7-bit address 0x08 through 0x77. One address-only write probe runs per foreground iteration with a 10 ms bus timeout. ACK addresses appear in hex, 24 per page. An ACK indicates a response and does not identify the device. An error stops the scan and preserves the partial results. |
| SPI Diagnostics | On a bare EXT connector with the data pins bridged, one action exchanges four bytes at 100 kHz, mode 0, MSB first. The screen shows the echoed bytes and whether all four matched. Known SD and cap chip selects stay high throughout. The bus is released immediately afterward. |
| GPIO Monitor | Samples Grove GPIO2 and GPIO1 as digital inputs with pulls disabled. Shows the latest levels and the number of changes between samples, at least 20 ms apart. Floating pins can vary and short pulses can be missed. |
| Electronics calculators | Ohm’s law in V/kΩ → mA, RC time constant in kΩ × µF → ms, and parallel resistance in kΩ. Select a formula and enter A and B as positive decimal values from 0.000001 to 1000000. Presets support button-only operation. Results describe ideal components. |
| Pinout reference | Four offline pages cover the ADV EXT header, onboard microSD sharing, Grove wiring, LCD, keypad, battery ADC and IR output. Connector pin numbers and GPIO numbers are labeled separately. |
| System Profiler | Samples CPU clock, uptime, free heap, minimum free heap since boot, largest allocatable heap block and flash chip capacity. Values come from the running device and update once per second. |

## Wiring and ownership

Use 3.3 V logic and a common ground. The red Grove conductor is a 5 V supply. UART uses the EXT header's GPIO13 TX and GPIO15 RX. The inspector uses yellow GPIO2 SDA and white GPIO1 SCL. Confirm orientation on the board before connecting a fixture. [M5Stack Cardputer ADV pin map](https://docs.m5stack.com/en/core/Cardputer-Adv)

Module Manager grants an exclusive lease before each operation. UART overlaps the U219 RF profile. I²C and GPIO monitoring share Grove pins with the external reader profiles. SPI claims GPIO40 SCK, 14 MOSI, 39 MISO, and CS lines 5, 6 and 12. A conflicting lease or unconfirmed board prevents initialization. Stop, Back, timeout, transaction completion or initialization failure triggers cleanup. Interactive sessions expire after 60 seconds. UART strings and observations stay in volatile memory. The target adds no storage writes or diagnostic logging.

## Bench checks still required

1. With external accessories disconnected, verify startup, text entry, Shift, punctuation, Delete and the G0 controls. Compare displayed profiler readings at idle and during repeated sessions.
2. Bridge UART TX13 to RX15. Send a short known string at each selectable rate. Check CRLF, receipt, cancellation, timeout and release. Use a known-rate serial source to characterize drops separately.
3. Connect a known 3.3 V-compatible Grove I²C device with appropriate pull-ups. Compare its expected 7-bit address against the displayed list. Repeat on an empty bus. Check error handling and subsequent reuse.
4. For SPI loopback, power off, remove accessories and microSD, and bridge GPIO14 MOSI to GPIO39 MISO. After powering on, run the explicit test. Expect 00 A5 5A FF. Repeat with the bridge removed and check the observed mismatch. Verify clock and idle select levels on a logic analyzer.
5. Drive each Grove monitor input from a known 0/3.3 V fixture with common ground. Verify stable levels, slow transitions, stop and timeout. Verify that an I²C session can subsequently claim the same pins.

## Reproduction

Build from repository root with `pio run -d blackbox -e blackbox-cardputer-adv`. Run `sh blackbox/tests/check_nfc_ir.sh` for the host checks. All 13 test and preview programs passed, including regression checks for prior applications. Hardware Lab coverage includes unknown-board and conflicting claims, failed initialization, address scan bounds, timeout with clock rollover, SPI comparison, console wrap and CRLF, keyboard composition, calculator units and cleanup through the application lifecycle. The eight preview panels use production drawing functions and synthetic input. This release has compile and host-test evidence only.

## Clean build measurements

Source baseline `f059b18`. The compiler emitted zero errors and zero warnings. The host Python environment emitted its existing urllib3/LibreSSL warning. Esptool verified the application image checksum and embedded hash.

| Metric | Recorded 0.6.0 | Local 0.7.0 | Change |
|---|---:|---:|---:|
| Padded application bytes | 1,264,304 | 1,280,640 | +16,336 |
| Free bytes in 5,111,808-byte partition | 3,847,504 | 3,831,168 | −16,336 |
| Static RAM bytes of 327,680 | 72,740 | 73,060 | +320 |

PlatformIO reported 1,280,269 linked flash bytes. The padded image occupies 25.05% of the factory partition. Dependencies match 0.6.0. PlatformIO Core 6.2.0, Espressif32 6.10.0, Arduino ESP32 2.0.17 (3.20017.241212+sha.dcc1105b), Xtensa and RISC-V GCC 8.4.0+2021r2-patch5, esptool 4.5.1, TFT_eSPI 2.5.43, Adafruit TCA8418 1.0.2, BusIO 1.17.2, Wire 2.0.0 and SPI 2.0.0.

| Binary | SHA-256 |
|---|---|
| bootloader.bin | `1776e4dd896a69d0a5c2e79957b0e2a88aa4129b1381d6478683515a1f6af343` |
| firmware.bin | `2a8d688cdf86a81fb64bc5764123831776b2b826238a1664b1450fe204667324` |
| partitions.bin | `40dc554ae4f93b09fa057db852ff05b3e03d42167fe65a490496def94b3648ec` |
