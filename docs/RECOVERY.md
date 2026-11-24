# RC1 recovery and backup restoration

| Symptom | Operator action |
|---|---|
| A dialog, editor or subtool has no obvious exit | Hold G0 for at least two seconds, then release. Home invokes the active application's cleanup and returns to the dashboard. An explicitly started recorder remains in RAM across app changes. |
| A peripheral reports a conflict | Open Module Manager and identify the owner. Exit the owning application or unmount storage before retrying. A locked profile needs qualification. |
| Radio start is deferred for low memory | Stop any active session, check the current and largest free blocks in System Health and retry once. Routine initialization errors roll back acquired stages. A shutdown fault latches the radio off until restart. |
| Invalid preferences | The UI runs with defaults. To replace the malformed record, confirm **Workspace → Preferences → Reset preferences**. An NVS initialization failure keeps settings volatile. Back up the device before diagnosing the partition. |
| Media missing or a report write failed | The session remains in RAM. Reopen Files and mount the original source. If an interrupted stage exists, explicitly recover it to an unused `PARTnnnn.TMP`. Inspect it, then retry export. RAM files and unsaved sessions disappear on restart. |
| Startup or display cannot be recovered | Disconnect accessories, check USB power and data, re-enter the manufacturer's download mode and verify the installed image addresses against the manifest. Reinstall a verified candidate or restore the board's complete backup. |

## Restore the previous firmware on macOS

Use the **same board**, its private backup directory and the Python environment from the installation guide. Compare the board, flash capacity and security status to the original backup record. Use the three-image install only for RC1. Restoring a full backup replaces everything in onboard flash, including current settings. Copy any wanted removable-storage files first.

1. Set the ADV side switch to OFF. Hold G0 while attaching USB-C, then release after power-up. Identify the new serial port.
2. Set the following paths to the actual backup and port. Verify the original SHA-256 and 8 MiB size. Stop if either check fails or the backup's board association is uncertain.
3. Write the complete verified backup at offset zero and compare it with device flash. If the port disappears, re-enter download mode and select the port again before verifying.

```sh
source /tmp/blackbox-rc1-tools/bin/activate
backup_dir="$HOME/blackbox-adv-backup.REPLACE_WITH_SAVED_DIRECTORY"
adv_port=/dev/cu.usbmodemREPLACE_WITH_YOUR_PORT
(cd "$backup_dir" && shasum -a 256 -c SHA256SUMS)
test "$(stat -f %z "$backup_dir/original.bin")" -eq 8388608
python -m esptool --chip esp32s3 --port "$adv_port" --baud 460800 write_flash \
  0x0 "$backup_dir/original.bin"
python -m esptool --chip esp32s3 --port "$adv_port" verify_flash \
  0x0 "$backup_dir/original.bin"
```

Restart normally, confirm the former firmware boots and verify its data with its own interface. If USB transfer is unreliable, try a direct data cable and 115200 baud. A known-good vendor image can restore bootability when a private backup is unavailable. It cannot reconstruct the former device settings. Record readback and restoration evidence during bench qualification. [Installation and backup](INSTALLATION.md) · [M5Stack ADV recovery mode](https://docs.m5stack.com/en/core/Cardputer-Adv) · [Espressif flash operations](https://docs.espressif.com/projects/esptool/en/release-v4/esp32s3/esptool/basic-commands.html)
