# Known issues at Bruce 1.17

## Independent BLACKBOX RC1

Physical acceptance remains pending. Device-specific heap fragmentation, real SDK callback and stop timing, LCD presentation, keyboard electrical behavior, battery measurement, SD reliability and accessory handoff require bench evidence. A shutdown failure latches the radio off until reboot. Malformed preferences preserve the existing record until explicit reset; a failed NVS initialization makes new settings volatile. A backing store may hold private data from older firmware. The candidate preserves existing partitions and its disabled crash writer cannot recover or clear older dumps. See [remaining checks](HARDWARE_QUALIFICATION.md), [known limitations and fixes](RELEASE_CANDIDATE.md) and [operator recovery](RECOVERY.md).

### Historical upstream findings

| Finding | Consequence and engineering response |
|---|---|
| Pin validators modify structures passed by value | Corrections can be lost before save. Validate the stored assignments, reject reserved pins and test invalid input. [Validators](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/configPins.cpp#L488-L519) |
| Shared SPI matching can use MOSI alone | A customized SCK/MISO pair may select the wrong bus. Check the complete mapping and chip-select ownership. [HAL](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/bus_HAL.cpp#L606-L623) |
| Saving configuration serializes sensitive fields to serial, internal storage and optionally SD | Use an allowlist for diagnostic output. A future credential-redaction change should cover the existing save path. [Serialization](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/config.cpp#L37-L68) · [Save](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/config.cpp#L446-L465) |
| Startup formats LittleFS when its mount fails | A dashboard refresh must read existing mount state. Review recovery behavior separately before changing it. [Storage startup](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/main.cpp#L184-L195) |
| Menu visibility, shortcut dispatch and startup apps are separate | Hiding an entry does not disable the underlying capability. [Shortcuts](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/mykeyboard.cpp#L273-L318) |
| Release statistics use total chip capacity | Track image fit against the factory partition. There is no second application slot. [Partitions](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/custom_8Mb.csv) |

The initial upstream audit left live heap, physical pin ownership and peripheral detection open. Independent BLACKBOX releases have their own build and validation records.

## BLACKBOX OS 0.8.0

| Finding | Review consequence |
|---|---|
| The ADV microSD electrical profile awaits bench qualification | Physical mount and durable file export are locked. The deterministic RAM workspace loses files on reboot. |
| Card latency, hot removal and FAT power-loss behavior require real media | Host fault injection covers short writes and interrupted staging. An interrupted file requires explicit recovery in Files before the next write. |
| Workspace bounds are deliberate | Names use the supported 8.3 subset, paths stop at 79 bytes, and the physical listing shows at most 32 supported entries in the first 64 inspected. Use smaller folders. Text and CSV readers show printable ASCII. Viewer Back keeps eight page positions before returning to the start. |
| Recent preference edits have a write-behind interval | A reset or power cut before the delayed NVS commit can lose the latest adjustments. A failed write is retried and its status appears in Workspace preferences. |
| Static RAM increased by 18,824 bytes | Measure minimum heap during simultaneous recording and internal radio diagnostics on the ADV. |
| Physical inputs and sessions remain untested for this release | Host previews and fixture results cover software behavior. Board identity, power, keyboard, display and bus handoffs need direct checks. [Bench procedure and build evidence](WORKSPACE.md) |
