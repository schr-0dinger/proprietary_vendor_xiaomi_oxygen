# OV12A Open Sensor Skeleton

This directory contains the second RE-backed sensor adapter family for oxygen:

- `libmmcamera_oxygen_ov12a_sunny.so`
- `libmmcamera_oxygen_ov12a_ofilm.so`

## What it preserves

- exported symbol name: `sensor_open_lib`
- vendor-specific sensor names for both Sunny and Ofilm variants
- recovered inline descriptor slices at:
  - `+0x0f8`
  - `+0x1c0`
  - `+0x1e8`
  - `+0x228`
  - `+0x2b0`
- six recovered output-info table entries

## What it does not claim yet

- recovered mode register tables
- runtime parity with Qualcomm camera userspace
- stream-mask-aware mode selection

## Current integration boundary

The OV12A core mirrors the IMX386 pre-runtime surface:

- typed `sensor_open_lib()` layout
- typed output-info queries
- exact-resolution request wrapper
- shared core query helpers reused across sensor families
