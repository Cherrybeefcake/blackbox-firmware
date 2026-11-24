# BLACKBOX OS 0.9.0 RC1 · engineering report

**Ready for supervised qualification on the M5Stack Cardputer ADV.** The independent firmware clean-built and all 20 host executables passed in normal and sanitizer configurations. Physical installation, recovery, power, real radio behavior and accessory qualification remain pending. The standard image retains the existing hardware locks.

The feature branch starts at the 0.8.0 workspace commit `227debedea2ae7c42982957b77497f7561341225`. The packaged application identifies its firmware inputs with `8332ac614e068fb9dd33f70279ed32a50912a72a`. Three images, a machine-readable layout, checksums and component notices are included. [Release manifest](RELEASE_MANIFEST.md) · [Package](../blackbox/release/rc1/) · [Installation](INSTALLATION.md) · [Recovery](RECOVERY.md)

## Build and memory measurements

Both columns are local clean builds with the same target and toolchain. The baseline was built from an isolated export of the named 0.8.0 commit. Remaining capacity uses the actual application file length. PlatformIO's flash figure describes linked sections; the installed file also contains image headers, alignment and validation data.

| Metric, bytes | 0.8.0 local baseline | 0.9.0 RC1 | Change |
|---|---:|---:|---:|
| Application image on disk | 1,349,328 | 1,339,584 | −9,744 |
| Linked program flash | 1,348,969 | 1,339,225 | −9,744 |
| Factory partition capacity | 5,111,808 | 5,111,808 | 0 |
| Remaining after application image | 3,762,480 | 3,772,224 | +9,744 |
| Static RAM, of 327,680 | 92,804 | 84,940 | −7,864 |

The candidate image fills **26.21%** of its partition. Linked static RAM is **25.92%** of PlatformIO's RAM denominator. The clean compile and link produced zero compiler warnings and zero errors. The host Python environment printed the same urllib3/LibreSSL compatibility warning twice during each build. SDK and third-party dependency versions matched the baseline. The freshly rebuilt baseline image SHA-256 was `3eca620140070b6fadf171f2afadab5d9d59ece801a83ab9c68fcfc3ff16f8d8`; the earlier 0.8.0 publication has its own digest. [Historical 0.8.0 record](WORKSPACE.md) · [RC1 hashes and versions](RELEASE_MANIFEST.md)

**Linker map and runtime costs.** The candidate map and size-sorted ELF symbols show 8,876 bytes for the RAM volume including its 8,192-byte payload, 6,400 bytes for the stable report export buffer, 2,560 bytes for Wi-Fi SDK record conversion, 1,960 bytes for the bounded wireless table and 1,264 bytes for recorder aggregates. The volume and report must coexist while a staged transfer is pending. Wi-Fi's SDK record layout differs from the application's compact records. BLE now writes under a callback lock into the session result table, removing a second 1,960-byte table. Disabling persistent crash dumps also removed linked SDK code and static data. The measured deltas above include all RC1 changes together.

The ESP-IDF Wi-Fi and BLE stacks allocate internal memory on explicit start. The new admission check requires 16 KiB free internal memory and an 8 KiB largest block; allocation can still fail above those floors. System Health exposes real boot minimum/current/largest heap for bench recording. Files and session reports retain fixed bounds. The display caches unchanged rows and skips unchanged busy/meter redraws. Device latency, frame rate, battery life, RF sensitivity and peak heap during a scan have no physical measurements in this release.

## Severity-ranked findings and corrections

| Severity | Finding and user impact | Change and evidence |
|---|---|---|
| High | The framework's generic installer placed an 8 KiB OTA initializer at `0xe000`. Its first sector overlaps the final NVS sector in the factory-only layout. | Removed that image from both effective uploader arguments and image records. Build and offline range checks enforce exactly three named images. |
| High | Arduino startup could erase NVS after either of two initialization errors, potentially losing settings outside BLACKBOX. | Guard both return codes before Arduino's fallback, retain volatile operation and present NVS availability. Host return-code test and final ELF guard check passed. |
| High | The bundled SDK's flash core-dump path could persist crashed task stacks containing transient diagnostics. | Disabled crash-dump initialization and the writer in the final link. Build validates required wrappers and absence of both flash and UART core-dump writers. The legacy partition remains intact. |
| Medium | A damaged dashboard selection could skip the active application's exit and leak ownership. Some subtools lacked a direct G0-only exit. | Retain the actual active lifecycle and route the two-second G0 release through its cleanup. Host test corrupts selection during a fixture scan and verifies release. |
| Medium | Cancelling during media removal could leave a stale owner; the Files status page could continue to describe a lost mount. | Cancellation detects offline storage, releases ownership and surfaces the error. Host tests cover removal during write and cancellation, preserved original and explicit stage recovery. |
| Medium | A malformed preference blob could be silently replaced by a later routine write. | Hold defaults in RAM and preserve the stored blob until the user confirms reset. Fixture checks cover malformed records, delayed writes and retry. |
| Medium | Radio teardown discarded failed stage state; low heap could enter expensive initialization. | Track successful stages through teardown, latch a shutdown failure, reject concurrent/low-memory starts and drain result callbacks before reading. Host checks exercise the state machine and thresholds. Actual SDK teardown needs bench testing. |
| Medium | An uncleared keyboard FIFO at startup could block input initialization; failed keyboard probing left the bus acquired. | Bound startup drain to the TCA8418's ten events and release a failed bus. Fixture covers absent keyboard, a stuck queue and G0 fallback. |
| Medium | Unlimited session duration could wrap elapsed time and accumulate observations indefinitely. | Cap recording at 24 hours with unsigned elapsed-time arithmetic. Check 86,400 samples across a simulated 32-bit millisecond rollover. |
| Low | Duplicate BLE storage, redundant wireless paints, unrelated keys moving selection, and header/branding crowding increased cost or obscured controls. | Reuse one result table, filter navigation keys, skip unchanged wireless frames, bound the header title and improve dashboard selection, help and startup controls. Six production-rendered fixture sets compare byte-for-byte. |

## Application and privacy review

| Applications | Reviewed paths and evidence |
|---|---|
| System Info, Settings | Build identity, keyboard-based board fingerprint, ADC validity, heap, preference persistence, readable labels, brightness and return behavior. System Health adds runtime, ownership and self-test pages under the existing System tile. |
| Wi-Fi Explorer, Bluetooth Explorer, Channel Analyzer | Explicit bounded passive sessions, queue/callback handoff, shared radio owner, duplicate/boundary handling, RSSI meter, stop and shutdown failure. Advertised identifiers serve local discovery. Exports accept aggregate channel counts. |
| Sub-GHz analysis | Bounded frequency plan, fixed history/peak hold, explicit measurement sessions, unavailable physical profile and release on cancellation or timeout. Standard configuration rejects the confirmed-U219 bench macro. |
| NFC Center, Infrared Lab | Bounded public record parsing, synthetic sources, source labels, explicit reader/emitter sessions, failure cleanup and physical profile locks. NFC identifiers and records stay outside report schemas. |
| Module Manager | Permanent system resources, unavailable state, shared pins/selects, exclusive bus ownership and explicit U031-B address-only check. ACK is an address observation. |
| Terminal, Hardware Lab | RX-only fixture, bounded input history, I²C/GPIO scopes, simulated SPI, calculators, pin references and profiler. Saved preferences select UI and tool defaults. Physical callbacks are selected by the compiled profiles. Raw serial bytes have no report field. |
| Files, Workspace | Visibility/execution boundary, path and size checks, staged export, interrupted stage recovery, recent IDs, settings packet schema, session sources, confirmation and offline help. File actions browse/view/copy/rename/delete within the validated workspace. |

All 13 real lifecycle tables were exercised across 1,300 host entry/exit cycles. The user-initiated software self-test checks registry integrity, a preference round trip in local memory, path containment and active resource consistency. It reads cached ownership without acquiring a bus. The four PASS values in the preview are fixture results. [Test matrix](TEST_MATRIX.md) · [RC1 UI preview](rc-preview.svg)

**Data boundary.** The independent target's explicit source list, startup paths and report writers contain no credential entry/collection, network association, file execution or UART-content export feature. Reports accept enumerated numeric metrics and source labels, not identifiers, tag contents or arbitrary text. Files permits bounded inspection and copying of user-supplied files under `/BLACKBOX`; those files can themselves contain personal information. Wi-Fi and BLE discovery names and addresses exist in volatile session data; hardware qualification and forensic RAM erasure are separate questions. A full operator backup or an old device partition can contain private data. Retain backups privately. The standard release has no route from preference data to enable a locked driver. The existing U031-B address check and passive radio sessions require a user action.

**Attribution.** The upstream Bruce license and attributions remain in the repository. The independent project rebuilds from the listed source revision and selected third-party components. The package includes component notices and framework source references. [Notices and corresponding source](../blackbox/release/rc1/NOTICES.md)

## Installation readiness and open work

1. At the bench, identify the ADV revision, secure a verified private full-flash backup, compare the three hashes, install the candidate manually and verify readback. Confirm the version and source ID at first boot. [Installation](INSTALLATION.md)
2. Record panel/input behavior, actual heap, repeated Wi-Fi/BLE start-stop and recovery. Check settings persistence, file recovery on a test fixture and resource handoff. Exercise a complete restore on the same backed-up test board. [Physical acceptance sheet](HARDWARE_QUALIFICATION.md)
3. Keep the external profiles locked while electrical qualification is pending. A separately authorized bench build and evidence for each actual accessory precede enablement. RAM files disappear on reboot; an unrecoverable SDK radio shutdown requires restart. Existing framework crash-partition contents and private backups require their owner's handling. [Recovery](RECOVERY.md)

No device was flashed, erased or exercised to produce these results. The installation package and checksums were validated offline. Stable release acceptance remains open.

## Changed files

The inventory below is relative to the 0.8.0 source commit.

```text
.github/workflows/blackbox.yml
blackbox/README.md
blackbox/platformio.ini
blackbox/release/rc1/NOTICES.md
blackbox/release/rc1/SHA256SUMS
blackbox/release/rc1/bootloader.bin
blackbox/release/rc1/firmware.bin
blackbox/release/rc1/flash-layout.json
blackbox/release/rc1/notices/Adafruit-BusIO.txt
blackbox/release/rc1/notices/Adafruit-TCA8418.txt
blackbox/release/rc1/notices/Arduino-ESP32-LGPL.md
blackbox/release/rc1/notices/BLACKBOX-AGPL.txt
blackbox/release/rc1/notices/ESP-IDF-Apache.txt
blackbox/release/rc1/notices/TFT_eSPI.txt
blackbox/release/rc1/partitions.bin
blackbox/src/crash_privacy.cpp
blackbox/src/device.h
blackbox/src/display.cpp
blackbox/src/files_ui.cpp
blackbox/src/health.cpp
blackbox/src/input.cpp
blackbox/src/main.cpp
blackbox/src/navigation.cpp
blackbox/src/nvs_guard.cpp
blackbox/src/platform.h
blackbox/src/radio.cpp
blackbox/src/reports.cpp
blackbox/src/reports.h
blackbox/src/search.cpp
blackbox/src/settings_nvs.cpp
blackbox/src/storage.cpp
blackbox/src/subghz.h
blackbox/src/wireless.h
blackbox/src/wireless_ui.cpp
blackbox/src/workspace.h
blackbox/src/workspace_ui.cpp
blackbox/tests/check_nfc_ir.sh
blackbox/tests/keyboard_fixture/Adafruit_TCA8418.h
blackbox/tests/keyboard_fixture/Arduino.h
blackbox/tests/keyboard_fixture/Wire.h
blackbox/tests/keyboard_test.cpp
blackbox/tests/nvs_guard_test.cpp
blackbox/tests/preferences_test.cpp
blackbox/tests/rc_platform/Arduino.h
blackbox/tests/rc_platform/TFT_eSPI.h
blackbox/tests/rc_platform/nvs_flash.h
blackbox/tests/rc_preview.cpp
blackbox/tests/run.sh
blackbox/tests/wireless_test.cpp
blackbox/tests/workspace_preview.cpp
blackbox/tests/workspace_test.cpp
blackbox/tools/build_id.py
blackbox/tools/factory_images.py
blackbox/tools/verify_release.py
docs/ARCHITECTURE.md
docs/BUILD.md
docs/DASHBOARD.md
docs/HARDWARE_QUALIFICATION.md
docs/INSTALLATION.md
docs/KNOWN_ISSUES.md
docs/RECOVERY.md
docs/RELEASE_CANDIDATE.md
docs/RELEASE_MANIFEST.md
docs/ROADMAP.md
docs/TEST_MATRIX.md
docs/WORKSPACE.md
docs/rc-preview.svg
docs/workspace-preview.svg
```
