#!/usr/bin/env python3
"""Dump known sensor_open_lib inline blocks from a Qualcomm sensor blob."""

from __future__ import annotations

import argparse
import pathlib
import struct
from typing import Iterable


def read_words(data: bytes, offset: int, count: int) -> list[int]:
    words: list[int] = []
    for idx in range(count):
        pos = offset + idx * 4
        if pos + 4 > len(data):
            break
        words.append(struct.unpack_from("<I", data, pos)[0])
    return words


def format_words(words: Iterable[int], start_index: int = 0) -> str:
    lines: list[str] = []
    for idx, word in enumerate(words, start=start_index):
        try:
            fval = struct.unpack("<f", struct.pack("<I", word))[0]
            float_text = f"{fval:.6g}"
        except Exception:
            float_text = "n/a"
        lines.append(f"{idx:02d}: 0x{word:08x}  {float_text}")
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path, help="Path to sensor shared object")
    parser.add_argument(
        "--name",
        help="Exact sensor name string to locate (e.g. oxygen_imx386_sunny)",
    )
    return parser.parse_args()


def find_name_offset(data: bytes, name: str | None) -> int:
    if name:
        needle = name.encode() + b"\x00"
        offset = data.find(needle)
        if offset == -1:
            raise ValueError(f"sensor name '{name}' not found in blob")
        return offset

    for prefix in (b"oxygen_", b"s5k5e8_", b"ov12a_", b"imx", b"s5k"):
        pos = data.find(prefix)
        if pos != -1:
            return pos

    raise ValueError("sensor name not provided and no known prefix found")


def main() -> int:
    args = parse_args()
    data = args.blob.read_bytes()
    base = find_name_offset(data, args.name)

    blocks = {
        "resolution_triplets_0x0f8": (0x0F8, 22),
        "reserved_0x150_words": (0x150, 28),
        "meta_0x1c0_words": (0x1C0, 10),
        "meta_0x1e8_words": (0x1E8, 16),
        "meta_0x228_words": (0x228, 34),
        "meta_0x2b0_words": (0x2B0, 6),
    }

    print(f"blob: {args.blob}")
    print(f"name_offset: 0x{base:08x}")
    print("")

    for label, (rel, count) in blocks.items():
        offset = base + rel
        words = read_words(data, offset, count)
        print(f"[{label}] offset=0x{offset:08x}")
        print(format_words(words))
        print("")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
