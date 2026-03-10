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

### `0x29c7`, `0x29e9`, `0x2a75`, `0x2b01`, `0x2b21`, `0x2bc5`, `0x2c91`

- All decode as Thumb code
- These are likely the major sensor operations exposed through the library descriptor

### `0x2ce1`, `0x2cf9`, `0x2d0f`, `0x2d15`, `0x2d85`, `0x2df5`, `0x2dfb`

- Also decode as valid code or code-adjacent entries
- These are likely utility callbacks, PDAF hooks, or small mode-specific helpers

## Inline data regions worth naming next

These offsets look like structured sensor metadata rather than register tables:

- `0x61c8`
- `0x6230`
- `0x62b8`

Observed characteristics:

- they contain sane small integers
- they contain values that look like dimensions, frame lengths, and float constants
- they sit far away from the register arrays at `0x4106c+`

These are good candidates for resolution or output descriptors.

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
