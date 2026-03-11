#!/usr/bin/env python3
"""Probe S5K5E8 output_info array by matching the known entry pattern."""

from __future__ import annotations

import argparse
import pathlib
import struct


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path)
    parser.add_argument("--offset", type=lambda x: int(x, 0), default=None)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    data = args.blob.read_bytes()

    pattern = struct.pack(
        "<HHHHIIH",
        2592, 1944, 3136, 1968,
        184000000, 165600000, 1,
    )

    hits = []
    if args.offset is None:
        for off in range(0, len(data) - len(pattern)):
            if data[off:off + len(pattern)] == pattern:
                hits.append(off)
    else:
        hits.append(args.offset)

    for off in hits:
        print(f"candidate offset=0x{off:x}")
        for i in range(6):
            base = off + i * 0x40
            x, y, line, frame, vt, op, binning = struct.unpack_from(
                "<HHHHIIH", data, base
            )
            print(
                f"  {i}: {x}x{y} line={line} frame={frame} "
                f"vt={vt} op={op} bin={binning}"
            )
    if not hits:
        print("no candidates found")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
