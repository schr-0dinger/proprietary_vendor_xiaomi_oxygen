#!/usr/bin/env python3
"""Extract sensor mode geometry from reg tables in a sensor blob."""

from __future__ import annotations

import argparse
import pathlib
import struct
from dataclasses import dataclass


ENTRY_SIZE = 8


@dataclass
class TableCandidate:
    offset: int
    entries: list[tuple[int, int]]


def read_entries(data: bytes, start: int, terminator: int) -> list[tuple[int, int]]:
    entries: list[tuple[int, int]] = []
    pos = start

    while pos + ENTRY_SIZE <= len(data):
        reg, val = struct.unpack_from("<HH", data, pos)
        if reg == 0 and val == 0:
            break
        entries.append((reg, val))
        pos += ENTRY_SIZE
        if reg == terminator:
            break

    return entries


def extract_modes(data: bytes, start_reg: int, terminator: int, min_len: int) -> list[TableCandidate]:
    candidates: list[TableCandidate] = []
    seen_payloads: set[tuple[tuple[int, int], ...]] = set()

    for offset in range(0, len(data) - 4, 2):
        reg = struct.unpack_from("<H", data, offset)[0]
        if reg != start_reg:
            continue

        entries = read_entries(data, offset, terminator)
        if len(entries) < min_len:
            continue

        payload = tuple(entries)
        if payload in seen_payloads:
            continue
        seen_payloads.add(payload)
        candidates.append(TableCandidate(offset=offset, entries=entries))

    return candidates


def pair_value(entries: dict[int, int], high_reg: int, low_reg: int) -> int | None:
    if high_reg not in entries or low_reg not in entries:
        return None
    return (entries[high_reg] << 8) | entries[low_reg]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path)
    parser.add_argument("--start-reg", default="0x3808")
    parser.add_argument("--terminator", default="0x0000")
    parser.add_argument("--min-len", type=int, default=20)
    parser.add_argument("--x-reg-high", default="0x3808")
    parser.add_argument("--x-reg-low", default="0x3809")
    parser.add_argument("--y-reg-high", default="0x380a")
    parser.add_argument("--y-reg-low", default="0x380b")
    parser.add_argument("--line-reg-high", default="0x380c")
    parser.add_argument("--line-reg-low", default="0x380d")
    parser.add_argument("--frame-reg-high", default="0x380e")
    parser.add_argument("--frame-reg-low", default="0x380f")
    return parser.parse_args()


def parse_u16(value: str) -> int:
    return int(value, 0) & 0xFFFF


def main() -> int:
    args = parse_args()
    data = args.blob.read_bytes()

    candidates = extract_modes(
        data=data,
        start_reg=parse_u16(args.start_reg),
        terminator=parse_u16(args.terminator),
        min_len=args.min_len,
    )

    regs = {
        "x_high": parse_u16(args.x_reg_high),
        "x_low": parse_u16(args.x_reg_low),
        "y_high": parse_u16(args.y_reg_high),
        "y_low": parse_u16(args.y_reg_low),
        "line_high": parse_u16(args.line_reg_high),
        "line_low": parse_u16(args.line_reg_low),
        "frame_high": parse_u16(args.frame_reg_high),
        "frame_low": parse_u16(args.frame_reg_low),
    }

    for idx, candidate in enumerate(candidates):
        entry_map = {reg: val for reg, val in candidate.entries}
        x = pair_value(entry_map, regs["x_high"], regs["x_low"])
        y = pair_value(entry_map, regs["y_high"], regs["y_low"])
        line = pair_value(entry_map, regs["line_high"], regs["line_low"])
        frame = pair_value(entry_map, regs["frame_high"], regs["frame_low"])

        print(f"mode {idx}: offset=0x{candidate.offset:x}")
        print(f"  x={x} y={y} line={line} frame={frame}")

    print(f"total candidates: {len(candidates)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
