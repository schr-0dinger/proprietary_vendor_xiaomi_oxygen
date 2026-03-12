#ifndef RE_CAMERA_RUNTIME_STRUCTS_H
#define RE_CAMERA_RUNTIME_STRUCTS_H

#include <stdint.h>

/*
 * Conservative reconstruction of the 0x28-byte context block copied by the
 * IMX386 sensor-lib helper at 0x2ce0.
 *
 * Proven facts:
 * - the helper copies exactly 0x28 bytes from obj+0x1e8
 * - 0x2c91 separately consumes helper-subobject field sub+0x1f8 where
 *   sub = obj + 0x8
 * - obj+0x230 is a float pixel-size field in the returned sensor blob
 *
 * Not proven:
 * - that this slice matches the higher-level probe/session object tail used by
 *   libmmcamera2_sensor_modules.so
 * - that obj+0x21c / obj+0x220 inside the returned sensor blob are focal-length
 *   and f-number fields
 */
struct sensor_lib_context_block_v0 {
  uint16_t field_1e8;       /* obj+0x1e8, unresolved paired u16 */
  uint16_t field_1ea;       /* obj+0x1ea, unresolved paired u16 */
  uint32_t aux0;            /* obj+0x1ec, unresolved */
  uint32_t min_like_1f0;    /* obj+0x1f0, IMX386 = 1.0f */
  uint32_t limit_like_1f4;  /* obj+0x1f4, IMX386 = 16.0f */
  uint32_t limit_like_1f8;  /* obj+0x1f8, IMX386 = 16.0f */
  uint32_t field_1fc;       /* obj+0x1fc, unresolved */
  uint32_t field_200;       /* obj+0x200, unresolved */
  uint32_t field_204;       /* obj+0x204, unresolved */
  uint32_t field_208;       /* obj+0x208, unresolved */
};

/*
 * Minimal reconstruction of the static output/timing register-address block
 * directly observed in libmmcamera_oxygen_imx386_sunny.so.
 */
struct sensor_output_reg_addr_v0 {
  uint16_t x_output;          /* base+0x1d4, IMX386: 0x034c */
  uint16_t y_output;          /* base+0x1d6, IMX386: 0x034e */
  uint16_t line_length_pclk;  /* base+0x1d8, IMX386: 0x0342 */
  uint16_t frame_length_lines;/* base+0x1da, IMX386: 0x0340 */
  uint32_t coarse_int_time;   /* base+0x1dc, IMX386: 0x0202 */
  uint32_t global_gain;       /* base+0x1e0, IMX386: 0x0204 */
};

/*
 * Relocation-backed helper ops table seeded into the IMX386 helper subobject by
 * 0x286c. This is a concrete ABI surface for the internal selector-family
 * helpers, even though several callback semantics are still intentionally named
 * conservatively.
 */
struct sensor_helper_ops_v0 {
  void *prepare0;              /* +0x00 -> 0x26e1 */
  void *prepare1;              /* +0x04 -> 0x26e1 */
  void *is_supported_selector; /* +0x08 -> 0x29c7 */
  void *get_slot_ptr;          /* +0x0c -> 0x29e9 */
  void *set_slot_ptr;          /* +0x10 -> 0x2a75 */
  void *is_valid_pair_slot;    /* +0x14 -> 0x2b01 */
  void *get_pair_slot;         /* +0x18 -> 0x2b21 */
  void *set_pair_slot;         /* +0x1c -> 0x2bc5 */
  void *ensure_ready;          /* +0x20 -> 0x2c91 */
  void *copy_context_block;    /* +0x24 -> 0x2ce1 */
  void *restore_and_resume;    /* +0x28 -> 0x2cf9 */
  void *get_state_flag;        /* +0x2c -> 0x2d0f */
  void *format_self_path;      /* +0x30 -> 0x2d15 */
  void *init_context;          /* +0x34 -> 0x2d85 */
  void *mark_primary_present;  /* +0x38 -> 0x2df5 */
  void *arm_primary_block;     /* +0x3c -> 0x2dfb */
};

/*
 * Compact 4-word snapshot block read by selector family 0xc0..0xc3. The
 * architectural register meaning of each word is not proven yet, so keep the
 * fields opaque while preserving the ABI shape.
 */
struct sensor_helper_snapshot4_v0 {
  uint32_t word_c0;            /* selector 0xc0 */
  uint32_t word_c1;            /* selector 0xc1 */
  uint32_t word_c2;            /* selector 0xc2 */
  uint32_t word_c3;            /* selector 0xc3 */
};

/*
 * Proven front slice of the helper subobject rooted at obj+0x8.
 *
 * Observed behavior:
 * - [sub+0x0] points at the relocation-backed helper ops table
 * - [sub+0x4] is a second initializer-written pointer, still unresolved
 * - [sub+0x8..0x37] hold the inline callback/slot table for selectors 0..0xc
 * - sub+0x3c/sub+0x40/sub+0x44 are the special pointer/state slots
 * - sub+0x48..0x4c are the arm/lazy-save flags for the family backing regions
 *
 * The larger save areas behind this header are modeled separately in the
 * open-source IMX386 scaffold as raw words because their full semantic names
 * are not required yet for a compile-safe replacement.
 */
struct sensor_helper_subobject_head_v0 {
  const struct sensor_helper_ops_v0 *ops; /* sub+0x00 */
  const void *opaque_init_ptr;            /* sub+0x04, unresolved */
  uint32_t slot_words[13];                /* sub+0x08..0x3c */
  uint32_t parse_cursor;                  /* sub+0x3c, selector 0xd */
  uint32_t terminal_src;                  /* sub+0x40, selector 0xe */
  uint32_t terminal_dst;                  /* sub+0x44, selector 0xf */
  uint8_t primary_arm_flag;               /* sub+0x48 */
  uint8_t primary_ready;                  /* sub+0x49, family 0x100 */
  uint8_t mode_ready;                     /* sub+0x4a, family 0x110 */
  uint8_t static_ready;                   /* sub+0x4b, family 0x70 */
  uint8_t snapshot4_ready;                /* sub+0x4c, family 0xc0 */
  uint8_t reserved_4d[0x03];
};

/*
 * Minimal downstream-visible header for the cached stream/crop configuration
 * blob at base+0x7a68.
 *
 * Observed behavior:
 * - opcode 0x68 fills this blob
 * - 0x1b2 bytes can be copied to a caller buffer
 * - the first 0x84 bytes are posted downstream
 *
 * The interior field layout is not yet stable enough to name, so keep this as
 * an opaque header-sized payload for now.
 */
struct sensor_cached_stream_crop_blob_v0 {
  uint8_t downstream_header[0x84]; /* base+0x7a68, downstream-visible prefix */
};

/*
 * Tail slice of the higher-level probe/export source object populated by
 * sensor_xml_util_get_camera_probe_config and later consumed by
 * translate_sensor_slave_info.
 *
 * Observed behavior:
 * - base+0x200 stores CameraId from camera_config.xml and is validated as < 4
 * - base+0x204 stores ModesSupported
 * - base+0x208 stores normalized Position
 * - base+0x20c stores MountAngle
 */
struct sensor_probe_source_tail_v0 {
  uint8_t camera_id;            /* base+0x200 */
  uint8_t reserved_201[0x03];
  int32_t modes_supported;      /* base+0x204 */
  uint32_t position;            /* base+0x208, normalized internal code */
  uint32_t sensor_mount_angle;  /* base+0x20c */
};

/*
 * Conservative draft for the session-data export blob populated by
 * module_sensor_get_session_data.
 *
 * Observed behavior around 0x11fb2..0x11fca and 0x1314a..0x1315a:
 * - a 0x1b2-byte block from base+0x7a68 is copied to dst+0x30c
 * - base+0x7a64 is copied to dst+0x4c0
 * - base+0x204/0x208/0x20c are copied to dst+0x00/0x0c/0x10
 * - translate_sensor_slave_info later maps those source fields into
 *   sensor_init_params.{modes_supported,position,sensor_mount_angle}
 *
 * The destination object contains many more fields, but this captures the
 * session-facing slice currently tied to the cached stream/crop blob.
 */
struct sensor_session_data_export_v0 {
  uint32_t modes_supported;       /* dst+0x000 <- base+0x204 */
  uint8_t reserved_004[0x08];
  uint32_t position;              /* dst+0x00c <- base+0x208 */
  uint32_t sensor_mount_angle;    /* dst+0x010 <- base+0x20c */
  uint8_t reserved_014[0x2f8];
  struct sensor_cached_stream_crop_blob_v0 stream_crop_header; /* dst+0x30c */
  uint8_t reserved_390[0x130];
  uint32_t cached_blob_meta;      /* dst+0x4c0 <- base+0x7a64 */
};

#endif /* RE_CAMERA_RUNTIME_STRUCTS_H */
