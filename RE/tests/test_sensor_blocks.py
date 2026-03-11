#!/usr/bin/env python3
"""Static sanity checks for Qualcomm sensor_open_lib inline blocks."""

from __future__ import annotations

import pathlib
import struct
import sys
from typing import Iterable


ROOT = pathlib.Path(__file__).resolve().parents[2]


def read_words(data: bytes, offset: int, count: int) -> list[int]:
    words: list[int] = []
    for idx in range(count):
        pos = offset + idx * 4
        if pos + 4 > len(data):
            break
        words.append(struct.unpack_from("<I", data, pos)[0])
    return words


def find_name_offset(data: bytes, name: str) -> int:
    needle = name.encode() + b"\x00"
    offset = data.find(needle)
    if offset == -1:
        raise AssertionError(f"sensor name '{name}' not found")
    return offset


def assert_words(label: str, words: Iterable[int], expected: Iterable[int]) -> None:
    got = list(words)
    want = list(expected)
    if got != want:
        raise AssertionError(f"{label} mismatch: got {got}, expected {want}")


def assert_word(label: str, word: int, expected: int) -> None:
    if word != expected:
        raise AssertionError(f"{label} mismatch: got 0x{word:08x}, expected 0x{expected:08x}")


def load_blob(path: pathlib.Path) -> bytes:
    if not path.exists():
        raise AssertionError(f"missing blob: {path}")
    return path.read_bytes()


def check_imx386() -> None:
    blob = ROOT / "proprietary/vendor/lib/libmmcamera_oxygen_imx386_sunny.so"
    data = load_blob(blob)
    base = find_name_offset(data, "oxygen_imx386_sunny")

    meta_1c0 = read_words(data, base + 0x1C0, 10)
    assert_words(
        "imx386 meta_0x1c0",
        meta_1c0,
        [
            0x00000001,
            0x00000000,
            0x00000001,
            0x00000001,
            0x00000003,
            0x034E034C,
            0x03400342,
            0x00000202,
            0x00000204,
            0x00000000,
        ],
    )

    meta_1e8 = read_words(data, base + 0x1E8, 16)
    assert_word("imx386 meta_0x1e8[1]", meta_1e8[1], 0x0000000A)
    assert_word("imx386 meta_0x1e8[2]", meta_1e8[2], 0x3F800000)
    assert_word("imx386 meta_0x1e8[3]", meta_1e8[3], 0x41800000)
    assert_word("imx386 meta_0x1e8[9]", meta_1e8[9], 0x0000FFF5)

    meta_228 = read_words(data, base + 0x228, 34)
    assert_word("imx386 meta_0x228[5]", meta_228[5], 0x00000FC0)
    assert_word("imx386 meta_0x228[6]", meta_228[6], 0x00000BC8)

    meta_2b0 = read_words(data, base + 0x2B0, 6)
    assert_words(
        "imx386 meta_0x2b0",
        meta_2b0,
        [0x00000003, 0x00000003, 0x00000000, 0x00000000, 0x00000001, 0x00021203],
    )

    output_base = 0x78FC8
    stride = 0x40
    expected = [
        (4032, 3016, 4296, 3070, 388000000, 398400000, 1),
        (2016, 1508, 2256, 1692, 114670000, 137600000, 1),
        (4032, 2256, 4296, 2310, 298000000, 308000000, 1),
        (3840, 2160, 4296, 2360, 297330000, 356800000, 1),
        (1920, 1080, 2256, 1692, 114670000, 137600000, 1),
        (1920, 1080, 2256, 1174, 318000000, 381600000, 1),
    ]
    for idx, want in enumerate(expected):
        off = output_base + idx * stride
        chunk = data[off : off + 20]
        x, y, line, frame = struct.unpack_from("<HHHH", chunk, 0)
        vt, op = struct.unpack_from("<II", chunk, 8)
        binning = struct.unpack_from("<H", chunk, 16)[0]
        got = (x, y, line, frame, vt, op, binning)
        if got != want:
            raise AssertionError(f"imx386 output_info[{idx}] mismatch: got {got}, expected {want}")

    triplets = read_words(data, base + 0x0F8, 1)
    assert_word("imx386 triplets[0]", triplets[0], 0x00000006)


def check_ov12a() -> None:
    blob = ROOT / "proprietary/vendor/lib/libmmcamera_oxygen_ov12a_sunny.so"
    data = load_blob(blob)
    base = find_name_offset(data, "oxygen_ov12a_sunny")

    meta_1c0 = read_words(data, base + 0x1C0, 10)
    assert_word("ov12a meta_0x1c0[5]", meta_1c0[5], 0x380A3808)
    assert_word("ov12a meta_0x1c0[6]", meta_1c0[6], 0x380E380C)
    assert_word("ov12a meta_0x1c0[7]", meta_1c0[7], 0x00003500)
    assert_word("ov12a meta_0x1c0[8]", meta_1c0[8], 0x00003508)

    meta_1e8 = read_words(data, base + 0x1E8, 16)
    assert_word("ov12a meta_0x1e8[1]", meta_1e8[1], 0x00000008)
    assert_word("ov12a meta_0x1e8[3]", meta_1e8[3], 0x41780000)

    meta_228 = read_words(data, base + 0x228, 34)
    assert_word("ov12a meta_0x228[5]", meta_228[5], 0x00001240)
    assert_word("ov12a meta_0x228[6]", meta_228[6], 0x00000DB0)

    meta_2b0 = read_words(data, base + 0x2B0, 6)
    assert_words("ov12a meta_0x2b0", meta_2b0, [0, 0, 0, 0, 0, 0])

    triplets = read_words(data, base + 0x0F8, 1)
    assert_word("ov12a triplets[0]", triplets[0], 0x00000006)


def check_s5k5e8() -> None:
    blob = ROOT / "proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_qtech.so"
    data = load_blob(blob)
    base = find_name_offset(data, "s5k5e8_qtec")

    meta_1c0 = read_words(data, base + 0x1C0, 10)
    assert_word("s5k5e8 meta_0x1c0[5]", meta_1c0[5], 0x034E034C)
    assert_word("s5k5e8 meta_0x1c0[6]", meta_1c0[6], 0x03400342)
    assert_word("s5k5e8 meta_0x1c0[7]", meta_1c0[7], 0x00000202)
    assert_word("s5k5e8 meta_0x1c0[8]", meta_1c0[8], 0x00000204)

    meta_1e8 = read_words(data, base + 0x1E8, 16)
    assert_word("s5k5e8 meta_0x1e8[9]", meta_1e8[9], 0x00004056)

    meta_228 = read_words(data, base + 0x228, 34)
    assert_word("s5k5e8 meta_0x228[5]", meta_228[5], 0x00000A20)
    assert_word("s5k5e8 meta_0x228[6]", meta_228[6], 0x00000798)

    meta_2b0 = read_words(data, base + 0x2B0, 6)
    assert_words("s5k5e8 meta_0x2b0", meta_2b0, [0, 0, 0, 0, 0, 0])

    triplets = read_words(data, base + 0x0F8, 1)
    assert_word("s5k5e8 triplets[0]", triplets[0], 0x00000008)


def main() -> int:
    check_imx386()
    check_ov12a()
    check_s5k5e8()
    print("OK: sensor_open_lib inline blocks match expected values")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
