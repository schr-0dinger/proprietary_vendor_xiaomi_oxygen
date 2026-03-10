# Open Fingerprint Skeleton

This directory is the starting point for `vendor/lib64/hw/fingerprint.msm8953.so`.

## Intent

- create a concrete source location for the future fingerprint gating shim
- keep boot non-blocking by default
- preserve room for a future disabled/passthrough split without enabling it yet

## Current shape

- local compat headers define a minimal legacy fingerprint HAL surface
- backend selection supports:
  - `disabled`
  - `passthrough`
  - `auto`
- default behavior is `disabled`
- the module is not wired into `vendor_overrides.mk`

## Current limits

- no TEE, Goodix, or HIDL passthrough integration yet
- no runtime validation
- intended as source scaffolding only
