# OV12A `sensor_open_lib` Notes

This file captures the current reverse-engineering state of:

- `proprietary/vendor/lib/libmmcamera_oxygen_ov12a_sunny.so`
- `proprietary/vendor/lib/libmmcamera_oxygen_ov12a_ofilm.so`

Both blobs share the same inline `sensor_open_lib` block layout and values.

## Inline block dump summary

Name offsets (file):

- `oxygen_ov12a_sunny`: `0x00004008`
- `oxygen_ov12a_ofilm`: `0x00004008`

### `0x0f8` resolution triplets

The first word is `0x00000006` (six entries), followed by the same triplet
pattern as IMX386.

### `0x1c0` register address pack

```
00: 0x00000000
01: 0x00000000
02: 0x00000001
03: 0x00000001
04: 0x00000000
05: 0x380a3808
06: 0x380e380c
07: 0x00003500
08: 0x00003508
09: 0x00000000
```

The packed addresses match OV register conventions:

- `0x3808/0x380a` => x/y output regs
- `0x380c/0x380e` => line/frame regs
- `0x3500/0x3508` => exposure/gain regs

### `0x1e8` gain/exposure limits (provisional)

```
00: 0x00000000
01: 0x00000008
02: 0x3f800000 (1.0)
03: 0x41780000 (15.5)
04: 0x41780000 (15.5)
09: 0x00007ff7
```

### `0x228` active array + calibration block

```
02: 0x3f9ef9db (1.242)
04: 0x3faa3d71 (1.33)
05: 0x00001240 (4672)
06: 0x00000db0 (3504)
07: 0x00080008
08: 0x00080008
09: 0x004003ff
10: 0x00400040
11: 0x00000040
12: 0x00000001
13: 0x00022b00
```

The active array is `4672x3504`. Black level values are consistent with the
other oxygen sensors.

### `0x2b0`

All-zero (6 words).

## Output info array

The output info array is present at file offset `0x77fc8` with a stride of
`0x40` and 6 populated entries:

- `idx 0`: `4096x3072`, `line=1168`, `frame=3302`, `vt=108000000`, `op=398400000`
- `idx 1`: `2048x1536`, `line=1064`, `frame=3346`, `vt=106900000`, `op=123360000`
- `idx 2`: `4096x2304`, `line=1168`, `frame=3080`, `vt=108000000`, `op=398400000`
- `idx 3`: `3840x2160`, `line=1168`, `frame=3080`, `vt=108000000`, `op=398400000`
- `idx 4`: `1920x1080`, `line=1064`, `frame=3346`, `vt=107400000`, `op=233600000`
- `idx 5`: `1280x720`, `line=1064`, `frame=844`, `vt=107800000`, `op=233600000`

Cross-check against `libmmcamera2_sensor_modules.so` (`sensor_get_output_info`):

- these values are read from fixed offsets relative to the sensor name base:
  - `+0x73fc0 + idx*0x40` (`x`, `y`, `line`, `frame`, `vt`, `op`)
- a companion `u16[4]` table at `+0x75188 + idx*0x08` is also read for each
  mode.
- for both OV12A blobs, all companion entries are zero in populated modes.

That means userspace effectively computes crop endpoints as `x-1` and `y-1`
for OV12A, and takes `line`/`frame` directly from the output-info slot.

## Mode geometry from reg tables

Using `extract_mode_geometry.py` against the OV12A reg tables (start reg
`0x3808`), the following candidate mode sizes appear:

- `4096x3072` line `1168` frame `3302` (duplicate table appears twice)
- `2048x1536` line `1064` frame `3346`
- `4096x2304` line `1168` frame `3080`
- `3840x2160` line `1168` frame `3080`
- `1920x1080` line `1064` frame `3346`
- `1280x720` line `1064` frame `844`

The line-length values are smaller than width for OV12A, but this is accepted
by the Qualcomm userspace path because the fields are consumed as raw timing
values (no normalization in `sensor_get_output_info`).
