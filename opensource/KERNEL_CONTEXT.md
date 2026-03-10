# Kernel Context

This repo now has the two kernel reference trees that matter most to the
open-vendor effort:

- downstream truth:
  [`android_kernel_xiaomi_oxygen/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen)
  - Linux `4.9.337`
- mainline target:
  [`linux/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/linux)
  - Linux `6.12`

## Why this matters

- The downstream tree defines the currently working device-specific kernel
  contracts that the proprietary userspace was written against.
- The mainline tree defines the direction the open replacements should converge
  on, especially for sensors, power, and other kernel-facing interfaces.

## Immediate subsystem anchors

- consumer IR, downstream
  - DTS:
    [`android_kernel_xiaomi_oxygen/arch/arm64/boot/dts/qcom/oxygen/misc.dtsi`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen/arch/arm64/boot/dts/qcom/oxygen/misc.dtsi)
  - driver:
    [`android_kernel_xiaomi_oxygen/drivers/media/rc/ir-spi.c`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen/drivers/media/rc/ir-spi.c)

- fingerprint, downstream
  - DTS:
    [`android_kernel_xiaomi_oxygen/arch/arm64/boot/dts/qcom/oxygen/fingerprint.dtsi`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen/arch/arm64/boot/dts/qcom/oxygen/fingerprint.dtsi)
  - driver family:
    [`android_kernel_xiaomi_oxygen/drivers/input/fingerprint/goodix_ta/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen/drivers/input/fingerprint/goodix_ta)

- camera, downstream
  - driver family:
    [`android_kernel_xiaomi_oxygen/drivers/media/platform/msm/camera_v2/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen/drivers/media/platform/msm/camera_v2)

- mainline msm8953 Xiaomi references
  - common:
    [`linux/arch/arm64/boot/dts/qcom/msm8953-xiaomi-common.dtsi`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/linux/arch/arm64/boot/dts/qcom/msm8953-xiaomi-common.dtsi)
  - mido:
    [`linux/arch/arm64/boot/dts/qcom/msm8953-xiaomi-mido.dts`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/linux/arch/arm64/boot/dts/qcom/msm8953-xiaomi-mido.dts)
  - vince:
    [`linux/arch/arm64/boot/dts/qcom/msm8953-xiaomi-vince.dts`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/linux/arch/arm64/boot/dts/qcom/msm8953-xiaomi-vince.dts)

## Important current gap

The mainline tree has Xiaomi `msm8953` references for `mido` and `vince`, but
not for `oxygen` yet. That means:

- downstream remains the authoritative source for current `oxygen` hardware
  wiring
- `mido` and `vince` mainline DTS files are reference material, not drop-in
  replacements
- open vendor code should avoid baking downstream-only assumptions into
  top-level HAL logic, but it still needs downstream-specific backends today
