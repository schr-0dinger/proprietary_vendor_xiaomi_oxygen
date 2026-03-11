# Camera Target List (from `camera_config.xml` + index)

This file maps the camera modules in `proprietary/vendor/etc/camera/camera_config.xml`
to the matching blobs we have in the tree. This is the active RE priority list.

## Sensor libraries (`sensor_open_lib`)

- `libmmcamera_oxygen_imx386_sunny.so`
- `libmmcamera_oxygen_ov12a_sunny.so`
- `libmmcamera_oxygen_ov12a_ofilm.so`
- `libmmcamera_oxygen_s5k5e8_qtech.so`
- `libmmcamera_oxygen_s5k5e8_ofilm.so`

## EEPROM helpers

- `libmmcamera_oxygen_imx386_sunny_eeprom.so`
- `libmmcamera_oxygen_ov12a_sunny_eeprom.so`
- `libmmcamera_oxygen_ov12a_ofilm_eeprom.so`
- `libmmcamera_oxygen_s5k5e8_qtech_eeprom.so`

## Actuator libraries

- `libactuator_oxygen_dw9763_sunny.so`
- `libactuator_oxygen_ov12a_sunny_dw9763.so`
- `libactuator_oxygen_ov12a_ofilm_dw9718.so`

## Chromatix XMLs (tuning selection)

- `oxygen_imx386_sunny_chromatix.xml`
- `oxygen_ov12a_sunny_chromatix.xml`
- `oxygen_ov12a_ofilm_chromatix.xml`
- `oxygen_s5k5e8_qtech_chromatix.xml`
- `oxygen_s5k5e8_ofilm_chromatix.xml`

## Priority order

1. OV12A (`sensor_open_lib` + output_info + remaining `0x6230` field decode).
2. S5K5E8 (`sensor_open_lib` + verify additional modes or confirm single-mode design).
3. OV12A EEPROM + actuators.
4. S5K5E8 EEPROM (QTECH only in tree; OFILM appears to reuse QTECH EEPROM).
5. IMX386 EEPROM + actuator (only after the IMX386 `sensor_open_lib` ABI is stable).
