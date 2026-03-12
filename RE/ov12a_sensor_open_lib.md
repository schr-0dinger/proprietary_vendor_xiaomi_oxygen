# OV12A `sensor_open_lib` Notes

This file captures the current offline reference state for
`proprietary/vendor/lib/libmmcamera_oxygen_ov12a_sunny.so`.

The goal is to use OV12A as the second sensor-family cross-check and the next
static reconstruction target while IMX386 waits for device/runtime validation.

## Current status

OV12A has now reached the same pre-runtime checkpoint as IMX386:

1. typed `sensor_open_lib()` layout exists in `opensource/camera/ov12a/`
2. shared Sunny/Ofilm core preserves the proven inline slices
3. host-side query tests validate the recovered output-info and metadata values

## Confirmed anchors

- Sensor name: `oxygen_ov12a_sunny`
- Returned object base: inline name starts at file offset `0x4008`

## Stable top-level blocks

### `obj+0x1c0` (10 words)

```text
0x00000000 0x00000000 0x00000001 0x00000001 0x00000000
0x380a3808 0x380e380c 0x00003500 0x00003508 0x00000000
```

Current interpretation:

- same structural role as IMX386 `0x1c0` tail
- OV12A-specific register values:
  - packed output registers: `0x380a3808`
  - packed frame/line registers: `0x380e380c`
  - coarse integration register: `0x3500`
  - gain register: `0x3508`

### `obj+0x1e8` (16 words)

```text
0x00000000 0x00000008 0x3f800000 0x41780000
0x41780000 0x00000000 0x00000000 0x00000000
0x00000000 0x00007ff7 0x00000000 0x00000000
0x00000000 0x00000000 0x00000000 0x00000000
```

Current interpretation:

- same copied 0x28-byte context-block role as IMX386
- field names still intentionally unresolved

### `obj+0x228` (34 words)

```text
0x00020002 0x00000002 0x3f9ef9db 0x00000002
0x3faa3d71 0x00001240 0x00000db0 0x00080008
0x00080008 0x004003ff 0x00400040 0x00000040
0x00000001 0x00022b00 ...
```

Current interpretation:

- same structural role as IMX386 `0x228` block
- proven stable header:
  - `0x228 = 0x00020002`
  - `0x22c = 0x00000002`
  - `0x230 = 0x3f9ef9db` (float-like, sensor-specific)
- words `5` and `6` match the recovered OV12A geometry/timing values used in
  the existing host-side block test:
  - `0x1240`
  - `0x0db0`

### `obj+0x2b0` (6 words)

```text
0x00000000 0x00000000 0x00000000
0x00000000 0x00000000 0x00000000
```

Current interpretation:

- all-zero tail block on OV12A, matching the current block test

## Recovered output-info table

Host-side reference values already checked in `RE/tests/test_sensor_blocks.py`:

1. `4096x3072`, `line=1168`, `frame=3302`, `vt=108000000`, `op=398400000`
2. `2048x1536`, `line=1064`, `frame=3346`, `vt=106900000`, `op=123360000`
3. `4096x2304`, `line=1168`, `frame=3080`, `vt=108000000`, `op=398400000`
4. `3840x2160`, `line=1168`, `frame=3080`, `vt=108000000`, `op=398400000`
5. `1920x1080`, `line=1064`, `frame=3346`, `vt=107400000`, `op=233600000`
6. `1280x720`, `line=1064`, `frame=844`, `vt=107800000`, `op=233600000`

## Offline plan for OV12A

1. Keep the shared Sunny/Ofilm OV12A core stable unless new blob evidence
   proves the variants diverge.
2. Reuse OV12A as the second-family validation target for shared query surfaces.
3. Shift the next offline reconstruction effort to S5K5E8 while IMX386 and
   OV12A wait for runtime testing on device.
