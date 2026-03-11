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

A heuristic scan for an 8-entry `msm_sensor_output_info_t` array (stride `0x40`)
did not find a plausible sequence yet. This may be stored elsewhere or
constructed at runtime for S5K5E8.

Next step is to locate the output info table by tracing code references to
the inline block or by matching known mode registers.
