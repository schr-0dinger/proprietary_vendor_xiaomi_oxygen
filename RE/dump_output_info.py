#!/usr/bin/env python3
"""Heuristically locate and dump msm_sensor_output_info_t arrays."""

from __future__ import annotations

import argparse
import pathlib
import struct
from dataclasses import dataclass


@dataclass
class OutputInfo:
    x: int
    y: int
    line: int
    frame: int
    vt: int
    op: int
    binning: int


def parse_output_info(chunk: bytes) -> OutputInfo:
    x, y, line, frame = struct.unpack_from("<HHHH", chunk, 0)
    vt, op = struct.unpack_from("<II", chunk, 8)
    binning = struct.unpack_from("<H", chunk, 16)[0]
    return OutputInfo(x=x, y=y, line=line, frame=frame, vt=vt, op=op, binning=binning)


def is_plausible(info: OutputInfo) -> bool:
    if info.x == 0 or info.y == 0:
        return False
    if info.line < info.x or info.frame < info.y:
        return False
    if not (50_000_000 <= info.vt <= 600_000_000):
        return False
    if not (50_000_000 <= info.op <= 600_000_000):
        return False
    if info.binning not in (1, 2, 4, 8):
        return False
    return True


def find_name_offset(data: bytes, name: str | None) -> int:
    if name is None:
        return -1
    needle = name.encode() + b"\x00"
    offset = data.find(needle)
    if offset == -1:
        raise ValueError(f"sensor name '{name}' not found")
    return offset


def guess_count(data: bytes, base: int) -> int:
    if base < 0:
        return 6
    word = struct.unpack_from("<I", data, base + 0x0F8)[0]
    if 1 <= word <= 16:
        return int(word)
    return 6


def scan_candidates(data: bytes, count: int, stride: int) -> list[tuple[int, list[OutputInfo], int]]:
    results: list[tuple[int, list[OutputInfo], int]] = []
    block = count * stride

    for off in range(0, len(data) - block, 4):
        infos: list[OutputInfo] = []
        score = 0
        for idx in range(count):
            chunk = data[off + idx * stride : off + idx * stride + 20]
            info = parse_output_info(chunk)
            infos.append(info)
            if is_plausible(info):
                score += 1
        if score >= max(2, count // 2):
            results.append((off, infos, score))

    results.sort(key=lambda item: item[2], reverse=True)
    return results


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path)
    parser.add_argument("--name", help="Exact sensor name string")
    parser.add_argument("--stride", type=lambda x: int(x, 0), default=0x40)
    parser.add_argument("--count", type=int, default=0, help="Override mode count")
    parser.add_argument("--limit", type=int, default=3)
    args = parser.parse_args()

    data = args.blob.read_bytes()
    base = find_name_offset(data, args.name)
    count = args.count if args.count else guess_count(data, base)

    candidates = scan_candidates(data, count, args.stride)
    if not candidates:
        print("no candidates found")
        return 1

    for rank, (off, infos, score) in enumerate(candidates[: args.limit]):
        print(f"candidate {rank}: offset=0x{off:08x} stride=0x{args.stride:x} score={score}/{count}")
        for idx, info in enumerate(infos):
            print(
                f"  {idx}: {info.x}x{info.y} line={info.line} frame={info.frame} "
                f"vt={info.vt} op={info.op} bin={info.binning}"
            )
        print("")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
