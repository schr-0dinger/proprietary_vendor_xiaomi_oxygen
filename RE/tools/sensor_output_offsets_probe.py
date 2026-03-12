#!/usr/bin/env python3
"""Probe sensor_lib output-info offsets used by libmmcamera2_sensor_modules.

This script inspects the fixed offsets observed in sensor_get_output_info():
  - output_info base: +0x73fc0, stride 0x40
  - companion u16 table: +0x75188, stride 0x08
"""

from __future__ import annotations

import argparse
import pathlib
import struct


def find_base(data: bytes, name: str | None) -> int:
    if name:
        needle = name.encode() + b"\x00"
        off = data.find(needle)
        if off < 0:
            raise ValueError(f"sensor name '{name}' not found in blob")
        return off
    # Fallback for convenience with oxygen camera blobs.
    for prefix in (b"oxygen_", b"s5k5e8_"):
        off = data.find(prefix)
        if off >= 0:
            return off
    raise ValueError("could not infer sensor name base; pass --name")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path)
    parser.add_argument("--name", help="Exact inline sensor name (for base offset)")
    parser.add_argument("--count", type=int, default=6, help="Number of output slots")
    args = parser.parse_args()

    data = args.blob.read_bytes()
    base = find_base(data, args.name)
    print(f"blob: {args.blob}")
    print(f"base_offset: 0x{base:08x}")
    print("")

    for idx in range(args.count):
        out_off = base + 0x73FC0 + idx * 0x40
        aux_off = base + 0x75188 + idx * 0x08

        x, y, line, frame = struct.unpack_from("<HHHH", data, out_off)
        vt, op = struct.unpack_from("<II", data, out_off + 0x08)
        binning = struct.unpack_from("<H", data, out_off + 0x10)[0]
        a0, a1, a2, a3 = struct.unpack_from("<HHHH", data, aux_off)

        # Matches sensor_get_output_info() arithmetic in libmmcamera2_sensor_modules.
        crop_x = x - 1 - a3
        crop_y = y - 1 - a1

        print(
            f"idx {idx}: {x}x{y} line={line} frame={frame} vt={vt} op={op} "
            f"bin={binning} aux=({a0},{a1},{a2},{a3}) crop_end=({crop_x},{crop_y})"
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
