# Reverse Engineering Notes

This tree is large enough that blob replacement needs triage before coding.

## Priority order

1. Camera sensor libraries
2. Small device HALs with stable AOSP/kernel interfaces
3. Java jars/apks that can be replaced by stubs or dropped
4. DSP, radio, GPU, Widevine, and firmware blobs that are not realistic short-term OSS targets

## Camera map for oxygen

The vendor camera config in `proprietary/vendor/etc/camera/camera_config.xml` names five sensor modules:

- `oxygen_imx386_sunny`
- `oxygen_ov12a_sunny`
- `oxygen_ov12a_ofilm`
- `oxygen_s5k5e8_qtech`
- `oxygen_s5k5e8_ofilm`

For each module, reconstruction usually splits into three pieces:

1. Sensor open library: `libmmcamera_<sensor>.so`
2. EEPROM calibration helper: `libmmcamera_<sensor>_eeprom.so`
3. Chromatix/tuning payload selection described by `*_chromatix.xml`

The `libmmcamera_<sensor>.so` blobs are the best first target because they often still export `sensor_open_lib` and contain plain register tables.

## Immediate IMX386 observations

- `RE/libmmcamera_oxygen_imx386_sunny.so` exports `sensor_open_lib`
- It also exports PDAF helpers, so the blob is not just register arrays
- `proprietary/vendor/etc/camera/oxygen_imx386_sunny_chromatix.xml` gives mode names that help map raw register tables to preview, video, 4K, HDR, and HFR use cases
- `RE/imx386_regs.c` currently captures likely register sequences, but it is not yet a drop-in Qualcomm sensor library implementation

## Working method

1. Inventory the module in `camera_config.xml` and its matching chromatix XML.
2. Inspect exports:

```bash
readelf -Ws RE/libmmcamera_oxygen_imx386_sunny.so
```

3. Extract likely register tables:

```bash
python3 RE/extract_sensor_modes.py \
  RE/libmmcamera_oxygen_imx386_sunny.so \
  --array-prefix imx386_mode
```

3.5. Dump known inline metadata blocks for cross-sensor comparison:

```bash
python3 RE/dump_sensor_open_lib.py \
  RE/libmmcamera_oxygen_imx386_sunny.so \
  --name oxygen_imx386_sunny
```

3.6. For OV/S5K sensors, derive mode geometry from their reg tables:

```bash
python3 RE/extract_mode_geometry.py \
  RE/libmmcamera_oxygen_ov12a_sunny.so \
  --start-reg 0x3808 --terminator 0x0000
```

3.7. Locate output info arrays (use relaxed scan for OV12A):

```bash
python3 RE/dump_output_info.py \
  RE/libmmcamera_oxygen_ov12a_sunny.so \
  --name oxygen_ov12a_sunny --relaxed
```

For S5K5E8, the inline sensor name is `s5k5e8_qtec` (missing `h`). The default
output-info scan can miss the single populated entry; use a direct pattern
search if needed.

4. Disassemble `sensor_open_lib` and identify the top-level structures it returns.
5. Recover mode metadata around each register table:
   - output size
   - line length / frame length
   - CSI lane config
   - pixel format
   - fps
6. Compare against existing downstream Qualcomm sensor drivers from similar Xiaomi/msm8953 devices.
7. Rebuild the open library in source form, then replace the blob only after the ABI and structure layout match.

### Batch scan helper

To get a broad inventory of camera blobs quickly (exports, imports, deps), use:

```bash
RE/batch_camera_scan.sh proprietary RE/out quick camera
```

This also runs the existing sensor dump helpers on any blob that exports
`sensor_open_lib`. Use:

- `r2` for lightweight radare2 metadata.
- `deep` for `r2 -A` plus strings and function list (slow).
- `camera` scope for just camera-related blobs.
- `all` scope to scan all vendor `.so` blobs (very slow).

## Scope guidance

Good OSS candidates in this tree:

- Camera sensor open libs
- EEPROM helpers
- Simple hardware HAL wrappers such as IR or small fingerprint shims if kernel/userspace protocol is known
- Java framework glue where behavior can be stubbed or reimplemented

Poor early targets:

- Modem/radio stack
- GNSS core stack
- Adreno GPU userspace
- DSP/Hexagon payloads
- DRM/Widevine
- Large Qualcomm performance/data stacks unless you only need compatibility shims

## Notes on `extract_sensor_modes.py`

The script is heuristic by design:

- It scans for `<u16 reg, u16 val>` patterns
- It deduplicates identical payloads found at multiple offsets by default
- It defaults to the common Sony table shape that starts at `0x3030` and ends at `0x3004`

Use `--keep-duplicates` when you need the physical table-slot layout instead of just unique payloads.

Adjust `--start-reg`, `--terminator`, and `--min-len` when working on other sensors.

## IMX386 `sensor_open_lib` status

Current first-pass reverse engineering for `RE/libmmcamera_oxygen_imx386_sunny.so`:

- `sensor_open_lib` is at `0x1a44`
- It returns `0x6008`, which points into `.data`
- The bytes at `0x6008` start with the inline sensor name string `oxygen_imx386_sunny`
- The object is not just a string. The surrounding `.data` block contains packed scalar configuration fields
- `.data.rel.ro` at `0x5e24` contains a relocation-backed pointer table that is part of the returned library object

Important implication:

- Replacing the blob requires reconstructing both the inline data blocks and the pointer-backed callback / sub-structure table
- Extracting register arrays alone is not enough

Use `RE/imx386_sensor_open_lib.md` for the current address map.

Additional sensor notes:

- `RE/ov12a_sensor_open_lib.md`
- `RE/s5k5e8_sensor_open_lib.md`

Camera blob export summary:

- `RE/camera_blob_exports.md`
