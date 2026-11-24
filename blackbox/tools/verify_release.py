# SPDX-License-Identifier: AGPL-3.0-or-later
"""Offline verification of all three factory image ranges and the binary partition table."""
import argparse
import hashlib
import json
from pathlib import Path
import struct


def verify(directory):
    layout = json.loads((directory / "flash-layout.json").read_text())
    regions = [("bootloader.bin", 0, 0x8000), ("partitions.bin", 0x8000, 0x9000),
               ("firmware.bin", 0x10000, 0x4F0000)]
    if layout["dirty"] or layout["qualified"] or layout["target"] != "M5Stack Cardputer ADV":
        raise ValueError("Expected a clean, bench-pending Cardputer ADV release")
    if layout["flash_bytes"] != 0x800000 or len(layout["images"]) != len(regions):
        raise ValueError("Unexpected flash layout")
    for image, (name, offset, end) in zip(layout["images"], regions):
        if image["name"] != name or int(image["offset"], 0) != offset:
            raise ValueError(f"Unexpected image at {offset:#x}")
        data = (directory / name).read_bytes()
        if len(data) != image["bytes"] or not 0 < len(data) <= end-offset:
            raise ValueError(f"Invalid length for {name}")
        if hashlib.sha256(data).hexdigest() != image["sha256"]:
            raise ValueError(f"SHA-256 mismatch for {name}")
        if name != "partitions.bin" and (len(data)<24 or data[0]!=0xe9 or int.from_bytes(data[12:14],"little")!=9):
            raise ValueError(f"Expected an ESP32-S3 image in {name}")
    table = (directory / "partitions.bin").read_bytes()
    expected = [(1,2,0x9000,0x6000,b"nvs"), (0,0,0x10000,0x4e0000,b"factory"),
                (1,0x82,0x4f0000,0x300000,b"spiffs"), (1,3,0x7f0000,0x10000,b"coredump")]
    for i, partition in enumerate(expected):
        magic, typ, subtype, offset, size, label, flags = struct.unpack_from("<HBBII16sI",table,32*i)
        if magic != 0x50aa or flags or (typ, subtype, offset, size, label.rstrip(b"\0")) != partition:
            raise ValueError("Incompatible partition table")
    if table[128:144]!=b"\xeb\xeb"+b"\xff"*14 or table[144:160]!=hashlib.md5(table[:128]).digest() or any(x!=255 for x in table[160:]):
        raise ValueError("Unexpected partition table trailer")
    print(f"Verified {layout['version']} from {layout['commit']}. Three images fit the 8 MiB ADV layout.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory",type=Path)
    verify(parser.parse_args().directory)
