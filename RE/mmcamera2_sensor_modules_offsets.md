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

Instruction-faithful pseudocode for this block:

```c
uint32_t w228 = *(uint32_t *)(base + 0x228);
uint32_t ref  = *(uint32_t *)(base + 0x22c);
uint32_t lo = w228 & 0xffff;
uint32_t hi = (w228 >> 16) & 0xffff;

uint32_t cand = (lo < hi) ? hi : lo;
if (hi < ref) {
  cand -= ref;  // 32-bit wrap
}

uint32_t out = (ref > 0) ? (ref - 1) : ref;
if ((int32_t)cand >= 0) {
  out = cand;
}

*(uint16_t *)(dst + 0x2c) = (uint16_t)out;
```

Empirical check with `RE/tools/sensor_0x228_eval.py`:

- `ov12a_sunny`: `w228=0x00020002`, `ref=2`, result `u16=2`
- `s5k5e8_qtech` (inline name `s5k5e8_qtec`): same, result `u16=2`
- `imx386_sunny`: same, result `u16=2`

For oxygen camera blobs, this path currently collapses to a constant output
`2` because both packed halfwords and reference are all `2`.

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

### `0x0002df6a` - `0x0002dff2` (`sensor_get_output_info`)

This path reads output-info fields from fixed offsets relative to the
`sensor_open_lib()` base pointer:

```
+0x73fc0 + idx*0x40  (u16) x_output
+0x73fc2 + idx*0x40  (u16) y_output
+0x73fc4 + idx*0x40  (u16) line_length_pclk
+0x73fc6 + idx*0x40  (u16) frame_length_lines
+0x73fc8 + idx*0x40  (u32) vt_pixel_clk
+0x73fcc + idx*0x40  (u32) op_pixel_clk
```

And a companion table:

```
+0x75188 + idx*0x08  (u16) a0
+0x7518a + idx*0x08  (u16) a1
+0x7518c + idx*0x08  (u16) a2
+0x7518e + idx*0x08  (u16) a3
```

Observed arithmetic:

```
crop_end_x = x_output - 1 - a3
crop_end_y = y_output - 1 - a1
```

For current oxygen blobs (`imx386`, `ov12a`, `s5k5e8`), `a0..a3` are all zero
for populated modes, so this path reduces to `(x-1, y-1)`.

Implication:

- The `line` / `frame` values in `output_info` are consumed as raw `u16` timing
  fields directly from this table (no unit normalization in this path).
- This explains why OV12A can report `line < width` while remaining accepted by
  userspace.

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
