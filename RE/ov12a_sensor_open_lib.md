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

A heuristic scan for a 6-entry `msm_sensor_output_info_t` array (stride `0x40`)
did not find a plausible sequence yet. This may be stored elsewhere or
constructed at runtime for OV12A.

Next step is to locate the output info table by tracing code references to
the inline block or by matching known mode registers.
