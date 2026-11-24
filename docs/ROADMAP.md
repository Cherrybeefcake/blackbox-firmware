# BLACKBOX OS roadmap

BLACKBOX OS targets the M5Stack Cardputer ADV. Development uses the independent PlatformIO environment `blackbox-cardputer-adv`.

| Release | Implemented |
|---|---|
| 0.1–0.2 | Board startup, keyboard and display, system information, settings and application dashboard |
| 0.3 | Peripheral inventory and exclusive pin and bus ownership |
| 0.4 | Internal Wi-Fi and Bluetooth diagnostic applications |
| 0.5 | Sub-GHz Signal Center and host simulation |
| 0.6 | NFC Center and Infrared Lab with disabled, bench-pending physical profiles |
| 0.7 | Hardware Lab with UART Terminal, I²C Inspector, SPI Diagnostics, GPIO Monitor, electronics calculators, ADV pinout reference and System Profiler |

## Next milestone — ADV bench validation

1. Verify the 0.7.0 dashboard, keyboard text entry and G0 navigation on the ADV. Run the loopback, address inspection and input-level checks with known fixtures. Record board revision, wiring and instrument readings alongside the firmware hash. [Hardware Lab procedures](HARDWARE_LAB.md)
2. Verify that closing each peripheral restores availability to other applications, including after timeout and bus errors. Measure runtime heap recovery, UART receive throughput and display responsiveness.
3. Complete the separate accessory checks for the existing gated profiles before enabling any physical profile. [Sub-GHz bench procedures](SUBGHZ.md) · [NFC and infrared bench procedures](NFC_IR.md)
4. For each implementation change, rebuild the independent target, run its host checks and record image growth, static RAM and test evidence.
