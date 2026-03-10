# Open Sensors Skeleton

This directory is the first source skeleton for `vendor/bin/sensors.qti`.

Current priority is downstream `4.9` compatibility. The mainline path is
reference-only for now.

## Intent

- keep a real build target for the future open sensors daemon
- preserve a backend split without losing focus on downstream `4.9` bring-up
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
