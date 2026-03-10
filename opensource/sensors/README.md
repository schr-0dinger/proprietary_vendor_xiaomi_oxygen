# Open Sensors Skeleton

This directory is the first source skeleton for `vendor/bin/sensors.qti`.

## Intent

- keep a real build target for the future open sensors daemon
- preserve the planned backend split between downstream and mainline kernels
- avoid claiming runtime compatibility with Qualcomm SSC/DSP userspace before
  the protocol and service contract are reconstructed

## Current behavior

- `sensors.qti.open` builds a binary named `sensors.qti`
- backend selection supports:
  - `downstream`
  - `mainline`
  - `auto`
- the binary is documentation-grade scaffolding only and is not wired into
  `vendor_overrides.mk`

## Likely next inputs

- stable downstream/custom kernel source with the working sensors stack
- init/service expectations for the target ROM base
- logs from the proprietary `sensors.qti` process once runtime work starts
