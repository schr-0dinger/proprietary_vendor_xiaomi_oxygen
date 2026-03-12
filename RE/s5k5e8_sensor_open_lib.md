# S5K5E8 `sensor_open_lib` Notes

This file starts the third-family offline track for:

- `proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_qtech.so`
- `proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_ofilm.so`

The goal is to bound the next implementation task before runtime testing starts.

## Confirmed qtech anchors

- Sensor name: `s5k5e8_qtec`
- Returned object base: inline name starts at file offset `0x4008`

## Stable top-level qtech blocks

### `obj+0x1c0` (10 words)

```text
0x00000001 0x00000000 0x00000001 0x00000001 0x00000002
0x034e034c 0x03400342 0x00000202 0x00000204 0x00000000
```

### `obj+0x1e8` (16 words)

```text
0x00000000 0x00000008 0x3f800000 0x41800000
0x41800000 0x00000000 0x00000000 0x00000000
0x00000000 0x00004056 0x00000000 0x00000000
0x00000000 0x00000000 0x00000000 0x00000000
```

### `obj+0x228` (34 words)

```text
0x00020002 0x00000002 0x3f8f5c29 0x00000002
0x00000000 0x00000a20 0x00000798 0x00080008
0x00080008 0x004003ff 0x00400040 0x00000040
0x00000001 0x00022b00 ...
```

### `obj+0x2b0` (6 words)

```text
0x00000000 0x00000000 0x00000000
0x00000000 0x00000000 0x00000000
```

## Current qtech output-info state

- `triplets[0] = 0x8`
- only the first output-info entry is currently non-zero in the existing host check:

```text
2592x1944 line=3136 frame=1968 vt=184000000 op=165600000 binning=1
```

The remaining seven slots are still zero in the current quick extraction and
need confirmation before we treat S5K5E8 as a real multi-mode adapter.

## Ofilm note

`libmmcamera_oxygen_s5k5e8_ofilm.so` contains the string
`oxygen_s5k5e8_ofilm`, but the quick `needle + "\\0"` base search used for the
other families did not produce a clean top-level base match yet.

That means the next S5K5E8 step is not implementation first. It is:

1. resolve the correct top-level object base for the Ofilm blob
2. verify whether Ofilm shares the qtech descriptor layout
3. only then decide whether one shared S5K5E8 core is valid
