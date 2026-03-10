# Open Vendor Scaffolding

This directory is the first concrete layer of the `oxygen` port-first open-vendor plan.

## What is here

- `tools/vendor_inventory.py`
  - Builds a machine-readable inventory from `oxygen-vendor.mk`, `mithorium-common-vendor.mk`, `Android.bp`, and `proprietary/vendor/etc/camera/camera_config.xml`.
  - Annotates each artifact with layer, partition, kind, architecture, subsystem, policy, and a small dependency graph.
- `policy/vendor_policy.json`
  - Default replacement policy map.
  - Current default stance is conservative: camera, sensors, and IR are replacement targets; fingerprint is a shim target; radio/IMS/data, GPU/media, location, DRM/TEE, and firmware stay binary by default.
- `vendor_overrides.mk`
  - Mixed-vendor overlay that filters selected prebuilts out of the generated `PRODUCT_COPY_FILES` lists and replaces them with source modules.
- `consumerir/`
  - First open replacement module, installed as `vendor/lib64/hw/consumerir.msm8953.so`.
  - Uses a backend split:
    - `backend/downstream`
    - `backend/mainline`
  - Backend selection is auto-probed at runtime and can be overridden with `OXYGEN_CONSUMERIR_BACKEND=downstream|mainline|auto`.
- `camera/`
  - Shared camera compat headers plus source skeletons for the oxygen camera family.
  - Includes one RE-backed sensor implementation for IMX386 and generic stubs for the remaining sensor, eeprom, and actuator libraries from `camera_config.xml`.
- `camera/imx386/`
  - First source skeleton for `libmmcamera_oxygen_imx386_sunny.so`.
  - Encodes the recovered `sensor_open_lib` inline layout as exact offsets and exposes the six mode tables in Qualcomm-style `msm_camera_i2c_reg_setting` form.
  - This is a research module, not a runtime-ready blob replacement yet.
- `sensors/`
  - Source skeleton for `vendor/bin/sensors.qti`.
  - Keeps the planned backend split in place without claiming runtime compatibility with SSC/DSP or the Qualcomm sensors stack yet.
- `fingerprint/`
  - Source skeleton for `vendor/lib64/hw/fingerprint.msm8953.so`.
  - Starts as a non-operational gating shim so the repo has a concrete place for the future disabled/passthrough split.

## Mixed-vendor enablement

The generated vendor manifests stay unchanged by default.

To enable the open IR HAL in a product or device makefile:

```make
OXYGEN_OPEN_VENDOR_COMPONENTS += consumerir
```

Both [`oxygen-vendor.mk`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/oxygen-vendor.mk) and [`mithorium-common-vendor.mk`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/mithorium-common-vendor.mk) now include this overlay at the end, so replacements can be layered on top without rewriting the generated blob lists.

## Inventory usage

Summary only:

```bash
python3 opensource/tools/vendor_inventory.py --summary
```

Full JSON inventory:

```bash
python3 opensource/tools/vendor_inventory.py > /tmp/oxygen-vendor-inventory.json
```

## Helpful Local Inputs

The most useful extra reference trees for the next implementation phase are:

- a stable custom or downstream `oxygen` kernel tree with the currently working
  drivers and DTS
- any `msm8953-mainline` tree or patch stack you want this work to converge on
- extracted init logs or service failure logs once runtime enablement begins

Useful but lower priority:

- firmware and calibration blobs that are loaded by kept-binary components
- ROM-side manifests or init changes from the `mido`/`vince` bases you expect
  to port from

## Current limits

- The open `consumerir` HAL preserves the Android legacy `consumerir` ABI and exact installed filename.
- The downstream `consumerir` path is now grounded against `android_kernel_xiaomi_oxygen/drivers/media/rc/ir-spi.c`: it uses `LIRC_SET_REC_FILTER` as the TX-length ioctl before `write()`. It is still not hardware-verified on-device here.
- Camera, sensors, and fingerprint scaffolding are present as source now, but only `consumerir` is wired into [`vendor_overrides.mk`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/opensource/vendor_overrides.mk).
- The IMX386 camera skeleton compiles as source scaffolding, but it is not yet wired into `vendor_overrides.mk` because the full Qualcomm `sensor_open_lib()` userspace ABI is still being reconstructed.
- The generic camera/eeprom/actuator stubs, the `sensors.qti` skeleton, and the fingerprint shim are coverage scaffolding only. They exist to anchor future RE and implementation work, not to replace blobs yet.
