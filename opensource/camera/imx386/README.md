# IMX386 Open Sensor Skeleton

This directory contains the first source skeleton for `libmmcamera_oxygen_imx386_sunny.so`.

## What it preserves

- exported symbol name: `sensor_open_lib`
- sensor name: `oxygen_imx386_sunny`
- six recovered mode tables from [`RE/imx386_regs.c`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/RE/imx386_regs.c)
- exact recovered offsets for the inline blocks currently anchored at:
  - `+0x0f8`
  - `+0x1c0`
  - `+0x228`
  - `+0x2b0`

## What it does not claim yet

- ABI correctness versus Qualcomm's real userspace `sensor_lib` layout
- runtime compatibility with `mm-qcamera-daemon`
- complete output-info decoding for all six resolution slots
- recovered callback table or PDAF helper wiring

## Current integration boundary

The IMX386 scaffold now exposes conservative typed helpers for the first
module-side query surfaces:

- mode lookup by index
- exact-resolution mode selection
- packaged output-info query
- request/result wrapper carrying `stream_mask`

Current limitation:

- the request wrapper preserves `stream_mask` in the API, but the current policy
  still uses exact-resolution-first selection only
- for duplicate resolutions such as `1920x1080`, the first matching mode wins
  until runtime integration proves the real chooser policy

The current value is that the RE is now represented as compileable source, with the blob layout turned into explicit code offsets instead of only markdown notes.
