#!/usr/bin/env python3
"""Evaluate libmmcamera2_sensor_modules 0x2da44 logic on sensor blob fields.

The code path reads from sensor_open_lib() base offsets:
  +0x228: packed u16/u16 (lo/hi)
  +0x22c: u32 reference value
and writes a u16 result to the destination struct (+0x2c).
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
    for prefix in (b"oxygen_", b"s5k5e8_", b"ov12a_", b"imx"):
        off = data.find(prefix)
        if off >= 0:
            return off
    raise ValueError("could not infer sensor name base; pass --name")


def to_i32(value: int) -> int:
    value &= 0xFFFFFFFF
    return value if value < 0x80000000 else value - 0x100000000


def eval_2da44(word_228: int, word_22c: int) -> tuple[int, int, int, int, int]:
    lo = word_228 & 0xFFFF
    hi = (word_228 >> 16) & 0xFFFF

    # Mirrors:
    #   if (lo < hi) r2 = hi; else r2 = lo;
    #   if (hi < ref) r2 -= ref;
    #   r0 = (ref > 0) ? ref - 1 : ref;
    #   if ((int32_t)r2 >= 0) r0 = r2;
    candidate = hi if lo < hi else lo
    if hi < word_22c:
        candidate = (candidate - word_22c) & 0xFFFFFFFF

    fallback = (word_22c - 1) & 0xFFFFFFFF if word_22c > 0 else word_22c
    result = candidate if to_i32(candidate) >= 0 else fallback
    return lo, hi, word_22c, candidate, result & 0xFFFF


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path)
    parser.add_argument("--name", help="Exact inline sensor name (for base offset)")
    args = parser.parse_args()

    data = args.blob.read_bytes()
    base = find_base(data, args.name)
    word_228 = struct.unpack_from("<I", data, base + 0x228)[0]
    word_22c = struct.unpack_from("<I", data, base + 0x22C)[0]
    lo, hi, ref, candidate, result = eval_2da44(word_228, word_22c)

    print(f"blob: {args.blob}")
    print(f"base_offset: 0x{base:08x}")
    print(f"word_0x228: 0x{word_228:08x} (lo={lo}, hi={hi})")
    print(f"word_0x22c: 0x{word_22c:08x} ({ref})")
    print(f"candidate: 0x{candidate:08x} ({to_i32(candidate)})")
    print(f"result_u16(strh @ +0x2c): {result}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
