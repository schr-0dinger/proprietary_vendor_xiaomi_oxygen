# S5K5E8 `sensor_open_lib` Notes

This file captures the current reverse-engineering state of:

- `proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_qtech.so`
- `proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_ofilm.so`

Both blobs share the same inline `sensor_open_lib` block layout and values.
The inline sensor name string is `s5k5e8_qtec` (note the missing `h`).

## Inline block dump summary

Name offset (file): `0x00004008`

### `0x0f8` resolution triplets

The first word is `0x00000008` (eight entries). The remaining triplet pattern
differs from IMX386/OV12A, and likely reflects a smaller set of supported
modes.

### `0x1c0` register address pack

```
00: 0x00000001
01: 0x00000000
02: 0x00000001
03: 0x00000001
04: 0x00000002
05: 0x034e034c
06: 0x03400342
07: 0x00000202
08: 0x00000204
09: 0x00000000
```

Sony/Samsung-style register addresses are used here (same pack as IMX386).

### `0x1e8` gain/exposure limits (provisional)

```
00: 0x00000000
01: 0x00000008
02: 0x3f800000 (1.0)
03: 0x41800000 (16.0)
04: 0x41800000 (16.0)
09: 0x00004056
```

### `0x228` active array + calibration block

```
02: 0x3f8f5c29 (1.12)
05: 0x00000a20 (2592)
06: 0x00000798 (1944)
07: 0x00080008
08: 0x00080008
09: 0x004003ff
10: 0x00400040
11: 0x00000040
12: 0x00000001
13: 0x00022b00
```

The active array is `2592x1944`. Black level values match the other sensors.

### `0x2b0`

All-zero (6 words).

## Output info array

The output info array is present at file offset `0x77fc8` with a stride of
`0x40`, but only one populated entry was found:

- `idx 0`: `2592x1944`, `line=3136`, `frame=1968`, `vt=184000000`, `op=165600000`

Direct pattern search confirms the array content: the first entry matches the
expected values and the remaining 5 entries are all-zero.

Cross-check against `libmmcamera2_sensor_modules.so` (`sensor_get_output_info`):

- this entry is read via fixed offsets relative to the sensor-name base:
  - `+0x73fc0 + idx*0x40` for output-info fields
  - `+0x75188 + idx*0x08` for a companion `u16[4]` table
- for both QTECH and OFILM blobs, the companion entries are zero for all slots.

So for populated slots, userspace reduces crop-end arithmetic to
`x-1` / `y-1` and uses `line` / `frame` directly from the output-info slot.

## Mode geometry from reg tables

Using `extract_mode_geometry.py` against the S5K5E8 reg tables (start reg
`0x034c`), only one candidate table was found:

- `2592x1944` line `3136` frame `1968`

This likely corresponds to the full-resolution mode. Additional modes may be
configured programmatically or stored under a different register start.

Manual scan shows only a single `0x034c`-anchored reg table with a standard
Samsung-style register sequence (0x034c/0x034d/0x034e/0x034f/0x0340/0x0341/0x0342/0x0343).
No other matching tables were found in this blob.

Next step is to locate additional mode tables or trace code references to
the inline block.
