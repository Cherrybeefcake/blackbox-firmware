# RC1 software and bench test matrix

At firmware source `8332ac614e068fb9dd33f70279ed32a50912a72a`, all **20 host executables passed twice** on macOS. The first pass used C++11 with warnings treated as errors. The second used AddressSanitizer and UndefinedBehaviorSanitizer with fail-on-error. Both passes compared all six generated SVGs with committed fixtures. The independent ESP32-S3 target also compiled, linked and passed the offline image and partition verifier.

| Executable | What the passing host check covers |
|---|---|
| `navigation_test` | Registry lookup, capability checks, tile boundaries, Back and returning Home |
| `modules_test` | Slot ownership, duplicate pins, conflicts, missing hardware, init rollback and cleanup |
| `wireless_test` | Bounded AP/BLE result model, sort/de-duplication, RSSI ring, scan ownership, low-heap admission boundaries, init failure, stop failure, latch and 32-bit timeout rollover |
| `signal_test` | Frequency validation, bounded synthetic sampling, peaks, history and session lifecycle |
| `u219_test` | Separately compiled bench-only adapter using a simulated GPIO/SPI fixture, including initialization failures and rollback |
| `lab_test` | Public NFC records and malformed input, bounded simulation, NFC and IR session states and release |
| `lab_profiles_test` | Compiled module descriptor lock and bus/ownership behavior with fixture interfaces |
| `lab_driver_test` | Reader and emitter adapters against host fixtures, with failure and completion paths |
| `bench_transport_test` | RX, I²C, SPI and GPIO session models, simulated transport, conflicts, cancellation and bounds |
| `bench_test` | Terminal and Hardware Lab tools, offline calculations, references, profiler and app controls |
| `workspace_test` | Workspace path constraints, staged copy, removal during write and cancel, short write, recovery, confirmations, search, preference validation, report schema, bounded output and numeric source checks |
| `preferences_test` | NVS adapter fixture, delayed persistence, writes/retries and preserving a malformed preference record until explicit reset |
| `keyboard_test` | Bounded TCA FIFO drain, failed probe bus release, G0 timing through clock wrap, shortcuts and Shift in an editor |
| `nvs_guard_test` | NVS init return codes, including both SDK fallback conditions that could trigger a bulk erase |
| `wireless_preview` | Production wireless UI with synthetic observations |
| `signal_preview` | Production Sub-GHz screens with a deterministic backend |
| `lab_preview` | Production NFC and IR screens using fixtures |
| `bench_preview` | Production Hardware Lab rendering and synthetic instruments |
| `workspace_preview` | Workspace and Files screens, including simulation and locked media |
| `rc_preview` | 100 cycles through all 13 production app lifecycles, Home cleanup, invalid selection while a fixture radio owns the session, mounted Files exit, and read-only System Health/self-test render |

The long-session check in `workspace_test` supplies **86,400 one-second observations** beginning near the 32-bit millisecond wrap. It checks the 24-hour stop, elapsed duration and exact aggregate count. The integrated UI check also verifies that refreshing an unchanged self-test page generates no further draw calls. These are simulated timings and paint-call counts.

### Repeat locally

```sh
sh blackbox/tests/run.sh
sh blackbox/tests/run.sh --sanitize
pio run -d blackbox -e blackbox-cardputer-adv -t clean
pio run -d blackbox -e blackbox-cardputer-adv
python3 blackbox/tools/verify_release.py blackbox/.pio/build/blackbox-cardputer-adv
python3 blackbox/tools/verify_release.py blackbox/release/rc1
```

The runner uses temporary build directories. Set `CXX` to select a sanitizer-capable compiler. Update preview fixtures intentionally with `sh blackbox/tests/run.sh --update-previews`. The production renderer uses a host display adapter with synthetic readings and font approximations.

### Evidence boundaries

- **Final ELF and images.** The build checks that the NVS guard and crash guards reached the linked firmware, checks that neither core-dump output implementation remains linked, and compares the extra image pairs to PlatformIO's effective uploader arguments. The offline verifier checks the ESP32-S3 headers, partition entries, partition-table checksum, region lengths and all three image SHA-256 values.
- **Package fault injection.** The offline verifier rejected a one-byte change in a temporary copy of the application and a changed bootloader offset. All four package checksums passed. A host compiler also rejected the confirmed-U219 flag when combined with the standard-release definition.
- **Hosted CI.** The isolated workflow uses a fresh Linux runner, repeats the normal and sanitized tests, builds only the independent target, verifies the outputs, and uploads its build artifacts. Build IDs follow the last commit touching firmware inputs. Host-specific toolchain paths and timestamps may change the binary digest. Use the manifest to identify the distributed macOS-built images. [Workflow](../.github/workflows/blackbox.yml)
- **Pending physical evidence.** The host harness substitutes transport, radio, storage, display and time at the device boundary. Real SDK callbacks, heap watermarks, electrical levels, bus handoff, panel legibility, power loss, USB installation and backup restoration require the actual assembly. [Qualification sheet](HARDWARE_QUALIFICATION.md) · [Release manifest](RELEASE_MANIFEST.md)
