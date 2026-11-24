# BLACKBOX OS roadmap

BLACKBOX OS targets the M5Stack Cardputer ADV. Development uses the independent PlatformIO environment `blackbox-cardputer-adv`.

| Release | Implemented |
|---|---|
| 0.1–0.2 | Board startup, keyboard and display, system information, settings and application dashboard |
| 0.3 | Peripheral inventory and exclusive pin and bus ownership |
| 0.4 | Internal Wi-Fi and Bluetooth diagnostic applications |
| 0.5 | Sub-GHz Signal Center and host simulation |
| 0.6 | NFC Center and Infrared Lab with disabled, bench-pending physical profiles |
| 0.7 | Receive-only Terminal, I²C map, SPI inventory and simulated transactions, sampled GPIO traces, offline calculators and references, and System Profiler |
| 0.8 | Workspace launcher, quick search, favorites and recent apps, NVS preferences, bounded Files manager, session recording, report preview/export and offline help |

## Next milestone — ADV bench validation

1. Verify the 0.8.0 dashboard, keyboard text entry and G0 navigation on the ADV. Verify the simulator and offline tools first. Review physical UART RX, Grove I²C and GPIO profiles individually against the bench assembly before any separately enabled hardware test. Record board revision, wiring and instrument readings alongside the firmware hash. [Hardware Lab procedures](HARDWARE_LAB.md)
2. Verify that closing each peripheral restores availability to other applications, including after timeout and bus errors. Measure runtime heap recovery, UART receive throughput and display responsiveness.
3. Complete the separate accessory checks for the existing gated profiles before enabling any physical profile. [Sub-GHz bench procedures](SUBGHZ.md) · [NFC and infrared bench procedures](NFC_IR.md)
4. For each implementation change, rebuild the independent target, run its host checks and record image growth, static RAM and test evidence.
5. Verify reboot persistence, report contents and runtime heap on the ADV. Qualify the microSD slot and shared expansion bus on the actual assembly before changing the physical storage lock. Exercise full media, removal during writes and recovery with backed-up media. [Workspace bench procedure](WORKSPACE.md)
