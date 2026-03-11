# Camera Blob Export Summary

This is a quick index of exported symbols and notable dependencies for the
camera-related proprietary blobs in `proprietary/vendor/lib/`.

## Sensor libraries

Each sensor library exports `sensor_open_lib` and additional internal helpers
that we are reconstructing via inline block dumps.

- `libmmcamera_oxygen_imx386_sunny.so`
- `libmmcamera_oxygen_ov12a_sunny.so`
- `libmmcamera_oxygen_ov12a_ofilm.so`
- `libmmcamera_oxygen_s5k5e8_qtech.so`
- `libmmcamera_oxygen_s5k5e8_ofilm.so`

## EEPROM libraries

### IMX386

`libmmcamera_oxygen_imx386_sunny_eeprom.so`

- `oxygen_imx386_sunny_eeprom_open_lib`
- `oxygen_imx386_sunny_eeprom_get_calibration_items`
- `oxygen_imx386_sunny_eeprom_format_calibration_data`
- depends on `eeprom_whitebalance_calibration`

### OV12A

`libmmcamera_oxygen_ov12a_sunny_eeprom.so`

- `oxygen_ov12a_sunny_eeprom_open_lib`

`libmmcamera_oxygen_ov12a_ofilm_eeprom.so`

- `oxygen_ov12a_ofilm_eeprom_open_lib`

Both depend on `eeprom_whitebalance_calibration` and Android property access.

### S5K5E8

`libmmcamera_oxygen_s5k5e8_qtech_eeprom.so`

- `oxygen_s5k5e8_qtech_eeprom_open_lib`
- `oxygen_s5k5e8_qtech_eeprom_get_calibration_items`
- `oxygen_s5k5e8_qtech_eeprom_format_calibration_data`
- `oxygen_s5k5e8_qtech_get_raw_data`
- exports `g_reg_array` and `g_reg_setting`

The ofilm variant reuses the qtech eeprom library name in `camera_config.xml`.

## Actuator libraries

All three actuator blobs export the single entry point:

- `actuator_driver_open_lib`

Blobs:

- `libactuator_oxygen_dw9763_sunny.so`
- `libactuator_oxygen_ov12a_sunny_dw9763.so`
- `libactuator_oxygen_ov12a_ofilm_dw9718.so`
