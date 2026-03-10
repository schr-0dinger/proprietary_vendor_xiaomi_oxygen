# Open Consumer IR HAL

This directory contains the first runtime-oriented open replacement in the
`oxygen` open-vendor effort: `consumerir.msm8953.so`.

## Current contract

- Preserves the legacy Android `consumerir` HAL module name and install path:
  `vendor/lib64/hw/consumerir.msm8953.so`
- Keeps the frontend ABI stable and splits kernel-facing behavior into:
  - `backend/downstream`
  - `backend/mainline`

## Downstream basis

The downstream path is grounded against the local kernel tree:

- DTS node:
  [`../android_kernel_xiaomi_oxygen/arch/arm64/boot/dts/qcom/oxygen/misc.dtsi`](/home/schr-0dinger/Xiaomi_Kernel/android_kernel_xiaomi_oxygen/arch/arm64/boot/dts/qcom/oxygen/misc.dtsi)
  defines an `ir-spi` device on `spi_8`.
- Driver:
  [`../android_kernel_xiaomi_oxygen/drivers/media/rc/ir-spi.c`](/home/schr-0dinger/Xiaomi_Kernel/android_kernel_xiaomi_oxygen/drivers/media/rc/ir-spi.c)
  exposes `/dev/lirc0` and uses `LIRC_SET_REC_FILTER` to set the TX buffer
  length before `write()`.

That means the blob's `0x4004691c` ioctl is not an opaque Xiaomi-private call.
It is the standard `LIRC_SET_REC_FILTER` slot being repurposed by the downstream
driver.

## Backend selection

Selection is runtime-probed:

- `downstream` when an `ir-spi` SPI driver or compatible string is visible
- `mainline` when generic rc-core userspace markers are visible
- fallback defaults to `downstream`

Override for bring-up or testing:

```sh
export OXYGEN_CONSUMERIR_BACKEND=mainline
```

Accepted values are `downstream`, `mainline`, and `auto`.

## Current limits

- Not hardware-verified on a running device yet.
- Mainline support is still a compatibility path, not a confirmed upstream
  `oxygen` backend.
- Carrier frequency ranges are still static and conservative.
