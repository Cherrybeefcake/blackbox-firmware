# Known issues at Bruce 1.17

| Finding | Consequence and engineering response |
|---|---|
| Pin validators modify structures passed by value | Corrections can be lost before save. Validate the stored assignments, reject reserved pins and test invalid input. [Validators](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/configPins.cpp#L488-L519) |
| Shared SPI matching can use MOSI alone | A customized SCK/MISO pair may select the wrong bus. Check the complete mapping and chip-select ownership. [HAL](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/bus_HAL.cpp#L606-L623) |
| Saving configuration serializes sensitive fields to serial, internal storage and optionally SD | Use an allowlist for diagnostic output. A future credential-redaction change should cover the existing save path. [Serialization](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/config.cpp#L37-L68) · [Save](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/config.cpp#L446-L465) |
| Startup formats LittleFS when its mount fails | A dashboard refresh must read existing mount state. Review recovery behavior separately before changing it. [Storage startup](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/main.cpp#L184-L195) |
| Menu visibility, shortcut dispatch and startup apps are separate | Hiding an entry does not disable the underlying capability. [Shortcuts](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/mykeyboard.cpp#L273-L318) |
| Release statistics use total chip capacity | Track image fit against the factory partition. There is no second application slot. [Partitions](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/custom_8Mb.csv) |

Local compilation, live heap, physical pin ownership and detection on the user's device remain unverified.
