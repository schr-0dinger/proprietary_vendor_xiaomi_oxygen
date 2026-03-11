# Open Vendor Scaffolding

This directory is the first concrete layer of the `oxygen` port-first open-vendor plan.

Current priority:

- primary target: `oxygen` on the downstream `4.9` kernel
- mainline is now a tracked blocker and a parallel compatibility target
  (still secondary to downstream bring-up)

## What is here

- `tools/vendor_inventory.py`
  - Builds a machine-readable inventory from `oxygen-vendor.mk`, `mithorium-common-vendor.mk`, `Android.bp`, and `proprietary/vendor/etc/camera/camera_config.xml`.
  - Annotates each artifact with layer, partition, kind, architecture, subsystem, policy, and a small dependency graph.
- `policy/vendor_policy.json`
  - Default replacement policy map.
  - Current default stance is conservative: camera, sensors, and IR are replacement targets; fingerprint is a shim target; radio/IMS/data, location, DRM/TEE, and firmware stay binary by default.
  - Graphics/GPU stays binary for downstream, but the mainline path will prefer Freedreno + Mesa while keeping firmware proprietary.
- `vendor_overrides.mk`
  - Mixed-vendor overlay that filters selected prebuilts out of the generated `PRODUCT_COPY_FILES` lists and replaces them with source modules.
- `consumerir/`
  - First open replacement module, installed as `vendor/lib64/hw/consumerir.msm8953.so`.
  - Uses a backend split, but the downstream `4.9` backend is the primary target today.
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
  - Keeps the planned backend split in place, with downstream `4.9` as the primary compatibility goal.
- `fingerprint/`
  - Source skeleton for `vendor/lib64/hw/fingerprint.msm8953.so`.
  - Starts as a non-operational gating shim so the repo has a concrete place for the future disabled/passthrough split.
- `KERNEL_CONTEXT.md`
  - Maps the local downstream and mainline kernel trees to the vendor-side open-source effort.
  - Records downstream `4.9` as the primary implementation target, with mainline tracked in parallel.

## Mixed-vendor enablement

The generated vendor manifests stay unchanged by default.

To enable the open IR HAL in a product or device makefile:

```make
OXYGEN_OPEN_VENDOR_COMPONENTS += consumerir
```

Additional component toggles now exist:

```make
OXYGEN_OPEN_VENDOR_COMPONENTS += sensors
OXYGEN_OPEN_VENDOR_COMPONENTS += fingerprint
OXYGEN_OPEN_VENDOR_COMPONENTS += camera-imx386
OXYGEN_OPEN_VENDOR_COMPONENTS += camera-stubs
OXYGEN_OPEN_VENDOR_COMPONENTS += camera-all
```

Current meaning:

- `consumerir`
  - real open replacement target
- `sensors`
  - source skeleton for `vendor/bin/sensors.qti`
- `fingerprint`
  - source skeleton for `vendor/lib64/hw/fingerprint.msm8953.so`
- `camera-imx386`
  - swaps in the IMX386 sensor/eeprom/actuator source set
- `camera-stubs`
  - swaps in the generic OV12A and S5K5E8 source stubs
- `camera-all`
  - enables both camera groups above

These toggles are for downstream `4.9` bring-up first. Mainline support is tracked in
parallel, but not promised for every component yet.

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

The most useful local kernel inputs for the current implementation phase are:

- the local downstream `oxygen` kernel tree at
  [`android_kernel_xiaomi_oxygen/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/android_kernel_xiaomi_oxygen)
- the local mainline reference tree at
  [`linux/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/linux)
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
- The new mixed-vendor toggles for `sensors`, `fingerprint`, `camera-imx386`, `camera-stubs`, and `camera-all` are build-selection hooks for controlled downstream `4.9` bring-up work, not for production boot yet.
