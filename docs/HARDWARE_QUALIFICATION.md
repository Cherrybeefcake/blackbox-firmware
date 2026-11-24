# Cardputer ADV qualification sheet · 0.9.0 RC1

**Physical status: pending for every row.** The packaged factory application is from `8332ac614e068fb9dd33f70279ed32a50912a72a`, SHA-256 `76f9a0c6201c6b435e603dbc55cd051e65cd6729187bf5ad21303d69010a2a33`. Complete a copy of this sheet per assembly. Keep photographs and traces free of network names, radio addresses, tag identifiers, raw UART contents and private backups.

| Bench record | Enter before testing |
|---|---|
| Run ID, operator, date, ambient conditions | ____ |
| Product marking, PCB revision, flash/security status | K132-Adv marking ____ · revision ____ · flash ____ |
| Accessory SKU, PCB revision, orientation, cable and supply | ____ |
| Firmware version, build ID, application SHA-256 and configuration | ____ |
| Meter, scope, optical detector or reference fixture, calibration and uncertainty | ____ |
| Expected voltage/timing ranges, tolerance and authority for each comparison | ____ |
| Evidence location and reviewer | ____ |

Use **pending**, **pass**, **fail** or **blocked** for each row, with observed values and an evidence ID. A pass requires the stated observation on the recorded assembly. Define numeric tolerances using the schematic, component limits or a known reference before taking comparative measurements. Keep the setting, load and measurement conditions with each reading.

### Standard package

| ID | Procedure and acceptance evidence | Result / evidence |
|---|---|---|
| ADV-01 | Photograph the ADV and verify revision against its schematic. Verify a private full backup before installation. Compare image hashes, target flash capacity and installed range readback. Confirm the former image can be restored on the same test board. | Pending / ____ |
| ADV-02 | From a cold start, record the version and source ID, panel orientation, 240×135 coverage, emerald/black/color order and legibility. Open all 13 tiles and cycle all pages, long names, empty lists, simulation and locked screens. Check text margins, selection and visible refresh behavior. | Pending / ____ |
| ADV-03 | Verify letters, Shift, four directions, Enter, Delete, Q and F. Exercise tap, hold-select and ≥2 s hold/release Home on G0 from an editor, confirmation and running app. Verify one event per release and that the keyboard responds after returning Home. | Pending / ____ |
| ADV-04 | Record battery ADC mV alongside calibrated DMM voltage at a documented safe test point and multiple charge states. Compare differences and uncertainties. Observe brightness across all menu levels and reboot after saving. | Pending / ____ |
| ADV-05 | Change a benign preference, wait for saved status, cold boot and compare; confirm cancellation and confirmed reset. Isolate malformed/full NVS tests to a backed-up test unit. Check usable volatile defaults and preservation of unrelated records. | Pending / ____ |
| ADV-06 | Run all four System Health checks. Observe no peripheral activity, file writes or preference changes attributable to the self-test. Record the keyboard fingerprint, heap values, radio, storage, resource counts and any errors. | Pending / ____ |
| RAD-01 | Start passive Wi-Fi discovery against known lab APs, check channels and RSSI against reference observations, exercise sort, empty state, selected AP meter, timeout, cancel and Home. Confirm Analyzer bars represent observed AP counts. | Pending / ____ |
| RAD-02 | Start passive BLE discovery with a named lab beacon and duplicate advertisements. Check names, de-duplication, RSSI, explicit cancellation and empty state. Verify no connection attempts on the fixture. | Pending / ____ |
| RAD-03 | Alternate Wi-Fi and BLE for 25 start/stop cycles, including timeout and cancel. Record current, minimum and largest heap before startup, while active and after each stop. Check for monotonic loss after warm-up, residual scans, unusable UI or contention. Record SDK shutdown errors separately. | Pending / ____ |
| BUS-01 | With no accessories active, inspect ownership of panel, keyboard, ADC and G0. Enter each locked profile, select physical mode and verify the unavailable state and idle signals. Run simulations while observing inactive physical buses. | Pending / ____ |
| BUS-02 | For a verified U031-B RFID2 on the correct Grove cable, inspect signal levels and pull-ups before use. Explicitly request the 0x28 address check, verify only that address on the analyzer, disconnect, check missing/error, then verify lease release. An ACK identifies a responding address only. | Pending / ____ |
| WS-01 | On RAM source, create, navigate, view, export and recover an interrupted stage. Inspect the numeric report schema and its source labels. Confirm the expected loss of RAM files after reboot. | Pending / ____ |

### Accessory qualification

First inspect hardware with power disconnected, then use current-limited power and suitable instruments. Record the exact assembly and schematic revision. An electrical inspection may be completed independently of driver activation. Running a locked driver on hardware requires **recorded electrical evidence, explicit authorization and a separately identified bench build**. Attach its source revision, enabled profile and SHA-256 to the row. Only the standard locked configuration is distributed here. The confirmed-U219 adapter test in the host suite uses a GPIO fixture.

| ID | Physical checks and required observation | Result / evidence |
|---|---|---|
| CAP-01 | U219 Cap CC1101 with its supported 433 MHz antenna. Verify 5 V supply, regulated logic levels, connector continuity, shared enable polarity and idle bias. Scope SD, NFC and RF chip selects. In an approved bench build, verify CC1101 identity, bounded RX measurements against a known signal, timeout and idle release. | Pending / ____ |
| CAP-02 | U219 ST25R3916 NFC section. Confirm common power, IRQ, selected chip, mode and deassertion of RF and SD selects. With a public fixture tag, inspect identity, public capacity and NDEF, then check field shutdown, absence of writes and release on missing tag, timeout and cancellation. | Pending / ____ |
| NFC-01 | U216 Unit NFC on straight HY2.0-4P cable. Verify 5 V supply, safe SDA/SCL pull-ups, address 0x50 and reader identity. Check public, empty and denied-read test tags; measure field-off after Stop and after a disconnect. Record any unconfirmed standby. | Pending / ____ |
| IR-01 | Confirm ADV GPIO44 idle polarity before enabling an approved bench emitter. Observe a single 38 kHz, 33% target duty, 1 ms burst with a photodiode and scope; record frequency, envelope, idle return and repeat-key behavior. Record the external detector. Receiver measurements in RC1 are simulated. | Pending / ____ |
| IO-01 | Verify 3.3 V UART bench fixture and common ground. Drive RX only, leave TX disconnected. Compare known test bytes at each supported rate, ring wrap, pause, skipped bytes, timeouts, cancellation and idle pins. Verify report exports omit byte contents. | Pending / ____ |
| IO-02 | Verify Grove I²C wiring, pull-ups and logic levels. Compare address-only ACK map with a known fixture at supported speeds. Inject missing/stalled fixture, verify bounded timeout and re-acquisition. Check internal keyboard independently. | Pending / ____ |
| IO-03 | Verify approved Grove GPIO inputs with known low, high and slow pulses. Compare sampled history against scope timing. Verify input-only behavior, conflict refusal, and released pins on exit. | Pending / ____ |
| SD-01 | Record actual microSD slot, available chip selects, SPI bus ownership and sacrificial card geometry. Check absent, full and valid card, filesystem bounds, failed mount with data retained, removal during browse and staged export, recovery and subsequent remount. Compare exported bytes and original files. | Pending / ____ |
| BUS-03 | Exercise SD/U219 RF/U219 NFC and shared Grove ownership in the relevant authorized configurations. Observe exactly one active select and inspect leases after failed initialization, timeout, cancel and Home. Repeat handoff 20 times; check display, keyboard and dashboard after each run. | Pending / ____ |

**Acceptance decision** ____ · open failures ____ · reviewer/date ____. A stable release requires a reviewed disposition for every applicable row. A capability with a pending hardware disposition stays locked wherever a lock applies.

[Cardputer ADV](https://docs.m5stack.com/en/core/Cardputer-Adv) · [U219](https://docs.m5stack.com/en/cap/Cap_CC1101) · [U216](https://docs.m5stack.com/en/unit/Unit_NFC) · [U031-B](https://docs.m5stack.com/en/unit/Unit_RFID2) · [Per-profile electrical references](NFC_IR.md) · [Hardware Lab fixtures](HARDWARE_LAB.md) · [Backup and recovery](RECOVERY.md)
