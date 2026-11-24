# SPDX-License-Identifier: AGPL-3.0-or-later
"""Keep the factory-only upload away from NVS; record the effective, offline flash layout."""
import hashlib
import platform
import platformio
import subprocess
import json
from pathlib import Path

Import("env")
images = list(env.get("FLASH_EXTRA_IMAGES", []))
flags = list(env.get("UPLOADERFLAGS", []))
for offset, image in images:
    if int(str(offset), 0) != 0xe000 or Path(str(image)).name != "boot_app0.bin":
        continue
    images.remove((offset, image))
    for i in range(len(flags) - 1):
        if str(flags[i]) == str(offset) and Path(str(flags[i+1])).name == "boot_app0.bin":
            del flags[i:i+2]
            break
    break
env.Replace(FLASH_EXTRA_IMAGES=images, UPLOADERFLAGS=flags)

def manifest(source, target, env):
    build = Path(env.subst("$BUILD_DIR"))
    flags = [env.subst(str(arg)) for arg in env["UPLOADERFLAGS"]]
    if "write_flash" not in flags or "boot_app0.bin" in " ".join(flags):
        raise ValueError("Unexpected installer arguments for factory layout")
    records = []
    for offset, image in env["FLASH_EXTRA_IMAGES"]:
        path = Path(env.subst(str(image)))
        if not any(a == str(offset) and b == str(path) for a, b in zip(flags, flags[1:])):
            raise ValueError("Installer arguments disagree with extra images")
        records.append((int(str(offset), 0), path))
    records.append((int(env.subst("$ESP32_APP_OFFSET"), 0), Path(str(target[0]))))
    if [(offset, path.name) for offset, path in records] != [
        (0, "bootloader.bin"), (0x8000, "partitions.bin"), (0x10000, "firmware.bin")
    ]:
        raise ValueError("Unexpected ADV factory image layout")
    entries = []
    for (offset, path), limit in zip(records, [0x8000, 0x9000, 0x4f0000]):
        data = path.read_bytes()
        if not data or offset + len(data) > limit:
            raise ValueError("Image exceeds its flash region")
        entries.append(dict(name=path.name, offset=hex(offset), bytes=len(data),
                            sha256=hashlib.sha256(data).hexdigest()))
    sdk=env.PioPlatform()
    packages={name:str(sdk.get_package_version(name)) for name in (
        "framework-arduinoespressif32", "tool-esptoolpy", "toolchain-xtensa-esp32s3", "toolchain-riscv32-esp")}
    libraries={lib.name:str(lib.version) for lib in env.GetLibBuilders() if lib.is_dependent}
    # Check the final ELF, including the precompiled ESP-IDF panic handler.
    nm=env.WhereIs("xtensa-esp32s3-elf-nm")
    symbols={line.split()[-1] for line in subprocess.check_output(
        [nm,"--defined-only",str(build/"firmware.elf")],text=True).splitlines() if line.split()}
    if not {"__wrap_nvs_flash_init","__wrap_esp_core_dump_init","__wrap_esp_core_dump_to_flash"} <= symbols:
        raise ValueError("Release storage guards missing from linked firmware")
    if {"esp_core_dump_to_flash","esp_core_dump_to_uart"} & symbols:
        raise ValueError("Unexpected core dump writer in linked firmware")
    result = dict(target="M5Stack Cardputer ADV", version="0.9.0 RC1",
                  commit=env["BLACKBOX_REVISION"], dirty=env["BLACKBOX_DIRTY"],
                  qualified=False, python=platform.python_version(), platformio=platformio.__version__,
                  espressif32=str(sdk.version), packages=packages, libraries=libraries, flash_bytes=8388608, images=entries)
    (build / "flash-layout.json").write_text(json.dumps(result, indent=2) + "\n")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", manifest)
