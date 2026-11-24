# BLACKBOX OS for M5Stack Cardputer ADV

**0.9.0 RC1 is a software-tested candidate for bench qualification.** The independent PlatformIO project in this directory targets the K132-Adv, with an ESP32-S3, 8 MiB flash, a 240×135 panel and the ADV keyboard. All 13 dashboard applications use the same input and lifecycle dispatcher.

| Available in the candidate | How to review it |
|---|---|
| System Info and System Health, Settings | Inspect version, build ID, board fingerprint, voltage, memory, ownership and storage state. Run the read-only software self-test. |
| Wi-Fi, Bluetooth, Channel Analyzer | Explicitly start bounded passive discovery using the internal radio. Channel counts describe observed APs. |
| Sub-GHz, NFC, Infrared | Select deterministic fixtures. Physical U219, U216 and IR profiles await qualification. |
| Module Manager, Terminal, Hardware Lab | Inspect leases, exercise simulated RX and bus diagnostics, or use calculators, references and live system profiling. The pre-existing RFID2 address-only check requires explicit selection. |
| Files and Workspace | Browse the bounded RAM volume, search, pin favorites and export selected numeric report summaries. Simulated files disappear on reboot. Physical microSD is locked. |

On the dashboard use **Q** to search and **F** for the configured shortcut. Tap G0 to advance and hold to select in menu-based apps. Holding G0 for at least two seconds and releasing returns to the dashboard, including from text entry and confirmation dialogs. Software tests and device qualification have separate records. [Candidate report](../docs/RELEASE_CANDIDATE.md) · [Qualification checklist](../docs/HARDWARE_QUALIFICATION.md)

## Build and test

From the repository root, use a C++11 host compiler and PlatformIO Core 6.2.0. Clone with Git history to embed the firmware source revision. An exported source tree builds with an unversioned identifier.

```sh
sh blackbox/tests/run.sh
sh blackbox/tests/run.sh --sanitize
pio run -d blackbox -e blackbox-cardputer-adv -t clean
pio run -d blackbox -e blackbox-cardputer-adv
python3 blackbox/tools/verify_release.py blackbox/.pio/build/blackbox-cardputer-adv
```

The verifier checks a clean build's three images, offsets, partition table and hashes offline. The packaged candidate is under `blackbox/release/rc1/`. Manual installation, backup and restoration are separate operations. [macOS installation](../docs/INSTALLATION.md) · [Recovery](../docs/RECOVERY.md) · [Build manifest](../docs/RELEASE_MANIFEST.md)

The independent source is distributed under AGPL-3.0-or-later. Keep the repository license, upstream attribution and component notices with redistributions. Build and installation instructions here apply only to the independent target. [License](../LICENSE) · [Upstream attribution](../THIRD_PARTY.md) · [Component notices](release/rc1/NOTICES.md)
