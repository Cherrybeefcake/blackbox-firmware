# Firmware architecture and dashboard proposal

Boot initializes board defaults and display, mounts storage, reads saved configuration and then detects ADV hardware. The input task updates shared key state under a mutex. MainMenu filters registered menu objects and dispatches selections through synchronous callbacks. Carousel and grid views share the display and status bar. [Startup](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/main.cpp#L455-L575) · [Menu](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/main_menu.cpp#L6-L105) · [Input](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/mykeyboard.cpp#L241-L266)

**Proposed dashboard, pending review.** Register one System entry through MenuItemInterface. Its callback owns a short-lived view and returns through the existing Back action. Keep collection separate from drawing through a small fixed-size status snapshot. Use the current theme, one title and five text rows in the 240 × 135 landscape view.

| Row | Source and behavior |
|---|---|
| Hardware | Expose the cached result of the existing keyboard probe. Show “ADV detected” on success and “Cardputer · ADV unconfirmed” on fallback. |
| Battery | Reuse the board battery reading used by the status bar. Show unavailable when a valid measurement cannot be obtained. |
| Memory | Show current free heap and largest contiguous internal block using the underlying counters. Static linker RAM belongs in the build report. |
| Storage | Show existing SD mount state and internal filesystem usage when available. Use mount state without retrying initialization or formatting. |
| Firmware | Read the compiled version and revision labels. Preserve dev/Homebrew labels when present. |

Draw labels once, refresh changed values about once per second, and check Back between refreshes. Recalculate layout using current display dimensions. Keep graphics in the existing display pipeline. Opening the screen should perform no bus scans, serial dispatch, config saves or SD writes. A diagnostic-only firmware profile would also need to address shortcuts, startup apps and serial command entry points.

[Status bar](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/display.cpp#L1272-L1324) · [Memory counters](https://github.com/Cherrybeefcake/blackbox-firmware/blob/2b6ba4d490bfcf94ce2574c6154537dbd5838c3d/src/core/ram_profile.cpp#L8-L35)
