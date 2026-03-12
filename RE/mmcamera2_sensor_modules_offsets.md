# mmcamera2_sensor_modules Sensor Lib Offset Notes

This file captures concrete field accesses observed in
`RE/libmmcamera2_sensor_modules.so` and in the per-sensor `sensor_open_lib()`
blobs used by oxygen. The goal is to keep offset evidence separated by pointer
type and avoid reusing the same raw offset across unrelated structures.

Unless noted otherwise, offsets below are relative to the pointer passed to the
specific function under discussion. Some earlier notes reused the same raw
offsets across different struct types; those cases are now called out
explicitly instead of being treated as one shared layout.

## Direct IMX386 sensor-lib anchor

`proprietary/vendor/lib/libmmcamera_oxygen_imx386_sunny.so` exports a trivial
`sensor_open_lib()` at `0x1a44` which returns a fixed pointer into `.data`:

- returned pointer lands immediately before the inline name block at
  approximately `0x6006..0x6008` depending on Thumb PC interpretation
- the inline string `"oxygen_imx386_sunny"` begins at `0x6008`

For field extraction below, the data dump is still anchored at the start of the
inline name block (`0x6008`), because that is the stable byte position we can
read directly from the blob.

Concrete IMX386 values from the returned object:

```
base+0x1d0 = 0x00000003
base+0x1d4 = 0x034e034c
base+0x1d8 = 0x03400342
base+0x1dc = 0x00000202
base+0x1e0 = 0x00000204
base+0x1e4 = 0x00000000
base+0x1e8 = 0x00000000
base+0x1ec = 0x0000000a
base+0x1f0 = 0x3f800000   // 1.0f
base+0x1f4 = 0x41800000   // 16.0f
base+0x1f8 = 0x41800000   // 16.0f
base+0x1fc = 0x00000000
base+0x200 = 0x00000000
base+0x204 = 0x00000000
base+0x208 = 0x00000000
base+0x20c = 0x00000000
base+0x220 = 0x00000000
base+0x224 = 0x00000000
base+0x228 = 0x00020002
base+0x22c = 0x00000002
```

Strong direct anchor from this dump:

- `base+0x1d4 = 0x034e034c`
  - `0x034c` = X output size register
  - `0x034e` = Y output size register
- `base+0x1d8 = 0x03400342`
  - `0x0342` = line length register
  - `0x0340` = frame length register
- `base+0x1dc = 0x0202`
  - coarse integration time register
- `base+0x1e0 = 0x0204`
  - gain register

This is the first direct proof from a sensor blob that the returned
`sensor_open_lib()` object contains a static register-address sub-struct around
`+0x1d4..+0x1e0`.

Cross-sensor comparison of the same returned-object slot:

- `oxygen_imx386_sunny`
  - `base+0x1d4 = 0x034e034c`
  - `base+0x1d8 = 0x03400342`
  - `base+0x1dc = 0x0202`
  - `base+0x1e0 = 0x0204`
- `oxygen_s5k5e8_qtech`
  - `base+0x1d4 = 0x034e034c`
  - `base+0x1d8 = 0x03400342`
  - `base+0x1dc = 0x0202`
  - `base+0x1e0 = 0x0204`
- `oxygen_ov12a_sunny`
  - `base+0x1d4 = 0x380a3808`
  - `base+0x1d8 = 0x380e380c`
  - `base+0x1dc = 0x3500`
  - `base+0x1e0 = 0x3508`

Interpretation:

- this slot is structurally stable across sensors
- its role is "output/timing/exposure/gain register-address block"
- the literal register values are sensor-family specific
  - Sony/Samsung-style here uses `0x034*` and `0x020*`
  - OmniVision-style here uses `0x380*` and `0x350*`

Cross-sensor comparison of nearby static limits:

- `base+0x1ec`
  - IMX386 = `0x0000000a`
  - OV12A = `0x00000008`
  - S5K5E8 = `0x00000008`
  - role still unresolved
- `base+0x1f0`
  - IMX386 = `1.0f`
  - OV12A = `1.0f`
  - S5K5E8 = `1.0f`
  - best current interpretation: static minimum gain
- `base+0x1f4`
  - IMX386 = `16.0f`
  - OV12A = `15.5f`
  - S5K5E8 = `16.0f`
  - best current interpretation: static maximum gain
- `base+0x230`
  - IMX386 = `1.25f`
  - OV12A = `1.242f`
  - S5K5E8 = `1.12f`
  - pixel size in microns

Correction on earlier `0x21c` / `0x220` claims:

- the direct IMX386 sensor-lib dump at the `0x6008`-anchored data block does
  not contain `3.81f` or `2.2f` at `+0x21c` / `+0x220`
- those earlier names came from a different pointer type in
  `libmmcamera2_sensor_modules.so`
- so `focal_length` / `f_number` should not currently be attached to the
  returned sensor-lib object at those offsets

The only nearby float field still directly supported by the sensor blob is:

- `base+0x230`
  - IMX386 = `1.25f`
  - best current interpretation: pixel size in microns

## Direct sensor-lib callback anchors

Important base split for the per-sensor blob:

- some helpers operate on the raw returned sensor object: `obj`
- others operate on an internal helper subobject: `sub = obj + 0x8`

Confirmed IMX386 correspondence:

- `sub+0x1d0 == obj+0x1d8`
- `sub+0x1d8 == obj+0x1e0`
- `sub+0x1e8 == obj+0x1f0`
- `sub+0x210 == obj+0x218`
- `sub+0x220 == obj+0x228`

This matters because helper-local reads of `+0x1f8`, `+0x210`, or `+0x220`
are not evidence for the same raw top-level offsets unless the base is proven.

The helper subobject also now has a clearer control surface:

- `sub+0x3c` / `sub+0x40` / `sub+0x44` are pointer-valued control slots
- selectors `-1` / `-2` / `0xd` address `sub+0x3c`
- selector `0xe` addresses `sub+0x40`
- selector `0xf` addresses `sub+0x44`
- selectors `0..0xc` address the callback/slot table at `sub+0x8 + idx*4`
- selector family `(sel & ~3) == 0xc0` uses the lazily snapshotted region
  rooted at `sub+0x1d8`, gated by byte `sub+0x4c`
- pair-selector family `0x100` uses backing region `sub+0x50`, gated by `sub+0x49`
- pair-selector family `0x110` uses backing region `sub+0xd8`, gated by `sub+0x4a`
- pair-selector family `0x70` uses backing region `sub+0x158`, gated by `sub+0x4b`
- `0x286c` seeds `[sub+0x0]` with the relocation-backed helper ops table at `0x5e2c`
- `0x29c0 -> ops+0x3c -> 0x2dfb`, and `0x2dfb` sets `sub+0x48 = 1`

Current safest family-level interpretation from IMX386:

- family `0x100` looks like small static prefix/header pairs
- family `0x110` looks like compact per-resolution metadata pairs
- family `0x70` looks like an indexed static-info/capability view whose high
  entries walk directly into the proven register-address block
- for family `0x100`, `sub+0x48` acts as an arm flag while `sub+0x49` remains
  the one-time init gate for the backing region
- on IMX386, the two `sub+0x48`-selected save stubs (`0x27fc` and `0x2804`)
  are identical `vstmia d0..d15` helpers, and their paired restore stubs
  (`0x3520` and `0x3528`) are likewise identical `vldmia d0..d15` helpers
- cross-sensor comparison shows the `0x100` family is vendor-invariant within a
  sensor family (IMX386/OV12A variants match their siblings), but it is not
  stable enough across sensor families to assign one semantic name per pair
  index yet
- the same is now true for `0x110`: it is vendor-invariant within a sensor
  family, still looks like compact per-resolution/mode metadata, but the slot
  meanings are not stable enough across sensor families to name globally yet
- the `0xc0` family is simpler: selectors `0xc0..0xc3` read one word each from
  a lazily snapshotted 4-word block at `sub+0x1d8..0x1e4`, with `sub+0x4c` as
  the one-time snapshot gate

Representative IMX386 pairs:

- family `0x100`
  - `0x100 -> (0x00000002, 0x00000000)`
  - `0x101 -> (0x00000000, 0x00000001)`
  - `0x102 -> (0x00000002, 0x00000001)`
  - `0x105 -> (0x016e3600, 0x00000001)`
  - `0x107 -> (0x00000002, 0x0000000b)`
- family `0x70`
  - `0x7c -> (0x00000001, 0x00000000)`
  - `0x7d -> (0x00000001, 0x00000001)`
  - `0x7e -> (0x00000003, 0x034e034c)`
  - `0x7f -> (0x03400342, 0x00000202)`

Stronger current naming:

- selector `0xd` / `sub+0x3c` is best treated as a mutable parse-cursor pointer
  inside the helper subobject
- selectors `0xe` / `0xf` form a paired unresolved pointer/state slot used near
  parser termination

### `IMX386 0x2c90` - `0x2cde`

```asm
0x2c94 ldrb.w r0, [r4, 0x210]
0x2cba ldr.w  r0, [r4, 0x1f8]
0x2cc6 ldrd   r2, r5, [r0, #12]
0x2cce ldr.w  r1, [r4, 0x1f8]
0x2cd2 adds   r2, r1, r0
```

Observed behavior:

- checks byte flag `sub+0x210`
- if not initialized, calls the helper at `0x2e30`
- if `sub+0x1f8 != 0`, performs a two-callback sequence and uses `sub+0x1f8`
  as an addend into the second call

Current safe interpretation:

- `sub+0x1f8` is definitely a live operational field in the helper subobject
- even though it numerically mirrors `obj+0x1f4` in IMX386, it should not be
  collapsed into the same semantic name yet

### `IMX386 0x2ce0` - `0x2cf6`

```asm
0x2ce2 add.w r0, r0, #0x1e8
0x2ce6 ldm.w r0!, {r2, r3, r4, r12, lr}
0x2cea stm.w r1!, {r2, r3, r4, r12, lr}
0x2cee ldm.w r0, {r2, r3, r4, r12, lr}
0x2cf2 stm.w r1, {r2, r3, r4, r12, lr}
```

Observed behavior:

- copies exactly `0x28` bytes from `obj+0x1e8` into caller storage

Current safe interpretation:

- `obj+0x1e8..obj+0x20f` is a compact context block with ABI relevance inside
  the sensor blob itself
- this block should stay mostly opaque until more direct sensor-blob consumers
  are mapped

## Key accesses (by address)

### `0x00013078` - `0x000130ca`

```
0x00013078 vldr s0, [r6, 0x21c]
0x00013080 ldr.w r0, [r6, 0x228]
0x00013088 ldr.w r1, [r6, 0x22c]
0x00013092 ldr.w r1, [r6, 0x220]
0x000130a2 vldr s2, [r6, 0x230]
0x000130aa vdiv.f32 s2, s16, s2      ; s16 = 1.0
0x000130b6 vldr s2, [r6, 0x220]
```

Observed behavior:

- `0x230` is treated as a float. The code computes `1.0 / *(float *)(base+0x230)`.
- `0x228` and `0x22c` are copied as raw 32-bit words into a larger output struct.
- `0x21c` and `0x220` are treated as floats and fed into downstream math.

This is consistent with `0x230` being pixel size in microns for the pointer
type used at this call site. It should not be used as evidence that adjacent
offsets around `0x21c` / `0x220` belong to the direct returned sensor-lib
object without first proving the pointer provenance.

### `0x0002da44` - `0x0002da72`

```
0x0002da44 ldr.w r2, [r0, 0x228]
0x0002da4c ldr.w r0, [r0, 0x22c]
0x0002da4a uxth r3, r2
0x0002da54 lsr.w r3, r2, 0x10
0x0002da64 subs r2, r2, r0
0x0002da72 strh.w r0, [sl, 0x2c]
```

Observed behavior:

- `0x228` is treated as two packed `u16` values (low/high halfwords).
- `0x22c` is a `u32` used as a comparison/subtraction reference.

Instruction-faithful pseudocode for this block:

```c
uint32_t w228 = *(uint32_t *)(base + 0x228);
uint32_t ref  = *(uint32_t *)(base + 0x22c);
uint32_t lo = w228 & 0xffff;
uint32_t hi = (w228 >> 16) & 0xffff;

uint32_t cand = (lo < hi) ? hi : lo;
if (hi < ref) {
  cand -= ref;  // 32-bit wrap
}

uint32_t out = (ref > 0) ? (ref - 1) : ref;
if ((int32_t)cand >= 0) {
  out = cand;
}

*(uint16_t *)(dst + 0x2c) = (uint16_t)out;
```

Empirical check with `RE/tools/sensor_0x228_eval.py`:

- `ov12a_sunny`: `w228=0x00020002`, `ref=2`, result `u16=2`
- `s5k5e8_qtech` (inline name `s5k5e8_qtec`): same, result `u16=2`
- `imx386_sunny`: same, result `u16=2`

For oxygen camera blobs, this path currently collapses to a constant output
`2` because both packed halfwords and reference are all `2`.

Control-flow context (same parent dispatcher):

- At `0x2da34`, code checks a request/control byte at `req+0x94`.
- If `req+0x94 == 1`, it executes the full `0x2da44` arithmetic above.
- Else it goes through `0x31478`:
  - if `req+0x125 == 1`, force `0` into `dst+0x2c`
  - otherwise copy `*(u16 *)(base+0x228)` into `dst+0x2c`
- Both paths then converge at `0x32296`.

So `dst+0x2c` here is a mode/control-dependent scalar, not always a direct crop
edge derived from `output_info`.

String context around this block also resolves to
`sensor_get_resolution_info` (`.rodata` near `0x65cae`), matching nearby debug
format strings such as `BRAK frame initial skip: %d`.

### `0x0003228e` - `0x000322cc`

```
0x0003228e ldrh.w r0, [r0, 0x228]
0x00032292 strh.w r0, [sl, 0x2c]
0x000322be ldr.w r0, [r0, 0x1f4]
0x000322c2 str.w r0, [sl, 0x30]
0x000322c8 ldr.w r0, [r0, 0x20c]
0x000322cc str.w r0, [sl, 0x34]
```

Observed behavior:

- `0x228` low halfword is exported into an output struct as `u16`.
- `0x1f4` (word 3 in the `0x1e8` block) is copied as a 32-bit float value.
- `0x20c` (word 9 in the `0x1e8` block) is copied as a raw 32-bit value.

This strongly suggests:

- `0x1f4` = max gain (float, 15.5 or 16.0).
- `0x20c` is a timing/count-style scalar, but the exact field name is not
  stable enough yet.

### `0x00035cb4` - `0x00035ce2`

```
0x00035cb4 ldrh.w r6, [r0, 0x1e8]
0x00035cdc ldrh.w r3, [r0, 0x1e8]
0x00035ce0 adds r3, 1
```

Observed behavior:

- `0x1e8` is loaded as a `u16`, written into an 8-byte I2C-style entry, then a
  second entry is emitted with `0x1e8 + 1` as the data value.
- The surrounding code repeatedly builds `{u16 addr, u16 data, u32 delay}`-like
  records for an exposure-setting array.
- This means `0x1e8` is participating in exposure-programming register
  sequences, not behaving like a fixed active-array dimension.

Additional confirmation from `0x0003601a..0x0003609a`:

- `0x1e8` and `0x1ea` are both loaded as `u16` sources.
- Each is copied into a reg-array entry as the first halfword, with the paired
  second halfword coming from call arguments / derived values.
- Both entries store `0` to the 32-bit tail word, matching the existing
  `reg_addr / reg_data / delay` record pattern in this function.

Current implication:

- The earlier "paired geometry" reading for `0x1e8/0x1ea` is no longer safe.
- At minimum, these two fields are used as small `u16` values in the
  exposure-programming path.
- Their exact semantic names remain unresolved.

### `0x0002df6a` - `0x0002dff2` (`sensor_get_output_info`)

Dispatcher mapping note:

- `sym.module_sensor_event_control_set_parm` contains a `tbh` at `0xff0a`.
- For this `tbh`, target base behaves as aligned PC (`0xff0c`), so index 0 uses
  halfword `0xf011` at `0xff0c` and jumps to `0x2df2e`.
- `0x2df2e` immediately falls through into `0x2df30`.

This means command index `0` of the `0xff0a` table (i.e. command ID `1` after
the preceding `r1 = cmd - 1` normalization) enters the
`sensor_get_output_info` handler.

String context for this handler is consistent with `sensor_get_output_info`
(`.rodata` near `0x660ac`), with nearby format strings:

- `requested dim %d %d stream mask %x`
- `pick res %d dim %dX%d op clk %d`

Current partial map for the `0xff0a` dispatch table:

- command `1` -> `0x2df2e` -> `sensor_get_output_info`
- command `2` -> `0x00ff30` -> set HAL version path (`"set HAL version failed"`)
- command `3` -> `0x01092e` -> set control mode (`+0x6dc8`)
- command `4` -> `0x01093c` -> set AE mode (`+0x6dcc`)
- commands `5`..`10` -> `0x010b1a` -> generic `module_sensor_hal_set_parm`
- command `11` -> `0x010950` -> EZTune chromatix event path
- commands `12`..`13` -> `0x010b1a` -> generic `module_sensor_hal_set_parm`
- command `14` -> `0x0109a2` -> special-mode selector feeding `sensor_util_set_special_mode`; adjacent strings tie this cluster to frame-skip / frame-duration handling
- command `15` -> `0x0109e6` -> manual AEC / special-mode path; debug strings include `sof %d g %f lc %d` and it also reaches `sensor_util_set_special_mode`
- command `16` -> `0x010a1c` -> LED mode setter (`module_sensor_set_parm_led_mode`)
- commands `17`..`18` -> `0x010b1a` -> generic `module_sensor_hal_set_parm`
- command `19` -> `0x010aaa` -> manual AEC update path (`e_manual_aec_update`)

This table is a mixed control dispatcher, not a pure `sensor_get_*` table.
Only command `1` is currently tied cleanly to `sensor_get_output_info`.

Caller-side context:

- `sym.module_sensor_capture_control` calls helper `fcn.0001578c`, which in
  turn issues calls into `module_sensor_event_control_set_parm`.
- One path performs a direct one-shot call at `0x15ecc`.
- Another path iterates a fixed-size command list and calls
  `module_sensor_event_control_set_parm` at `0x1600c`.
- In that loop:
  - entry count comes from `[fp + 0x8]`
  - entry base comes from `[fp + 0xc]`
  - stride is `8` bytes per entry
  - each entry pointer is passed as the dispatcher payload argument

This indicates the command IDs above are part of a compact capture-control
command ABI, not just isolated ad hoc setters.

Current working shape for each 8-byte capture-control entry:

```c
struct capture_ctrl_cmd_entry {
  uint32_t tag;      // outer capture-control tag, consumed by caller-side logic
  void *data;        // payload pointer consumed by dispatcher cases
};
```

Evidence:

- The batch path passes `entry_ptr` itself as the dispatcher payload argument at
  `0x16008..0x1600c`.
- Dispatcher cases repeatedly read `*(void **)(entry_ptr + 4)`, e.g.:
  - `0x109a4`: `ldr r5, [r7, 4]`
  - `0x10a2e`: `ldr r0, [r7, 4]`
  - `0x10aba`: `ldr r1, [r7, 4]`
- The caller-side helper also reads `*(u32 *)entry_ptr` after dispatch
  (`0x15f14`), showing the first word belongs to outer capture-control logic,
  not just the dispatcher internals.

So the 8-byte record likely carries an outer event/tag in word 0 and a typed
payload pointer in word 1. The `0xff0a` command ID is supplied separately by the
outer call context, not stored inside this 8-byte record.

Direct-call selector path (`fcn.0001578c`, `0x15f14..`):

- A separate direct-call path branches on `*(u32 *)payload` loaded from
  `[r5 + 0x14]` at `0x15f66..0x15f6a`.
- This selector handling is adjacent to, but not identical with, the 8-byte
  batch-record path above.

Selectors with concrete behavior:

- `0x2e` -> `0x17824`
  - reads a payload pointer from `[selector_payload + 4]`
  - zeros a temp block at `sp+0x58`, then calls helper `fcn.0000b7ec`
  - stages a `SENSOR_SET_FPS` control packet on stack at `sp+0x240`
  - invokes a module callback with opcode `0x0c` using that staged packet
  - then invokes another callback with opcode `0x68`, copies `0x1b2` bytes from
    internal storage at `base+0x7a68` into the caller-provided buffer, and posts
    a downstream event with size `0x84` from that same internal block
  - later debug strings in this path mention crop transforms
    (`crop (%d,%d,%d,%d) ==> ...`)
  - current best interpretation: selector `0x2e` is an FPS-triggered stream /
    crop reconfiguration path, not just a scalar FPS setter
  - current best interpretation of `base+0x7a68` in this path:
    - opcode `0x68` fills the block at `base+0x7a68`
    - `0x1b2` bytes are copied from that block to the caller buffer
    - a `0x84`-byte prefix of the same block is posted downstream
    - nearby strings are:
      - `config: dim=%dx%d, mask=0x%x stream type: %d`
      - `CROP_INFO_SENSOR: str_id %d str_type %d crop (...) ==> (...)`
      - `CROP_INFO_CAMIF: str_id %d str_type %d crop (...) ==> (...)`
    - so `base+0x7a68` is best described as a cached stream/crop configuration
      blob, with a downstream-visible header of at least `0x84` bytes and local
      state extending to at least `0x1b2` bytes
    - a conservative header draft for that downstream-visible prefix now lives
      in `RE/camera_runtime_structs.h` as
      `struct sensor_cached_stream_crop_blob_v0`
    - the session-data export copy path for this cached blob also now has a
      conservative draft in `RE/camera_runtime_structs.h` as
      `struct sensor_session_data_export_v0`
  - downstream crop/config consumer:
    - `fcn.000592e0` sits next to the crop debug strings and is referenced from
      the new-resolution path
    - its core write sequence at `0x593f4..0x59412` updates:
      - `base+0x208`
      - `base+0x1e8`
      - `base+0x1ec`
      - `base+0x1f0`
      - `base+0x1f4`
      - `base+0x1fc`
    - this ties the `0x2e` stream/crop reconfiguration flow back into the
      existing `0x1e8` inline block
    - implication: at least part of `0x1e8..0x208` is runtime-refreshed by crop
      / config processing, so that region should not be modeled as fully static
      data in an early C clone
    - current best interpretation of the two easiest outputs:
      - `base+0x208`
        - written by `fcn.000592e0`
        - also written during offload/probe init paths
        - read as both `u32` and `u16` by `port_sensor_create`, `pdaf_set_window_update`,
          and `sensor_fc_process`
        - best described for now as a runtime resolution/config selector index
      - `base+0x1fc`
        - written by `fcn.000592e0` to values `1` or `2`
        - read by `sensor_fc_process` and `flash_get_frame_skip_timing_params`
        - best described for now as a small runtime mode/scale flag, not a
          large free-form integer
    - value-flow shape for `base+0x1e8..0x1f4`:
      - `fcn.000592e0` writes the four words in one bundle with
        `stm.w r0, {r1, r3, sb, lr}` at `0x59400`
      - the first two words are assembled from paired source values before the
        store, so they are not independent constants
      - downstream code treats at least `base+0x1e8` / `base+0x1ea` as paired
        halfword values, but not safely as geometry:
        - `sensor_cmn_lib_fill_exposure` reads `ldrh [base+0x1e8]` and
          `ldrh [base+0x1ea]`
        - it emits those values into exposure-setting reg-array records, with
          adjacent entries using `value` and `value + 1`
      - this currently suggests:
        - `0x1e8` / `0x1ea` are two adjacent `u16` sources used by the
          exposure-programming path
        - `0x1ec` / `0x1f0` are still companion words from the same refreshed
          bundle, but their exact role is unresolved
        - `0x1f4` remains part of the same refreshed bundle in this path, even
          though other paths also treat it as a standalone scalar
    - current safe interpretation:
      - `0x1e8..0x1f4` is a runtime-refreshed crop/config tuple
      - exact field names inside that tuple are not yet stable enough to freeze
        into a C ABI
      - `0x1ec` and `0x1f0` currently look more internal/local than `0x208`,
        `0x1fc`, or the paired halfwords at `0x1e8/0x1ea`, because they do not
        show the same broad public-ABI style readback pattern
      - this older interpretation is now superseded by the stricter
        sensor-blob split above
      - the current conservative C draft for the direct sensor-blob slice now
        lives in `RE/camera_runtime_structs.h` as
        `struct sensor_lib_context_block_v0`
    - stronger evidence for the exported session slice:
      - `module_sensor_offload_init_config` writes:
        - `base+0x204 <- req+0x9c`
        - `base+0x208 <- normalized(req+0xa0)`, where the observed mapping is:
          - `0 -> 0`
          - `1 -> 1`
          - `0x100 -> 2`
          - anything else triggers `invalid position = %d`
        - `base+0x20c <- req+0x98` on the `req+0x94 == 1` path
      - `pdaf_set_window_update` copies:
        - `base+0x208 -> +0x220`
        - `base+0x20c -> +0x224`
        - both are then used as nested loop bounds while filling per-window
          entries at `+0x238/+0x23a/+0x23c/+0x23e`
      - re-audit of the PDAF path:
        - the earlier shorthand `base+0x208/0x20c` was too aggressive
        - caller `pdaf_calc_defocus` invokes `pdaf_set_window_update` with:
          - `r0 = pdaf control object`
          - `r1 = r10 + 0xa79c`
        - so the PDAF code reads `0x208/0x20c` from an auxiliary PDAF/config
          buffer at `+0xa79c`, not directly from the top-level
          `sensor_open_lib()` object
        - this removes the apparent conflict with the probe-path/kernel ABI
          naming for the top-level exported fields
      - `module_sensor_get_session_data` exports:
        - `base+0x204 -> dst+0x000`
        - `base+0x208 -> dst+0x00c`
        - `base+0x20c -> dst+0x010`
      - current best interpretation from those three paths:
        - `0x204` is source `modes_supported`
        - `0x208` is source `position`, normalized internally as:
          - `0 -> BACK_CAMERA_B`
          - `1 -> FRONT_CAMERA_B`
          - `2 -> AUX_CAMERA_B`
        - `0x20c` is source `sensor_mount_angle`
      - `translate_sensor_slave_info` strengthens the above:
        - call site is now pinned in `sensor_init_probe` at `0x27718`
        - the function is not a simple `dst/src` copy; it consumes multiple
          inputs:
          - `r0` = destination slave-info export buffer
          - `r1` = sensor-slave/power-sequence style source (`+0x20/+0x24/+0x28`,
            `+0xf8`, `+0x1bc`, `+0x1c0`, power-seq arrays)
          - `r2` = higher-level source carrying strings and the exported
            `0x204/0x208/0x20c` fields
          - `r3` = destination buffer for translated power settings
        - this is important for the eventual open implementation because
          `0x204/0x208/0x20c` belong to the higher-level exported source, not to
          the raw sensor-slave/power-seq block
        - the post-translate probe path is now tied to the kernel ABI:
          - `sensor_init_probe` passes the translated buffer through
            `CFG_SINIT_PROBE`
          - the kernel handler copies compat userspace data into
            `struct msm_camera_sensor_slave_info`
          - kernel consumers then read:
            - `slave_info->camera_id`
            - `slave_info->sensor_init_params.position`
            - `slave_info->sensor_init_params.sensor_mount_angle`
            - `slave_info->bypass_video_node_creation`
        - this gives a solid name match for the translated destination fields:
          - `dst+0x60c` = `sensor_init_params.modes_supported`
          - `dst+0x610` = `sensor_init_params.position`
          - `dst+0x614` = `sensor_init_params.sensor_mount_angle`
          - `dst+0x618` = `bypass_video_node_creation`
        - `base+0x208` is translated back into external values:
          - internal `0 -> BACK_CAMERA_B`
          - internal `1 -> FRONT_CAMERA_B`
          - internal `2 -> AUX_CAMERA_B (0x100)`
          - any other value logs `invalid position = %d`
        - the translated value is stored at `dst+0x610`
        - `base+0x204` is separately translated into:
          - `-1 -> CAMERA_MODE_INVALID (4)`
          - `1 -> CAMERA_MODE_2D_B`
          - `2 -> CAMERA_MODE_3D_B`
          - other values log an invalid-value error before the function returns
        - together with local `camera_config.xml` fields
          (`ModesSupported`, `Position`, `MountAngle`), this is now strong
          enough to map the original higher-level source fields as:
          - `0x204` -> source `modes_supported`
          - `0x208` -> source `position` (normalized internally as `0/1/2`)
          - `0x20c` -> source `sensor_mount_angle`
        - `sensor_xml_util_get_camera_probe_config` now pins the adjacent field
          directly:
          - initializes `base+0x200 = 0xff`
          - binds XML node `"CameraId"` to that byte field
          - logs `camera_id = %d`
          - validates the parsed value as `< 4`
          - so `0x200` is source `camera_id`
        - a matching conservative source-tail C draft now lives in
          `RE/camera_runtime_structs.h` as
          `struct sensor_probe_source_tail_v0`
- `0x54` -> `0x16e2e`
  - leads into a path that updates session-local state, then calls
    `module_sensor_capture_control`, `sensor_fc_post_static_meta`, and
    `sensor_fc_process`
  - this behaves like a higher-level session/control transition rather than a
    simple scalar setter
- `0x70` and `0xc7` -> `0x16ce2`
  - both collapse into `sensor_util_post_event_on_src_port`
  - these look like passthrough control/event types

Selectors narrowed but not yet confidently named:

- range-filtered values in the `0x1b..0x3a` window also collapse into
  `0x16ce2` via the bitmask test at `0x15f1c..0x15f34`
- `0x0a` also collapses into `0x16ce2`
- `0x51` reaches `0x16e2e` debug/error handling
- values above `0x53` branch through `0x16e22`, where `0x54` is special-cased
  and `0xbd` is also handled separately

Additional naming evidence from `.rodata`:

- `0x01023a..0x01027c` logs `control mode %d ae mode %d` using internal
  offsets `+0x6dc8` and `+0x6dcc`
- `0x010950` sits next to `_eztune_chromatix_event`
- `0x0109a2` sits next to `frame_skip` and repo strings for
  `sensor_util_set_frame_skip_to_isp` / `module_sensor_handle_isp_frame_skip`;
  its downstream path also reaches `sensor_util_set_special_mode`
- `0x0109e6` sits next to `sof %d g %f lc %d` and repo strings for
  `port_sensor_handle_manual_aec_update`; downstream code also references
  `manual_data` and reaches `sensor_util_set_special_mode`
- `0x010aaa` sits next to `e_manual_aec_update`
- `0x011148` is a direct call to `sensor_util_set_special_mode`
- `0x0114be` is `sensor_fc_store`; nearby strings mention
  `CAM_INTF_META_SENSOR_FRAME_DURATION`, `apply frame %d frame time %lld`, and
  `set frame duration failed`

This path reads output-info fields from fixed offsets relative to the
`sensor_open_lib()` base pointer:

```
+0x73fc0 + idx*0x40  (u16) x_output
+0x73fc2 + idx*0x40  (u16) y_output
+0x73fc4 + idx*0x40  (u16) line_length_pclk
+0x73fc6 + idx*0x40  (u16) frame_length_lines
+0x73fc8 + idx*0x40  (u32) vt_pixel_clk
+0x73fcc + idx*0x40  (u32) op_pixel_clk
```

And a companion table:

```
+0x75188 + idx*0x08  (u16) a0
+0x7518a + idx*0x08  (u16) a1
+0x7518c + idx*0x08  (u16) a2
+0x7518e + idx*0x08  (u16) a3
```

Observed arithmetic:

```
crop_end_x = x_output - 1 - a3
crop_end_y = y_output - 1 - a1
```

For current oxygen blobs (`imx386`, `ov12a`, `s5k5e8`), `a0..a3` are all zero
for populated modes, so this path reduces to `(x-1, y-1)`.

Implication:

- The `line` / `frame` values in `output_info` are consumed as raw `u16` timing
  fields directly from this table (no unit normalization in this path).
- This explains why OV12A can report `line < width` while remaining accepted by
  userspace.

Field mapping in this path (same destination struct layout used here):

- `dst+0x28 = a2`
- `dst+0x2c = x_output - 1 - a3`
- `dst+0x30 = a0`
- `dst+0x34 = y_output - 1 - a1`

This is consistent with a crop/window rectangle representation in this specific
`sensor_get_output_info` path.

## Working interpretation (current)

These are not final field names, but they are supported by the access patterns:

- `0x1e8` / `0x1ea`: paired `u16` fields used by exposure-programming paths;
  exact semantics unresolved.
- `0x1f4` (word 3): max gain (float).
- `0x204`: source/exported `modes_supported`.
- `0x208`: source/exported `position`, normalized internally as `0/1/2` and
  translated back to external values `0/1/0x100` by
  `translate_sensor_slave_info`.
- `0x20c`: source/exported `sensor_mount_angle`.
- `0x200`: source `camera_id`, parsed from `CameraId` by
  `sensor_xml_util_get_camera_probe_config`.
- `0x228` (word 0): packed `u16` pair used in crop/constraint math.
- `0x22c` (word 1): `u32` parameter used alongside the packed pair.
- `0x230` (word 2): pixel size in microns (float).

Next step is to tie these offsets to a known Qualcomm `sensor_lib_t` header or
to follow the consuming functions further to identify exact field names.
