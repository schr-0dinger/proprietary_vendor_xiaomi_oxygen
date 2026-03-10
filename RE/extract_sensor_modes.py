#!/usr/bin/env python3
"""Extract likely register tables from Qualcomm camera sensor blobs.

This is intentionally heuristic. It works well for Xiaomi/Qualcomm sensor libs
that embed `<u16 reg, u16 val>` pairs inside larger open-library structures.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
from dataclasses import dataclass


ENTRY_SIZE = 8


@dataclass(frozen=True)
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


def extract_modes(
    data: bytes,
    start_reg: int,
    terminator: int,
    min_len: int,
    dedupe_payloads: bool,
) -> list[TableCandidate]:
    candidates: list[TableCandidate] = []
    seen_offsets: set[int] = set()
    seen_payloads: set[tuple[tuple[int, int], ...]] = set()

    for offset in range(0, len(data) - 4, 2):
        reg = struct.unpack_from("<H", data, offset)[0]
        if reg != start_reg:
            continue

        entries = read_entries(data, offset, terminator)
        if len(entries) < min_len:
            continue

        payload = tuple(entries)
        if offset in seen_offsets:
            continue
        if dedupe_payloads and payload in seen_payloads:
            continue

        seen_offsets.add(offset)
        seen_payloads.add(payload)
        candidates.append(TableCandidate(offset=offset, entries=entries))

    return candidates


def emit_text(
    candidates: list[TableCandidate],
    array_prefix: str,
    show_offsets: bool,
) -> str:
    lines: list[str] = []

    for idx, candidate in enumerate(candidates):
        if show_offsets:
            lines.append(f"/* candidate {idx}: offset 0x{candidate.offset:x} */")
        lines.append(f"static const struct regval {array_prefix}_{idx}[] = {{")
        for reg, val in candidate.entries:
            lines.append(f"    {{0x{reg:04x}, 0x{val:04x}}},")
        lines.append("};")
        lines.append("")

    lines.append(f"/* total candidates: {len(candidates)} */")
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("blob", type=pathlib.Path, help="Path to a sensor shared object")
    parser.add_argument(
        "--start-reg",
        default="0x3030",
        help="Expected first register in a table, default: 0x3030",
    )
    parser.add_argument(
        "--terminator",
        default="0x3004",
        help="Register value treated as end-of-table, default: 0x3004",
    )
    parser.add_argument(
        "--min-len",
        type=int,
        default=10,
        help="Minimum number of entries before a candidate is kept",
    )
    parser.add_argument(
        "--array-prefix",
        default="sensor_mode",
        help="Prefix used for emitted C arrays",
    )
    parser.add_argument(
        "--no-offsets",
        action="store_true",
        help="Do not emit file offsets as comments",
    )
    parser.add_argument(
        "--keep-duplicates",
        action="store_true",
        help="Emit duplicate payloads found at different offsets",
    )
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
        dedupe_payloads=not args.keep_duplicates,
    )

    print(
        emit_text(
            candidates=candidates,
            array_prefix=args.array_prefix,
            show_offsets=not args.no_offsets,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
