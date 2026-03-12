# IMX386 `sensor_open_lib` Notes

This file captures the current reverse-engineering state of `RE/libmmcamera_oxygen_imx386_sunny.so`.

The current conservative C-facing slice for the helper ABI is captured in
`RE/camera_runtime_structs.h` as:

- `struct sensor_helper_ops_v0`
- `struct sensor_helper_subobject_head_v0`
- `struct sensor_helper_snapshot4_v0`

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

`0x286c` seeds the helper subobject with this table directly:

- `[sub + 0x0] = 0x5e2c`
- so the helper `ops` pointer is the relocation-backed table starting at `0x5e2c`

Resolved helper ops slots:

- `ops+0x00 -> 0x26e1`
- `ops+0x04 -> 0x26e1`
- `ops+0x08 -> 0x29c7`
- `ops+0x0c -> 0x29e9`
- `ops+0x10 -> 0x2a75`
- `ops+0x14 -> 0x2b01`
- `ops+0x18 -> 0x2b21`
- `ops+0x1c -> 0x2bc5`
- `ops+0x20 -> 0x2c91`
- `ops+0x24 -> 0x2ce1`
- `ops+0x28 -> 0x2cf9`
- `ops+0x2c -> 0x2d0f`
- `ops+0x30 -> 0x2d15`
- `ops+0x34 -> 0x2d85`
- `ops+0x38 -> 0x2df5`
- `ops+0x3c -> 0x2dfb`

## First interesting code targets

### `0x26e1`

- Thumb function starting at `0x26e0/0x26e1`
- Large function with stack frame and internal calls
- Almost certainly a real library callback, not inert data
- It allocates a large temporary stack object and calls `0x25f4`
- `0x25f4` drives an interface made of function pointers at object offsets `0x8`, `0xc`, `0x10`, `0x14`, `0x18`, `0x1c`, `0x20`, `0x24`, and `0x28`
- This looks like a wrapper around a smaller internal vtable-like object. The
  later `obj+8` analysis now suggests that this subobject also carries saved
  execution/register context, not just table metadata.

### `0x29c7`, `0x29e9`, `0x2a75`, `0x2b01`, `0x2b21`, `0x2bc5`, `0x2c91`

- All decode as Thumb code
- These are likely the major sensor operations exposed through the library descriptor

### `0x2ce1`, `0x2cf9`, `0x2d0f`, `0x2d15`, `0x2d85`, `0x2df5`, `0x2dfb`

- Also decode as valid code or code-adjacent entries
- These are likely utility callbacks, PDAF hooks, or small mode-specific helpers

## Provisional callback role map

These names are not final, but they are grounded in the observed control flow.

### `0x29c7`

- Tiny validator body
- This is the concrete implementation behind `ops+0x08`
- Checks whether a selector belongs to one of the supported compact selector classes
- Best current name: `is_supported_selector_family()`

### `0x29e9`

- Getter for pointer-valued slots
- Accepts an object pointer and selector
- Handles special selectors `-1`, `-2`, `0xd`, `0xe`, `0xf`
- Reads from inline offsets `+0x3c`, `+0x40`, `+0x44`, or generic table storage starting at `+0x8`
- Selector family masked by `0xc0` triggers lazy initialization using the block at `+0x1d8`
- Best current name: `get_slot_ptr()`

Now directly confirmed from the code:

- selector `-1` / `-2` / `0xd` -> `sub+0x3c`
- selector `0xe` -> `sub+0x40`
- selector `0xf` -> `sub+0x44`
- selectors `0..0xc` -> word table at `sub+0x8 + idx*4`
- selector family with `(sel & ~3) == 0xc0` lazily snapshots from `sub+0x1d8`
  using the save helper at `0x2858`, gated by byte `sub+0x4c`

Stronger evidence for selector `0xd`:

- parser paths at `0x1fec`, `0x2052`, `0x213a`, `0x23c0`, `0x2430`, and
  `0x24a2` repeatedly call the get/set wrappers with selector `0xd`
- the returned/stored value is advanced by `+4`, `+0x204`, or by iterating over
  packed entries
- this is consistent with selector `0xd` naming a mutable parse cursor pointer
  (or current decode position) in the helper subobject

### `0x2a75`

- Setter counterpart to `0x29e9`
- Writes to inline offsets `+0x3c`, `+0x40`, `+0x44` or generic table storage at `+0x8`
- Selector family masked by `0xc0` triggers lazy initialization of the `+0x1d8` block
- Best current name: `set_slot_ptr()`

Now directly confirmed from the code:

- selector `-1` / `-2` / `0xd` writes `sub+0x3c`
- selector `0xe` writes `sub+0x40`
- selector `0xf` writes `sub+0x44`
- selectors `0..0xc` write `sub+0x8 + idx*4`
- selector family with `(sel & ~3) == 0xc0` lazily snapshots from `sub+0x1d8`
  on first use, gated by byte `sub+0x4c`

Current safe interpretation of the special slots:

- `sub+0x3c` / selector `0xd`: mutable parse cursor pointer
- `sub+0x40` / selector `0xe`: paired pointer/state slot, still unresolved
- `sub+0x44` / selector `0xf`: paired pointer/state slot, still unresolved

The `0xe` / `0xf` pair is used together near the end of the parser loop:

```asm
0x222c add  r2, sp, #4
0x2230 movs r1, #0xe
0x2232 bl   0x28b0   ; get selector 0xe
...
0x223a movs r1, #0xf
0x223c bl   0x28de   ; set selector 0xf
```

So these two slots are related, but not yet named safely.

One stronger behavioral detail is now confirmed:

- at parser completion (`0x2224..0x2240`), if status bit 0 is clear:
  - selector `0xe` is read into a temporary
  - that same value is written back through selector `0xf`

So the safest current wording is:

- `sub+0x40` / selector `0xe`: source-side terminal pointer/state slot
- `sub+0x44` / selector `0xf`: sink-side mirrored terminal pointer/state slot

That is still deliberately conservative; the exact semantic names (for example
start/end, current/next, or input/output cursor) are not proven yet.

### Generic selector-dispatch helper at `0x22f8`

The helper at `0x22f8` is not specific to selectors `0xe` / `0xf`. It is a
generic adapter that routes several selector families into the lower-level
get/set wrappers:

- for op-class `0`:
  - accepts selectors `0x0..0xf`
  - uses `0x28b0` (getter)
- for op-class `1`:
  - accepts selectors `0x70..0x8f`
  - uses `0x2922` (pair getter path)
- for op-class `2`:
  - accepts selectors `0xc0..0xc3`
  - uses `0x28b0`
- for op-class `3`:
  - accepts selectors `0x100..0x10f` or `0x100..0x11f` depending on mode
  - uses `0x29c0` / `0x2922`

Instruction-faithful shape:

```asm
0x22a8 bl 0x28b0   ; plain getter family
...
0x22bc bl 0x29c0   ; tail-call via ops+0x3c into the 0x100-family gate
...
0x22d4 bl 0x2922   ; pair getter path
```

Current interpretation:

- `0x22f8` is a selector-family dispatcher for the helper subobject ABI
- it explains why parser code reuses selector `0xd` and then mixes in
  `0x70` / `0xc0` / `0x100` class selectors through the same front-end helper

### `0xc0` selector family

The plain getter path at `0x29e8..0x2a4e` now resolves this family directly:

```asm
0x2a2a bic    r0, r5, #0x3
0x2a2e cmp    r0, #0xc0
0x2a32 ldrb.w r0, [r4, #0x4c]
0x2a38 movs   r0, #0x1
0x2a3a strb.w r0, [r4, #0x4c]
0x2a3e add.w  r0, r4, #0x1d8
0x2a42 blx    0x2858
0x2a46 add.w  r0, r4, r5, lsl #2
0x2a4a sub.w  r0, r0, #0x128
0x2a4e ldr    r0, [r0]
```

Safe interpretation:

- selectors `0xc0..0xc3` index a 4-word snapshot block rooted at `sub+0x1d8`
- `sub+0x4c` is the one-time snapshot gate for this family
- the first access snapshots the block through `0x2858`
- later accesses return one word at:
  - `0xc0 -> sub+0x1d8`
  - `0xc1 -> sub+0x1dc`
  - `0xc2 -> sub+0x1e0`
  - `0xc3 -> sub+0x1e4`

The save helper `0x2858` is an ARM stub that stores four coprocessor-register
words into this block, so the safest current name is:

- compact 4-word snapshot family

That is now enough to preserve the ABI shape without pretending we know the
architectural register semantics yet.

### `0x29c0`

- Tiny trampoline, not a substantive validator body
- Implementation:

```asm
0x29c0 ldr r1, [r0]
0x29c2 ldr r1, [r1, #0x3c]
0x29c4 bx  r1
```

- Current safest reading:
  - dispatches through `ops+0x3c`, which resolves to `0x2dfb`
  - `0x2dfb` sets `sub+0x48 = 1` and returns
  - used by the `0x22f8` front-end before the `0x100`-family pair getter path
  - so `0x29c0` is an arming step for the `0x100` family, not the actual pair
    getter/setter logic

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
- Best current name: `ensure_subcontext_ready()`

Important correction:

- this helper operates on the internal subobject rooted at `obj + 0x8`
- so the `+0x210` and `+0x1f8` offsets here are `sub+0x210` / `sub+0x1f8`
- they are not automatically the same fields as raw top-level
  `obj+0x210` / `obj+0x1f8`

### `0x2ce1`

- Plain block copy helper
- Copies 40 bytes from `obj + 0x1e8` into caller storage
- Best current name: `copy_context_block()`

### `0x2cf9`

- Wrapper that passes `obj + 0x8` into `0x2e84`
- Then tail-calls another routine at `0x3590`
- Best current name: `restore_and_resume_subcontext()`

Stronger current interpretation:

- this is a restore-and-resume path for the helper subobject, not a plain
  metadata getter

### `0x2922`

- Pair getter wrapper
- Calls `[sub->ops + 0x14]` as a readiness/permission gate
- On success, calls `[sub->ops + 0x18]`
- Stores the returned `(r0, r1)` pair to caller memory
- Best current name: `get_pair_slot_wrapped()`

Instruction-faithful shape:

```asm
ldr r2, [ops, #0x14]
blx r2
cmp r0, #1
bne fail
ldr r2, [ops, #0x18]
blx r2
strd r0, r1, [out]
```

### `0x2952`

- Extended pair operation wrapper
- Calls `[sub->ops + 0x14]` as the same readiness/permission gate
- On success, calls `[sub->ops + 0x1c]` with four arguments
- Best current name: `set_pair_slot_wrapped()` or `operate_pair_slot_wrapped()`

Instruction-faithful shape:

```asm
ldr r2, [ops, #0x14]
blx r2
cmp r0, #1
bne fail
ldr r4, [ops, #0x1c]
blx r4
```

Current interpretation:

- `0x2922` / `0x2952` are generic wrappers around the lower-level pair-slot ABI
- they are the bridge used by the `0x22f8` selector-family dispatcher for the
  `0x70` / `0x100`-class pair operations

### Concrete pair-selector backing regions

The lower-level pair getter/setter paths at `0x2b20` and `0x2bc4` now resolve
the selector families to specific backing regions inside the helper subobject:

- selector family `0x100`
  - lazy-save gate: `sub+0x49`
  - mode/arm flag: `sub+0x48`
  - save area: `sub+0x50`
  - getter loads pair from `sub + 0x50 + idx*8`
  - setter stores pair to the same region
- selector family `0x110`
  - lazy-save gate: `sub+0x4a`
  - save area: `sub+0xd8`
  - getter loads pair from `sub + 0xd8 + (idx-0x110)*8`
  - setter stores pair to the same region
- selector family `0x70`
  - lazy-save gate: `sub+0x4b`
  - save area: `sub+0x158`
  - getter loads pair from `sub + 0x158 + (idx-0x70)*8`
  - setter stores pair to the same region

Instruction-faithful load/store offsets:

```asm
; getter 0x100
0x2b46 add.w r0, r5, #0x50
0x2b4a ldrb.w r1, [r5, #0x48]
0x2b4e cbz    r1, 0x2b92
0x2b50 blx    0x2804
...
0x2b92 blx    0x27fc
...
0x2b96 add.w r0, r5, r4, lsl #3
0x2b9a sub.w r0, r0, #0x7b0   ; == sub+0x50 + idx*8

; getter 0x110
0x2b80 add.w r0, r5, #0xd8
...
0x2b88 add.w r0, r5, r4, lsl #3
0x2b8c sub.w r0, r0, #0x7a8   ; == sub+0xd8 + (idx-0x110)*8

; getter 0x70
0x2b62 add.w r0, r5, #0x158
...
0x2b6a add.w r0, r5, r4, lsl #3
0x2b6e sub.w r0, r0, #0x228   ; == sub+0x158 + (idx-0x70)*8
```

Direct dump correlation from IMX386:

- `sub+0x50` matches the small raw-prefix data from the top-level descriptor
- `sub+0xd8` matches the compact resolution metadata / triplet region
- `sub+0x158` reaches into the area that begins with the static
  output/timing/exposure/gain register block

This is the strongest current link between selector families and concrete blob
substructures.

Stronger family-specific interpretation:

- family `0x100`
  - best current reading: small static prefix/header pairs
  - reason:
    - `sub+0x50` maps directly onto raw top-level `obj+0x58`
    - that region contains the early small scalar data from the sensor
      descriptor prefix
    - representative pairs are compact control-like constants rather than
      register addresses or per-resolution triplets
- family `0x110`
  - best current reading: compact per-resolution metadata pairs
  - reason:
    - `sub+0xd8` maps directly onto raw top-level `obj+0xe0`
    - that region contains the same small regular values seen in the
      `resolution_triplets_0x0f8` / compact mode-metadata discussion
    - the content is too small and regular to be register addresses
- family `0x70`
  - best current reading: static sensor capability / register-address pairs
  - reason:
    - `sub+0x158` maps into raw top-level `obj+0x160`
    - the nonzero tail reaches:
      - `obj+0x1c0`: small scalar header words
      - `obj+0x1c8`: packed `x_output/y_output` register addresses
      - `obj+0x1d0`: packed `line_length/frame_length` and exposure register
    - this is consistent with a static capability pair-family, not per-mode
      triplets

Concrete IMX386 `0x70` family tail entries from the dump:

- family index `0x7c` (`sub+0x1b8` / `obj+0x1c0`):
  - pair = `(0x00000001, 0x00000000)`
- family index `0x7d` (`sub+0x1c0` / `obj+0x1c8`):
  - pair = `(0x00000001, 0x00000001)`
- family index `0x7e` (`sub+0x1c8` / `obj+0x1d0`):
  - pair = `(0x00000003, 0x034e034c)`
- family index `0x7f` (`sub+0x1d0` / `obj+0x1d8`):
  - pair = `(0x03400342, 0x00000202)`

So the family `0x70` window is not random:

- its high entries walk directly into the proven static register-address block
- that makes it a strong candidate for an indexed static-info/capability view
  over the top-level sensor descriptor

Concrete IMX386 `0x100` family entries from the dump:

- family index `0x100` (`sub+0x50` / `obj+0x58`):
  - pair = `(0x00000002, 0x00000000)`
- family index `0x101` (`sub+0x58` / `obj+0x60`):
  - pair = `(0x00000000, 0x00000001)`
- family index `0x102` (`sub+0x60` / `obj+0x68`):
  - pair = `(0x00000002, 0x00000001)`
- family index `0x105` (`sub+0x78` / `obj+0x80`):
  - pair = `(0x016e3600, 0x00000001)`
- family index `0x107` (`sub+0x88` / `obj+0x90`):
  - pair = `(0x00000002, 0x0000000b)`

So the family `0x100` window currently looks like:

- an indexed view over the small static prefix/header section of the top-level
  descriptor
- likely compact sensor capability or mode-policy tuples
- not a direct register-address family and not the per-resolution metadata area
- its lazy materialization has two pieces:
  - `sub+0x49`: one-time init gate for the backing region
  - `sub+0x48`: arm flag selecting whether the init stub is `0x27fc` or `0x2804`

Cross-sensor check:

- IMX386 and both OV12A variants carry the same `0x100` family pairs
- S5K5E8 carries a different `0x100` family pattern, but the two S5K5E8 vendor
  variants match each other

So the safest current conclusion is:

- the `0x100` block is vendor-invariant within a given sensor family
- but not stable enough across sensor families to assign one global semantic
  field name per pair index yet
- this makes it look more like a compact sensor-family policy/header tuple block
  than optical metadata or purely vendor-tuning data

Cross-sensor check for the `0x110` family:

- IMX386 and both OV12A variants carry the same `0x110` pair pattern:
  - `0x113 -> (6, 0)`
  - `0x115 -> (1, 1)`
  - `0x117 -> (1, 2)`
  - `0x118 -> (1, 0)`
  - `0x119 -> (0, 2)`
- S5K5E8 qtech and ofilm carry a different but internally matching pattern:
  - `0x113 -> (8, 1)`
  - `0x114 -> (1, 0)`
  - `0x115 -> (1, 1)`
  - `0x117 -> (1, 0)`
  - `0x119 -> (1, 2)`

So the safest current conclusion for `0x110` is:

- it is also vendor-invariant within a sensor family
- it still behaves like compact per-resolution/mode metadata rather than lens
  or optical metadata
- but the pair indices are not stable enough across sensor families to freeze
  one global semantic name per slot

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
- It copies a 0x40-byte callback template into `sub+0x8`
- It clears bytes up to roughly `+0x1c2`
- It calls `0x2d85` on that sub-object immediately after initialization
- `0x25f4` and the `0x28xx-0x29xx` helpers then operate on this sub-object through function pointers

This is now best described as an internal helper subobject used by the
higher-level sensor library callbacks. It carries both table-like state and a
saved execution/register context used by the restore path.

The copied template comes from the relocation-backed callback table already
visible in `.data.rel.ro`:

- template source starts at `0x5e2c`
- it contains the 16 callback entries previously listed:
  - `0x26e1`, `0x26e1`, `0x29c7`, `0x29e9`
  - `0x2a75`, `0x2b01`, `0x2b21`, `0x2bc5`
  - `0x2c91`, `0x2ce1`, `0x2cf9`, `0x2d0f`
  - `0x2d15`, `0x2d85`, `0x2df5`, `0x2dfb`

So `0x286c` is not building the helper subobject from scratch; it seeds it from
the blob's embedded callback template and then clears the larger save-area
behind it.

The callback wrapper at `0x2cf8` makes this base split explicit:

```asm
0x2cfa add.w r4, r0, #0x8
0x2cfe mov   r0, r4
0x2d00 bl    0x2e84
0x2d04 mov   r0, r4
0x2d0a b.w   0x3590
```

So later helper offsets must be read with two bases in mind:

- raw top-level sensor object: `obj + off`
- helper subobject: `sub + off`, where `sub = obj + 0x8`

This resolves several earlier offset collisions. For IMX386:

- `sub+0x1d0 == obj+0x1d8`
- `sub+0x1d8 == obj+0x1e0`
- `sub+0x1e8 == obj+0x1f0`
- `sub+0x210 == obj+0x218`
- `sub+0x220 == obj+0x228`

Direct dump sanity check from the blob supports this:

- `sub+0x220` starts with `0x00020002, 0x00000002, 0x3fa00000`, which matches
  raw top-level `obj+0x228`
- `sub+0x1e8` starts with `1.0f, 16.0f, 16.0f`, which matches raw top-level
  `obj+0x1f0`

### Register-restore helpers inside the subobject path

The helper at `0x2e84` is now much clearer when read together with the ARM
trampolines:

```asm
0x2e88 ldrb.w r0, [r4, #0x41]
0x2e8e ldrb.w r1, [r4, #0x40]
0x2e92 add.w  r0, r4, #0x48
...
0x2e98 blx    0x3528
0x2e9e blx    0x3520
...
0x2ea2 ldrh.w r0, [r4, #0x42]
...
0x2eac add.w  r0, r4, #0xd0
0x2eb0 blx    0x3530
...
0x2ebe add.w  r0, r4, #0x150
0x2ec2 blx    0x3538
...
0x2ecc add.w  r0, r4, #0x1d0
0x2ed4 b.w    0x35a0
```

Resolved targets:

- `0x3520` / `0x3528`: load a large register bank from `sub+0x48`
  - for IMX386 these two restore stubs are byte-for-byte equivalent and both
    restore `d0..d15`
- `0x3530`: load another large register bank from `sub+0xd0`
- `0x3538..0x3574`: load a further coprocessor register bank from `sub+0x150`
- `0x35a0 -> 0x3574`: final tail stage using `sub+0x1d0`
- `0x3590 -> 0x350c`: restores general registers, `sp`, and branch target from
  the subobject

Instruction-faithful core of `0x350c`:

```asm
mov lr, r0
ldm lr, {r0-r12}
ldr sp, [lr, #0x34]
ldr lr, [lr, #0x3c]
bx  lr
```

Current safe interpretation:

- the helper subobject is not just metadata storage
- it includes saved execution/register context
- `0x2e84` restores floating/coproc state in stages gated by bytes at
  `sub+0x40..0x44`
- `0x3590` resumes execution using the restored general-register frame

That makes the `obj+8` subobject look more like a resumable execution context
or generated callback state block than a plain sensor-parameter table.

### Save-side gate bytes and their paired save areas

The getter/setter paths at `0x2b20` and `0x2bc4` tie the gate bytes directly to
three lazily materialized save areas:

- `sub+0x48`
  - gate byte: `sub+0x49`
  - first-use save helper: `0x27fc` or `0x2804`
  - on IMX386, `0x27fc` and `0x2804` are byte-for-byte equivalent ARM stubs:
    both save `d0..d15` with `vstmia`
  - restored later by `0x3520` / `0x3528`
- `sub+0xd8`
  - gate byte: `sub+0x4a`
  - first-use save helper: `0x280c`
  - restored later by `0x3530`
- `sub+0x158`
  - gate byte: `sub+0x4b`
  - first-use save helper: `0x2814`
  - restored later by `0x3538..0x3574`

Separately:

- `sub+0x4c` gates the one-time snapshot rooted at `sub+0x1d8`
  used by the `0xc0` selector family

So the `sub+0x40..0x4c` bytes are now partially named by behavior:

- `sub+0x40` / `sub+0x41` / `sub+0x42` / `sub+0x43` are restore-time control
  bytes read by `0x2e84`
- `sub+0x49` / `sub+0x4a` / `sub+0x4b` are lazy-save completion flags for the
  three save areas
- `sub+0x4c` is the lazy-save completion flag for the `sub+0x1d8` snapshot path

Current safe conclusion for `sub+0x48` on IMX386:

- it still acts as a control/arm flag in the `0x100` family save path
- but the two observed save targets (`0x27fc` and `0x2804`) are identical in
  this blob, and the paired restore targets (`0x3520` and `0x3528`) are also
  identical
- so the flag changes control flow without changing the saved VFP bank on the
  current IMX386 implementation

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

### Output info array (mode geometry + clocks)

There is a contiguous array of 6 output entries starting at file offset
`0x78fc8`. Each entry is 0x40 bytes wide; the `msm_sensor_output_info_t`
payload occupies the first 20 bytes (with padding), and the rest is unknown
per-mode metadata.

Extracted output_info values:

- `idx 0`: `4032x3016`, `line=4296`, `frame=3070`, `vt=388000000`, `op=398400000`
- `idx 1`: `2016x1508`, `line=2256`, `frame=1692`, `vt=114670000`, `op=137600000`
- `idx 2`: `4032x2256`, `line=4296`, `frame=2310`, `vt=298000000`, `op=308000000`
- `idx 3`: `3840x2160`, `line=4296`, `frame=2360`, `vt=297330000`, `op=356800000`
- `idx 4`: `1920x1080`, `line=2256`, `frame=1692`, `vt=114670000`, `op=137600000`
- `idx 5`: `1920x1080`, `line=2256`, `frame=1174`, `vt=318000000`, `op=381600000`

These match the expected mode families (full, binned preview, 16:9, 4K,
1080p, and a high-FPS 1080p variant).

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

### Provisional `0x6230` field map (inferred)

This is a best-effort decode based on cross-sensor comparisons. Names are
intentional guesses; treat them as provisional until we locate a CAF sensor
lib header.

Index -> value (IMX386) -> proposed meaning:

- `0`: `0x00020002` -> packed small constants (likely even/odd alignment or
  minimum crop step in pixels)
- `1`: `0x00000002` -> small constant (likely scale/step selector)
- `2`: `0x3fa00000` (1.25) -> pixel size in microns (IMX386 = 1.25µm, S5K5E8 = 1.12µm)
- `3`: `0x00000002` -> small constant (likely another step or selector)
- `4`: `0x40b8f5c3` (5.78) -> unknown float; possibly focal length or sensor-specific
  calibration factor (OV12A shows 1.33, S5K5E8 is 0)
- `5`: `0x00000fc0` -> active array width (4032)
- `6`: `0x00000bc8` -> active array height (3016)
- `7`: `0x000c000c` -> packed margin or crop start (12, 12)
- `8`: `0x00100010` -> packed margin or crop start (16, 16)
- `9`: `0x004003ff` -> packed black level / max raw value (0x0040, 0x03ff)
- `10`: `0x00400040` -> packed black level per channel (0x0040, 0x0040)
- `11`: `0x00000040` -> black level scalar (64)
- `12`: `0x00000003` -> CFA / format selector (unknown, small enum)
- `13`: `0x00022b00` -> unknown packed register/value (IMX-only)
- `17`: `0x00023601` -> unknown packed register/value (IMX-only)
- `21`: `0x00023502` -> unknown packed register/value (IMX-only)

The IMX-only packed words likely encode extra sensor-specific registers or
PDAF-related hooks; OV12A and S5K5E8 set these to zero.

#### Evidence from `libmmcamera2_sensor_modules.so`

`libmmcamera2_sensor_modules.so` reads the first word of this block as two
packed `u16` values and uses the next word (`0x22c`) as a comparison/subtraction
reference. It also treats `0x230` (word 2) as a float and computes `1.0 / value`,
which aligns with the pixel size interpretation.

See `RE/mmcamera2_sensor_modules_offsets.md` for the exact disassembly
addresses and notes.

### `0x1e8` looks like gain/exposure limits (inferred)

The `0x1e8` block is consistent across sensors:

- word 1 is `8` or `10` (a small integer, likely gain or exposure step count)
- word 2 is `1.0` (base gain)
- word 3/4 are `15.5` or `16.0` (likely max analog/digital gain)
- word 9 varies per sensor (`0xFFF5`, `0x7FF7`, `0x4056`), possibly the max
  frame-length or coarse integration time limit in sensor lines

Tentative interpretation:

- word 1: small integer (8 or 10) -> likely max analog gain step count or
  gain table index count
- word 2: base gain = 1.0
- word 3/4: max gain (15.5 or 16.0)
- word 9: sensor-specific upper limit (possibly max coarse integration time)

This mapping is provisional; it matches patterns but still needs validation
against a known sensor-lib header or CAF source.

#### Evidence from `libmmcamera2_sensor_modules.so`

The Qualcomm sensor modules library consumes these fields directly. The key
accesses are documented in `RE/mmcamera2_sensor_modules_offsets.md`, but the
two most important references for this block are:

- `0x1f4` (word 3) is copied as a float into an output struct (likely max gain).
- `0x20c` (word 9) is copied as a raw `u32` limit (likely max coarse integration
  or linecount limit).

This strengthens the interpretation of the `0x1e8` block as gain/exposure
limits, with word 3 as `max_gain` and word 9 as an exposure ceiling.

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
