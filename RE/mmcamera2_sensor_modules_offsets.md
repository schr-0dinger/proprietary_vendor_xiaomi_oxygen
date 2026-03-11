# mmcamera2_sensor_modules Sensor Lib Offset Notes

This file captures concrete field accesses observed in
`RE/libmmcamera2_sensor_modules.so` for the sensor library object returned by
`sensor_open_lib()`. The goal is to anchor the `0x1e8` and `0x228` inline blocks
to real usage in Qualcomm userspace.

Offsets below are relative to the pointer stored at `sensor_open_lib()` return
value (the same base used in `RE/imx386_sensor_open_lib.md`).

## Key accesses (by address)

### `0x00013078` - `0x000130ca`

```
0x00013078 vldr s0, [r6, 0x21c]
0x00013080 ldr.w r0, [r6, 0x228]
0x00013088 ldr.w r1, [r6, 0x22c]
0x00013092 ldr.w r1, [r6, 0x220]
0x000130a2 vldr s2, [r6, 0x230]
0x000130aa vdiv.f32 s2, s16, s2      ; s16 = 1.0
0x000130b6 vldr s2, [r6, 0x220]
```

Observed behavior:

- `0x230` is treated as a float. The code computes `1.0 / *(float *)(base+0x230)`.
- `0x228` and `0x22c` are copied as raw 32-bit words into a larger output struct.
- `0x21c` and `0x220` are treated as floats and fed into downstream math.

This is consistent with `0x230` being pixel size in microns (IMX386=1.25,
S5K5E8=1.12, OV12A=1.242).

### `0x0002da44` - `0x0002da72`

```
0x0002da44 ldr.w r2, [r0, 0x228]
0x0002da4c ldr.w r0, [r0, 0x22c]
0x0002da4a uxth r3, r2
0x0002da54 lsr.w r3, r2, 0x10
0x0002da64 subs r2, r2, r0
0x0002da72 strh.w r0, [sl, 0x2c]
```

Observed behavior:

- `0x228` is treated as two packed `u16` values (low/high halfwords).
- `0x22c` is a `u32` used as a comparison/subtraction reference.

This implies the first word of the `0x228` block is a packed pair of 16-bit
values used in crop/constraint logic.

### `0x0003228e` - `0x000322cc`

```
0x0003228e ldrh.w r0, [r0, 0x228]
0x00032292 strh.w r0, [sl, 0x2c]
0x000322be ldr.w r0, [r0, 0x1f4]
0x000322c2 str.w r0, [sl, 0x30]
0x000322c8 ldr.w r0, [r0, 0x20c]
0x000322cc str.w r0, [sl, 0x34]
```

Observed behavior:

- `0x228` low halfword is exported into an output struct as `u16`.
- `0x1f4` (word 3 in the `0x1e8` block) is copied as a 32-bit float value.
- `0x20c` (word 9 in the `0x1e8` block) is copied as a raw 32-bit value.

This strongly suggests:

- `0x1f4` = max gain (float, 15.5 or 16.0).
- `0x20c` = coarse integration / exposure limit (sensor-specific integer).

### `0x00035cb4` - `0x00035ce2`

```
0x00035cb4 ldrh.w r6, [r0, 0x1e8]
0x00035cdc ldrh.w r3, [r0, 0x1e8]
0x00035ce0 adds r3, 1
```

Observed behavior:

- `0x1e8` is also accessed as a `u16` in a mode-table style loop.
- This suggests the first word of the `0x1e8` block is treated as a small count
  or selector, not just a float.

## Working interpretation (current)

These are not final field names, but they are supported by the access patterns:

- `0x1e8 + 0x0` (word 0): used as `u16` and float in different contexts; likely
  a small integer selector that is also stored as float in some paths.
- `0x1f4` (word 3): max gain (float).
- `0x20c` (word 9): coarse integration max / linecount limit (u32).
- `0x228` (word 0): packed `u16` pair used in crop/constraint math.
- `0x22c` (word 1): `u32` parameter used alongside the packed pair.
- `0x230` (word 2): pixel size in microns (float).

Next step is to tie these offsets to a known Qualcomm `sensor_lib_t` header or
to follow the consuming functions further to identify exact field names.
