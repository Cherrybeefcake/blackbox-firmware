# Manual installation on macOS

RC1 targets the **M5Stack Cardputer-Adv K132-Adv with 8 MiB ESP32-S3 flash**. Bench acceptance is pending. The commands in this guide are for an operator at the device. Compilation, the host tests and the offline verifier require no serial connection.

## 1. Obtain and check the package

Use a USB-C **data** cable, a stable USB port, Python 3.11 and the v4 esptool command syntax below. From a checkout of the candidate branch, run the following in macOS Terminal.

```sh
python3.11 -m venv /tmp/blackbox-rc1-tools
source /tmp/blackbox-rc1-tools/bin/activate
python -m pip install 'esptool==4.5.1'
release_dir="$PWD/blackbox/release/rc1"
python blackbox/tools/verify_release.py "$release_dir"
(cd "$release_dir" && shasum -a 256 -c SHA256SUMS)
python -m esptool --chip esp32s3 image_info "$release_dir/firmware.bin"
```

Match the source revision and SHA-256 values to the reviewed branch manifest. Verify the package before connecting the device. The checksum file detects damaged files. Trust in the publisher comes from the reviewed repository revision. [Release manifest](RELEASE_MANIFEST.md)

## 2. Identify the board and port

Confirm the **ADV marking on the actual unit** and record its board revision. Disconnect external modules and remove the microSD card. Close serial monitors. List ports before and after connecting USB. Use the newly appearing port in the assignment below.

```sh
python -m serial.tools.list_ports -v
adv_port=/dev/cu.usbmodemREPLACE_WITH_YOUR_PORT
```

For download mode, turn the side power switch **OFF**, hold **G0**, attach the USB-C data cable and release G0 after power-up. Confirm that the ESP32-S3 responds, flash capacity is 8 MB and security status permits ordinary readback and installation. An ESP32-S3 response alone does not identify the board model. Secure-boot, encrypted-flash or read-restricted units require their owner's provisioning and recovery procedure. [M5Stack ADV download mode](https://docs.m5stack.com/en/core/Cardputer-Adv) · [Espressif S3 boot modes](https://docs.espressif.com/projects/esptool/en/release-v4/esp32s3/esptool/entering-bootloader.html)

```sh
python -m esptool --chip esp32s3 --port "$adv_port" flash_id
python -m esptool --chip esp32s3 --port "$adv_port" get_security_info
```

## 3. Back up all eight MiB before installation

A full-flash backup includes the original bootloader, layout, application and private device settings. Keep it in a private directory outside the repository. Verify the size and compare with the device before writing anything. Re-enter download mode as needed between commands.

```sh
umask 077
backup_dir=$(mktemp -d "$HOME/blackbox-adv-backup.XXXXXX")
python -m esptool --chip esp32s3 --port "$adv_port" read_flash 0x0 0x800000 "$backup_dir/original.bin"
test "$(stat -f %z "$backup_dir/original.bin")" -eq 8388608
python -m esptool --chip esp32s3 --port "$adv_port" verify_flash 0x0 "$backup_dir/original.bin"
(cd "$backup_dir" && shasum -a 256 original.bin > SHA256SUMS)
```

Record which physical board the verified backup belongs to. A differently partitioned previous firmware may use any of the regions below for data, even when their offsets look familiar. [Espressif flash backup and write commands](https://docs.espressif.com/projects/esptool/en/release-v4/esp32s3/esptool/basic-commands.html)

## 4. Install the three verified images

The candidate uses a factory partition at `0x10000` of length `0x4e0000`. NVS occupies `0x9000–0xefff`. The release recipe removes the generic Arduino OTA initializer at `0xe000`, which would overwrite part of NVS. This three-image installation writes the bootloader, partition table and factory application. Keep the flash header settings in the supplied images. The other data partitions can contain information left by earlier firmware.

```sh
python -m esptool --chip esp32s3 --port "$adv_port" --baud 460800 write_flash \
  0x0000 "$release_dir/bootloader.bin" \
  0x8000 "$release_dir/partitions.bin" \
  0x10000 "$release_dir/firmware.bin"
python -m esptool --chip esp32s3 --port "$adv_port" verify_flash \
  0x0000 "$release_dir/bootloader.bin" \
  0x8000 "$release_dir/partitions.bin" \
  0x10000 "$release_dir/firmware.bin"
```

A writing error or hash mismatch calls for the recovery procedure. [Espressif flash verification](https://docs.espressif.com/projects/esptool/en/release-v4/esp32s3/esptool/advanced-commands.html) · [Recovery and restore](RECOVERY.md)

## 5. First boot

Power up normally with G0 released. The expected sequence is a BLACKBOX RC1 startup screen followed by the dashboard. Open **System → System Health** and compare the displayed build ID with the manifest. Continue to **Self-test** and select Run. Record the result, heap readings and any error state on the qualification sheet. First boot does not mount a card, start a radio scan or activate external accessories. Existing compatible BLACKBOX preferences can carry over. A damaged preference record selects defaults and waits for an explicit reset. [Physical acceptance sheet](HARDWARE_QUALIFICATION.md)
