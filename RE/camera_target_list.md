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

## Current TODO

1. Finish IMX386 `sensor_open_lib()` ABI documentation.
   - name remaining raw top-level `obj+0x1e8..0x20f` fields only when directly proven
   - finish helper subobject selector-family notes (`0x70`, `0x100`, `0x110`, `0xc0`)
   - keep raw top-level object and helper subobject as separate pointer types
2. Freeze the first compile-safe IMX386 replacement layout.
   - keep uncertain blocks as raw words
   - preserve proven register-address block and mode/output tables
   - avoid assigning lens metadata to the wrong pointer type
3. Map only the required `libmmcamera2_sensor_modules.so` ABI for one working path.
   - probe/session export
   - output info
   - required control/exposure paths for IMX386 bring-up
4. Validate and extend the open IMX386 scaffold.
   - keep `opensource/camera/imx386/` aligned with proven blob contents
   - implement required callbacks after the ABI slice is frozen
   - compare against proprietary blob values where practical
5. After IMX386 sensor blob stability, move to adjacent camera pieces.
   - IMX386 EEPROM
   - IMX386 actuator
   - chromatix/tuning integration later, after functional bring-up
6. Cross-check second sensor family for shared versus sensor-specific fields.
   - OV12A next for shared-layout validation
   - S5K5E8 after that

## Endpoint

This RE track ends when the IMX386 path is good enough to write and validate a
minimal open replacement without guessing unstable fields.

Concrete stop condition:

1. `sensor_open_lib()` layout is frozen for the fields the stack actually reads.
2. Helper selector families (`0x70`, `0x100`, `0x110`, `0xc0`) are understood
   enough to preserve behavior without inventing semantics.
3. The minimum required `libmmcamera2_sensor_modules.so` ABI for one working
   IMX386 path is mapped.
4. `opensource/camera/imx386/` can return a compile-safe descriptor and survive
   first bring-up testing.

This does not require "perfect documentation" of every field in every blob.
It requires a stable enough ABI slice for one working sensor replacement.

## Current Handoff

The IMX386 work has now reached the last pre-runtime checkpoint:

1. The descriptor layout is compile-checked with typed substructures.
2. The first behavior-facing helpers (`imx386_query_mode()` and
   `imx386_query_output_info()`) are implemented.
3. A compiled local test confirms those helpers match the expected proprietary
   IMX386 values for checked modes.

The next stage is runtime validation on the real camera stack or a tighter
module-side integration test. Further static RE is no longer the highest-value
step for IMX386 bring-up.

OV12A now matches that offline checkpoint:

1. typed descriptor layout exists for both Sunny and Ofilm variants
2. shared query helpers are validated by host-side tests
3. Sunny and Ofilm currently share one static core because their recovered
   descriptor slices and output-info tables match

## No-Device Plan

Until the device is available again, the highest-value camera work is:

1. Keep IMX386 stable and avoid open-ended static churn.
   - only fix issues discovered by host-side tests
   - do not keep renaming unresolved fields without new evidence
2. Keep OV12A on the same pre-runtime, host-validated track.
   - reuse the shared sensor-core query surface
   - keep Sunny and Ofilm variants on one static core unless new evidence breaks that assumption
3. Start the third-family offline track with S5K5E8.
   - verify whether the qtech/ofilm split is real or just packaging
   - recover enough output-info data to reach the same query-helper checkpoint
4. Prepare host-side comparison tests so runtime time is spent on integration,
   not on basic descriptor mismatches.
5. Defer EEPROM / actuator / chromatix until either:
   - IMX386 runtime testing starts, or
   - S5K5E8 reaches the same pre-runtime checkpoint
