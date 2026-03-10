# Open Camera Skeleton

This directory holds the camera-side source scaffolding for the `oxygen`
open-vendor effort.

## Scope

The current tree covers all module-specific libraries declared in
[`camera_config.xml`](/home/schr-0dinger/Xiaomi_Kernel/proprietary_vendor_xiaomi_oxygen/proprietary/vendor/etc/camera/camera_config.xml):

- sensor libraries
  - `libmmcamera_oxygen_imx386_sunny.so`
  - `libmmcamera_oxygen_ov12a_sunny.so`
  - `libmmcamera_oxygen_ov12a_ofilm.so`
  - `libmmcamera_oxygen_s5k5e8_qtech.so`
  - `libmmcamera_oxygen_s5k5e8_ofilm.so`
- eeprom libraries
  - `libmmcamera_oxygen_imx386_sunny_eeprom.so`
  - `libmmcamera_oxygen_ov12a_sunny_eeprom.so`
  - `libmmcamera_oxygen_ov12a_ofilm_eeprom.so`
  - `libmmcamera_oxygen_s5k5e8_qtech_eeprom.so`
- actuator libraries
  - `libactuator_oxygen_dw9763_sunny.so`
  - `libactuator_oxygen_ov12a_sunny_dw9763.so`
  - `libactuator_oxygen_ov12a_ofilm_dw9718.so`

## Layout

- `include/qcom_sensor_compat.h`
  - Minimal Qualcomm camera type definitions copied from the local downstream
    kernel headers.
- `include/qcom_camera_stub.h`
  - Shared scaffolding types used by the generic sensor/eeprom/actuator stubs.
- `imx386/`
  - RE-backed IMX386 source skeleton.
- `stubs/`
  - Generic source stubs for the remaining oxygen-specific camera libraries.

## Current limits

- Only IMX386 contains recovered structure data.
- The generic stubs preserve module names and export entry points, but do not
  yet claim ABI compatibility with Qualcomm camera userspace.
- No camera library from this directory is wired into mixed-vendor replacement
  yet.
