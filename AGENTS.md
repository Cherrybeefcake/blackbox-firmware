# BLACKBOX engineering standards

Baseline for review: Bruce 1.17, 2b6ba4d490bfcf94ce2574c6154537dbd5838c3d. BLACKBOX OS targets the M5Stack Cardputer ADV. Compile its independent blackbox-cardputer-adv environment under blackbox/. The upstream build and board definitions remain available for reference.

- Reuse the display, keyboard event consumer, theme and menu infrastructure. Give one screen ownership of input at a time.
- Prefer fixed-size storage and bounded text buffers for frequently refreshed values. Justify persistent allocations and avoid allocating each frame. Draw small text and icons directly and redraw changed fields.
- For each implementation task record padded application bytes, change from the preceding comparable build, remaining application partition bytes, and static RAM. Identify the source commit and build profile.
- Allowlist diagnostic fields. Exclude passwords, tokens, raw configuration and session identifiers from serial output, screenshots and exported reports.
- Validate pin ranges, board reservations, bus ownership and chip-select conflicts before opening a peripheral. A configured pin does not establish device presence.
- Preserve upstream attribution, LICENSE, THIRD_PARTY.md and component notices. Document deliberate changes in behavior. Keep hardware flashing separate from compilation.
