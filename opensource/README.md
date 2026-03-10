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

## Current limits

- The open `consumerir` HAL preserves the Android legacy `consumerir` ABI and exact installed filename.
- The backend split is real, but the downstream path is still a compatibility-oriented reconstruction rather than a hardware-verified bit-for-bit clone of Xiaomi's blob protocol.
- Camera, sensors, and fingerprint scaffolding are still policy/inventory targets only. The first camera RE work remains under [`RE/`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/RE).
