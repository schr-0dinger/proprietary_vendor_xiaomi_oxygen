# IMX386 `sensor_open_lib` Notes

This file captures the current reverse-engineering state of `RE/libmmcamera_oxygen_imx386_sunny.so`.

## Confirmed facts

- Exported function: `sensor_open_lib`
- Address: `0x1a44`
- Implementation:

```asm
0x1a44  ldr r0, [0x1a4c]
0x1a46  add r0, pc
0x1a48  bx  lr
```

- Literal value at `0x1a4c`: `0x45be`
- Resolved return value: `0x6008`

So `sensor_open_lib()` returns a pointer into `.data`, not a heap object and not code.

## Returned object layout

The returned pointer lands in the middle of a larger data block:

- `0x6000`: relocated word with raw file value `0x6000`
- `0x6008`: start of string `oxygen_imx386_sunny`
- `0x6028+`: inline scalar fields

This strongly suggests a Qualcomm-style static sensor library descriptor where:

- a leading header or self-reference exists before the name
- the name is inlined directly in the structure
- later fields are scalar sensor configuration values
- pointer members are stored in `.data.rel.ro`

## File vs memory offsets

`.data` is mapped at `0x6000` with file offset `0x5000`. Any in-memory
address in the `0x6xxx` range maps to file offset `0x5xxx`.

The previously referenced blocks land at:

- `0x61c8` -> file offset `0x51c8`
- `0x61f0` -> file offset `0x51f0`
- `0x6230` -> file offset `0x5230`
- `0x62b8` -> file offset `0x52b8`

## Relocation-backed pointer table

`readelf -r` shows `R_ARM_RELATIVE` relocations in `.data.rel.ro` from `0x5e2c` through `0x5e68`.

Raw values in that table:

- `0x5e2c -> 0x26e1`
- `0x5e30 -> 0x26e1`
- `0x5e34 -> 0x29c7`
- `0x5e38 -> 0x29e9`
- `0x5e3c -> 0x2a75`
- `0x5e40 -> 0x2b01`
- `0x5e44 -> 0x2b21`
- `0x5e48 -> 0x2bc5`
- `0x5e4c -> 0x2c91`
- `0x5e50 -> 0x2ce1`
- `0x5e54 -> 0x2cf9`
- `0x5e58 -> 0x2d0f`
- `0x5e5c -> 0x2d15`
- `0x5e60 -> 0x2d85`
- `0x5e64 -> 0x2df5`
- `0x5e68 -> 0x2dfb`

Interpretation:

- these are function or nested-structure pointers belonging to the returned sensor library object
- `0x26e1` appears twice, which usually means the same callback is used for two table slots
- several entries are Thumb code pointers because the low bit is set

## First interesting code targets

### `0x26e1`

- Thumb function starting at `0x26e0/0x26e1`
- Large function with stack frame and internal calls
- Almost certainly a real library callback, not inert data
- It allocates a large temporary stack object and calls `0x25f4`
- `0x25f4` drives an interface made of function pointers at object offsets `0x8`, `0xc`, `0x10`, `0x14`, `0x18`, `0x1c`, `0x20`, `0x24`, and `0x28`
- This looks like a parser / enumerator wrapper around a smaller internal vtable-like object

### `0x29c7`, `0x29e9`, `0x2a75`, `0x2b01`, `0x2b21`, `0x2bc5`, `0x2c91`

- All decode as Thumb code
- These are likely the major sensor operations exposed through the library descriptor

### `0x2ce1`, `0x2cf9`, `0x2d0f`, `0x2d15`, `0x2d85`, `0x2df5`, `0x2dfb`

- Also decode as valid code or code-adjacent entries
- These are likely utility callbacks, PDAF hooks, or small mode-specific helpers

## Provisional callback role map

These names are not final, but they are grounded in the observed control flow.

### `0x29c7`

- Tiny validator
- Calls a function pointer from `[obj->ops + 0x3c]`
- Then checks whether a selector value belongs to a narrow valid range
- Likely `is_valid_slot()` or `is_valid_key()`

### `0x29e9`

- Getter for pointer-valued slots
- Accepts an object pointer and selector
- Handles special selectors `-1`, `-2`, `0xd`, `0xe`, `0xf`
- Reads from inline offsets `+0x3c`, `+0x40`, `+0x44`, or generic table storage starting at `+0x8`
- Selector family masked by `0xc0` triggers lazy initialization using the block at `+0x1d8`
- Best current name: `get_slot_ptr()`

### `0x2a75`

- Setter counterpart to `0x29e9`
- Writes to inline offsets `+0x3c`, `+0x40`, `+0x44` or generic table storage at `+0x8`
- Selector family masked by `0xc0` triggers lazy initialization of the `+0x1d8` block
- Best current name: `set_slot_ptr()`

### `0x2b01`

- Tiny validator for another selector family
- Masks with `0xf` and `0x1f`
- Accepts values aligned to `0x70`, `0x100`, and `0x110`
- Best current name: `is_valid_pair_slot()`

### `0x2b21`

- Getter for doubleword or pair entries
- Returns two words with `ldrd`
- Uses three lazily initialized backing regions:
  - `+0x50`
  - `+0xd8`
  - `+0x158`
- Controlled by flags at bytes `+0x48`, `+0x49`, `+0x4a`, `+0x4b`
- Best current name: `get_pair_slot()`

### `0x2bc5`

- Setter counterpart to `0x2b21`
- Writes two values into the same three backing regions used by `0x2b21`
- Uses negative constant offsets that land inside the `+0x50`, `+0xd8`, and `+0x158` blocks
- Best current name: `set_pair_slot()`

### `0x2c91`

- State-transition helper
- Checks byte flag at `+0x210`
- If not initialized, calls `0x2e30`
- Then invokes callbacks through `[obj->ops + 0x34]` and `[obj->ops + 0xc]`
- Reads an inline value from `+0x1f8`
- Best current name: `ensure_ready()` or `activate_context()`

### `0x2ce1`

- Plain block copy helper
- Copies 40 bytes from `obj + 0x1e8` into caller storage
- Best current name: `copy_context_block()`

### `0x2cf9`

- Wrapper that passes `obj + 0x8` into `0x2e84`
- Then tail-calls another routine at `0x3590`
- Best current name: `finalize_or_publish()`

### `0x2d0f`

- Single-byte getter
- Returns `*(u8 *)(obj + 0x211)`
- Best current name: `get_state_flag()`

### `0x2d15`

- Builds a formatted string with `dladdr()` and `__snprintf_chk()`
- Stores an address delta through an output pointer
- Likely reports the current shared-object path or symbol-relative offset
- Best current name: `format_self_path_or_offset()`

### `0x2d85`

- Initialization helper for the region starting at `obj + 0x8`
- Reads a callback from `[obj->ops + 0xc]`
- Computes a normalized address value and calls `0x2edc`
- Sets byte flag `+0x210` on one path
- Best current name: `init_context_from_self()`

### `0x2df5`

- Very small setter
- Writes `1` to byte `+0x48`
- Best current name: `mark_primary_block_present()`

### `0x2dfb`

- Falls through into `0x2d85`
- Likely an alternate entry point that presets state before common initialization
- Best current name: `init_context_from_self_alt()`

## Internal helper object at `+0x8`

The block initialized by `0x286c` starts at `obj + 0x8` and is a distinct sub-object.

Observed behavior:

- `0x286c` writes two pointers at the start of this sub-object
- It clears bytes up to roughly `+0x1c2`
- It calls `0x2d85` on that sub-object immediately after initialization
- `0x25f4` and the `0x28xx-0x29xx` helpers then operate on this sub-object through function pointers

This is likely an internal context or enumerator used by the higher-level sensor library callbacks.

## Parent object structure clues around `0x6008`

The data between the inline name and the embedded metadata blocks is not random padding. It already looks like mode-count and per-resolution metadata.

### Embedded metadata at `0x61c8`, `0x61f0`, `0x6230`, `0x62b8`

These are raw little-endian words in the returned object (file offsets above).

`0x61c8` (10 words):

- `0x00000001 0x00000000 0x00000001 0x00000001`
- `0x00000003 0x034e034c 0x03400342 0x00000202`
- `0x00000204 0x00000000`

`0x61f0` (16 words):

- `0x00000000 0x0000000a 0x3f800000 0x41800000`
- `0x41800000 0x00000000 0x00000000 0x00000000`
- `0x00000000 0x0000fff5 0x00000000 0x00000000`
- `0x00000000 0x00000000 0x00000000 0x00000000`

`0x6230` (34 words):

- `0x00020002 0x00000002 0x3fa00000 0x00000002`
- `0x40b8f5c3 0x00000fc0 0x00000bc8 0x000c000c`
- `0x00100010 0x004003ff 0x00400040 0x00000040`
- `0x00000003 0x00022b00 0x00000000 0x00000000`
- `0x00000000 0x00023601 0x00000000 0x00000000`
- `0x00000000 0x00023502 0x00000000 0x00000000`
- `0x00000000 0x00000000 0x00000000 0x00000000`
- `0x00000000 0x00000000`

`0x62b8` (6 words):

- `0x00000003 0x00000003 0x00000000 0x00000000 0x00000001 0x00021203`

## Cross-sensor comparison

Dumping the same blocks from other oxygen sensors shows patterns that help
interpret a few fields. The helper below prints the same blocks:

```
python3 RE/dump_sensor_open_lib.py <blob> --name <sensor_name>
```

### `0x61c8` pack looks like output/exp register addresses

- `imx386`: `0x034e034c 0x03400342 0x00000202 0x00000204`
- `ov12a`: `0x380a3808 0x380e380c 0x00003500 0x00003508`
- `s5k5e8`: `0x034e034c 0x03400342 0x00000202 0x00000204`

The middle two words are classic Sony/Samsung output regs, while OV uses
`0x3808/0x380a/0x380c/0x380e`. This strongly suggests:

- word 5 packs `x_output` and `y_output` register addresses
- word 6 packs `line_length_pclk` and `frame_length_lines` register addresses
- word 7/8 are coarse integration and global gain register addresses

### `0x6230` contains active array dimensions

Across sensors, words 5/6 line up with known native sizes:

- `imx386`: `0x0fc0` x `0x0bc8` = `4032 x 3016`
- `ov12a`: `0x1240` x `0x0db0` = `4672 x 3504`
- `s5k5e8`: `0x0a20` x `0x0798` = `2592 x 1944`

The following two packed words (imx386 `0x000c000c`, `0x00100010`) vary between
`0x0008` and `0x0010` depending on sensor, and look like crop/guard margins.

### `0x62b8` is IMX386-only (PDAF?)

`ov12a` and `s5k5e8` are all-zero at `0x62b8`, while IMX386 has:

`0x00000003 0x00000003 0x00000000 0x00000000 0x00000001 0x00021203`

This likely encodes PDAF or another IMX-specific capability flag block.

### Small per-resolution block at `0x6100`

Relocation-applied words:

- `0x00000006`
- `0x00000000`
- `0x00000000`
- `0x00000000`
- `0x00000001`
- `0x00000001`
- `0x00000000`
- `0x00000000`
- `0x00000001`
- `0x00000002`
- `0x00000001`
- `0x00000000`
- `0x00000000`
- `0x00000002`
- `0x00000000`
- `0x00000000`
- `0x00000001`
- `0x00000002`
- `0x00000002`
- `0x00000000`
- `0x00000001`
- `0x00000000`

Current interpretation:

- the leading `0x6` matches the six `sensor_resolution_index` entries in `proprietary/vendor/etc/camera/oxygen_imx386_sunny_chromatix.xml`
- this is likely a compact per-resolution metadata array embedded in the returned descriptor
- the values are too small and too regular to be register addresses or geometry; they look more like frame-skip, mode-class, or stream-selection metadata

More specific shape:

- after the leading `0x6`, the remaining nonzero part divides cleanly into seven triplets
- the seven triplets are:
  - `[0, 0, 0]`
  - `[1, 1, 0]`
  - `[0, 1, 2]`
  - `[1, 0, 0]`
  - `[2, 0, 0]`
  - `[1, 2, 2]`
  - `[0, 1, 0]`

This does not fit "six plain entries" very well, but it does fit the chromatix configuration model:

- one common entry
- six `sensor_resolution_index` entries

So the best current interpretation is:

- `0x6100` starts with a resolution count of `6`
- the triplets after that are likely one common/default tuple plus six per-resolution tuples
- the tuple members are still unnamed, but they are probably small selectors such as frame-skip counts, mode classes, or tuning-group indices

One additional constraint from the current grouping attempt:

- the third tuple element does not separate photo modes from video modes
- photo-side values for that field are `[0, 2, 0]`
- video-side values for that field are `[0, 2, 0]`

So the third element is unlikely to be the coarse "photo vs video family" selector.

The first and second tuple elements remain the better candidates for family or class selection.

This block should be treated as part of the same top-level descriptor that also embeds the larger metadata blocks at `+0x1c0`, `+0x228`, and `+0x2b0`.

### Resolution count vs unique register scripts

The chromatix XML defines six resolution indices:

- `0`: preview / zsl
- `1`: snapshot
- `2`: HDR snapshot
- `3`: 4K video
- `4`: video
- `5`: HFR 120

The current extractor run finds only five unique register-table payloads in the blob.

This is not a contradiction. It likely means at least two resolution indices share the same low-level sensor programming script while differing only in higher-level metadata or tuning selection.

That matches the structure seen here:

- a six-entry small metadata block near `0x6100`
- five unique extracted `<u16 reg, u16 val>` payload families
- repeated per-mode metadata values embedded near `0x6230+`

There is already one concrete duplicate in `RE/imx386_regs.c`:

- `imx386_mode_1`
- `imx386_mode_4`

Those two reconstructed tables are byte-for-byte identical in the current extraction.

So the "six resolution indices, five unique scripts" result is real and not just a heuristic artifact from the extractor.

At the blob-storage level, the six physical table slots are:

- `0x4106c`
- `0x4507c`
- `0x4908c`
- `0x4d09c`
- `0x510ac`
- `0x550bc`

Using `RE/extract_sensor_modes.py --keep-duplicates` confirms all six stored tables.

The duplicate payload is:

- `0x4507c`
- `0x510ac`

Those two slots carry the same register sequence.

One important negative result:

- none of the six table start offsets appear as direct absolute pointers anywhere in the raw blob image
- no simple relocated pointer array to `0x4106c`, `0x4507c`, `0x4908c`, `0x4d09c`, `0x510ac`, or `0x550bc` has been found

That implies the mode-table ownership is probably expressed through:

- implicit slot ordering
- relative indexing from nearby packed metadata
- or code-generated addresses rather than a plain array of pointers

The code now supports that interpretation directly:

- `0x29e9` / `0x2a75` implement computed access to pointer-valued slots
- selectors `0..0xc` map to inline words starting at `obj + 0x8`
- selectors with `(sel & ~3) == 0xc0` map into a lazily initialized block at `obj + 0x1d8`
- that block is initialized on first use through the helper entry at `0x2858`

- `0x2b21` / `0x2bc5` implement computed access to pair/doubleword slots
- selector families `0x70`, `0x100`, and `0x110` map into three lazily initialized inline regions:
  - `obj + 0x158`
  - `obj + 0x50`
  - `obj + 0xd8`
- those regions are initialized by tiny constructors at:
  - `0x2814`
  - `0x2804`
  - `0x280c`

So the object model here is not "top-level struct with many direct pointers". It is closer to:

- a small vtable-backed context at `obj + 0x8`
- several computed slot spaces
- multiple lazy inline backing blocks that are materialized only when a selector family is accessed

The current extracted register scripts also split into two obvious families:

- `imx386_mode_0`, `imx386_mode_2`, `imx386_mode_3`
  - `0x3049 = 1`
  - `0x30e6/0x30e7 = 0x0259`
  - `0x663a = 2`
  - `0x9311 = 0`
  - `0xa0cd..0xa0cf = 0x19`
- `imx386_mode_1`, `imx386_mode_4`, `imx386_mode_5`
  - `0x3049 = 0`
  - `0x30e6/0x30e7 = 0`
  - `0x663a = 1`
  - `0x9311 = 0x3f`
  - `0xa0cd..0xa0cf = 0x0a`

Within those families, the main visible differences are the exposure defaults at `0x0202/0x0203`.

This suggests the small tuples at `0x6100` may be indexing mode families or tuning classes rather than describing the full register scripts directly.

### Provisional resolution-index mapping

This is still a hypothesis, but it is the strongest one so far.

The six chromatix resolution indices split naturally into two groups:

- photo-oriented:
  - `0`: preview / zsl
  - `1`: snapshot
  - `2`: HDR snapshot
- video-oriented:
  - `3`: 4K video
  - `4`: video
  - `5`: HFR 120

The six stored sensor tables also split naturally into two groups:

- family A:
  - `0x4106c`
  - `0x4908c`
  - `0x4d09c`
- family B:
  - `0x4507c`
  - `0x510ac`
  - `0x550bc`

Best current interpretation:

- family A most likely backs chromatix indices `0`, `1`, and `2`
- family B most likely backs chromatix indices `3`, `4`, and `5`

Reasons:

- the embedded geometry block at `0x6230` clearly contains the full-resolution size `4032x3016`
- that fits the photo-oriented side of the chromatix configuration better than the video/HFR side
- the B-family duplicate pair fits the XML well because `4K video` and `video` can plausibly share the same low-level sensor script and differ only in higher-level tuning selection
- the shortest default exposure is in `0x550bc`, which makes it the strongest current HFR-120 candidate

Provisional slot assignment inside those groups:

- likely video side:
  - `0x550bc` -> index `5` (`HFR 120`)
  - `0x4507c` / `0x510ac` -> indices `3` and `4` (`4K video` and `video`) in unknown order
- likely photo side:
  - `0x4106c`, `0x4908c`, `0x4d09c` -> indices `0`, `1`, and `2` in unknown order

The photo-side ordering is still unresolved, but one clue stands out:

- `0x4106c` is the only long script with the extra `0x3100-0x3137` block

That makes it the strongest current candidate for the primary full-resolution preview/ZSL-style mode, leaving `0x4908c` and `0x4d09c` as the likely snapshot / HDR-snapshot pair.

The video-side ordering is also still unresolved, but the strongest current clue is:

- `0x550bc` has the smallest default exposure value in the B-family

That keeps it as the best current HFR-120 candidate, leaving the duplicate pair at `0x4507c` and `0x510ac` as the most likely `4K video` / `video` pair.

Current unresolved point:

- there is still no direct evidence that distinguishes `0x4507c` from `0x510ac`
- so `index 3` vs `index 4` remains unresolved
- the same applies to `0x4908c` vs `0x4d09c` for `snapshot` vs `HDR snapshot`

The next code-side target is therefore not a pointer table, but the constructor or selector path that populates the lazy backing regions for the `0x70`, `0x100`, and `0x110` selector families.

## Inline data regions with likely Qualcomm struct matches

These offsets look like structured sensor metadata rather than register tables:

- `0x61c8`
- `0x6230`
- `0x62b8`
- `0x7b200`

The raw file bytes at these offsets are mostly zero because the object relies on relocations. With relocations applied, the blocks become meaningful.

For `0x61c8`, `0x6230`, and `0x62b8`, the most important ownership result is:

- there are no direct absolute pointers to these addresses anywhere in the relocated image
- they sit at fixed offsets from the `sensor_open_lib()` return value `0x6008`
- so they are most likely inline substructures inside the same static sensor descriptor block

Offset from the returned pointer:

- `0x61c8 = 0x6008 + 0x1c0`
- `0x6230 = 0x6008 + 0x228`
- `0x62b8 = 0x6008 + 0x2b0`

This is a stronger fit than the earlier idea that some other object points to them through relocated data pointers.

### `0x61c8`

Relocation-applied words:

- `0x00000001`
- `0x00000000`
- `0x00000001`
- `0x00000001`
- `0x00000003`
- `0x034e034c`
- `0x03400342`
- `0x00000202`
- `0x00000204`
- `0x00000000`
- `0x00000000`
- `0x0000000a`

The key values here are sensor register addresses:

- `0x034c` and `0x034e`
- `0x0342` and `0x0340`
- `0x0202`
- `0x0204`

Those line up directly with local downstream Qualcomm camera definitions:

- `struct msm_sensor_output_reg_addr_t`
  - `x_output`
  - `y_output`
  - `line_length_pclk`
  - `frame_length_lines`
- `struct msm_sensor_exp_gain_info_t`
  - `coarse_int_time_addr`
  - `global_gain_addr`
  - `vert_offset`

Current interpretation:

- this block is not a mode table
- it likely packs output-register addresses and exposure/gain register metadata
- the surrounding scalar fields are probably adjacent sensor-lib configuration members rather than standalone arrays

### `0x6230`

Relocation-applied words start with:

- `0x00020002`
- `0x00000002`
- `0x3fa00000`
- `0x00000002`
- `0x40b8f5c3`
- `0x00000fc0`
- `0x00000bc8`
- `0x000c000c`
- `0x00100010`
- `0x004003ff`
- `0x00400040`
- `0x00000040`
- `0x00000003`
- `0x00022b00`

The obvious geometry values are:

- `0x0fc0` = `4032`
- `0x0bc8` = `3016`

That makes this a strong candidate for a `struct msm_sensor_output_info_t`-style region or a closely related packed output descriptor:

- `x_output`
- `y_output`
- `line_length_pclk`
- `frame_length_lines`
- `vt_pixel_clk`
- `op_pixel_clk`
- `binning_factor`

This still needs field-by-field alignment, but it is clearly output metadata, not register script data.

One additional structure clue inside this block:

- `0x6230` starts with scalar header-like fields and geometry values
- `0x6264`, `0x6274`, and `0x6284` then embed the repeated mode-metadata family
  - `0x00022b00`
  - `0x00023601`
  - `0x00023502`

So `0x6230` is better described as a packed per-resolution descriptor region than as a single plain `msm_sensor_output_info_t` instance.

Current interpretation:

- the front of the block likely contains output-info / timing-like fields
- the tail of the block contains additional per-mode selectors or classification values
- this region probably describes one resolution family inline inside the returned sensor descriptor

### `0x62b8`

Relocation-applied words begin with:

- `0x00000003`
- `0x00000003`
- `0x00000000`
- `0x00000000`
- `0x00000001`
- `0x00021203`

This is another small packed metadata block, but the exact Qualcomm struct match is still unclear.

### `0x7b200`

With relocations applied, this region starts with:

- `0x00000006`
- `0x00000000`
- `0x00000000`
- `0x00001a51`
- `0x00001b61`
- `0x00000fc0`
- `0x00000002`
- `0x00000000`
- `0x00000035`
- `0x00000fc0`
- `0x00000002`
- `0x00000001`
- `0x00000036`

This region has `R_ARM_RELATIVE` relocations at:

- `0x7b20c`
- `0x7b210`
- `0x7b268`

After relocation, the first two pointer-like fields resolve to:

- `0x1a51`
- `0x1b61`

Those are Thumb code addresses in the same blob, not plain sensor metadata pointers.

Current interpretation:

- this is probably a helper or PDAF-related descriptor block
- it is not yet safe to call this part of the `sensor_open_lib()` return object
- the earlier guess that it was a `sensor_driver_params_type`-style block was too strong

The exact owner of this block still needs to be confirmed from xrefs or by tracing the PDAF export path.

## Repeated non-mode data family

The region beginning at `0x79358` is not one of the six large register tables extracted into `RE/imx386_regs.c`.

Several nearby blocks repeat the same small value family:

- `0x00022b00`
- `0x00023601`
- `0x00021203`

These appear at:

- `0x79358`
- `0x7945c`
- `0x79560`
- `0x79664`
- `0x79768`
- `0x7986c`

Current interpretation:

- these are per-mode metadata or calibration-like stubs
- they are separate from the main `<u16 reg, u16 val>` programming tables at `0x4106c+`
- they should be mapped alongside the mode descriptors, not confused with the register sequences themselves

## Register table offsets already confirmed

Current extracted mode-table candidates:

- `0x4106c`
- `0x4507c`
- `0x4908c`
- `0x4d09c`
- `0x510ac`
- `0x550bc`

These still need to be mapped to concrete Qualcomm resolution entries.

## Immediate next reverse-engineering steps

1. Identify where the `.data.rel.ro` pointer table is referenced from the returned object around `0x6000`.
2. Name the callback functions by behavior, starting with `0x26e1` and the `0x29xx-0x2dxx` group.
3. Decode the inline metadata blocks near `0x61c8`, `0x6230`, and `0x62b8`.
4. Match those blocks to the six extracted register tables.
5. Compare the resulting layout against known Qualcomm `sensor_lib_t` downstream definitions.
