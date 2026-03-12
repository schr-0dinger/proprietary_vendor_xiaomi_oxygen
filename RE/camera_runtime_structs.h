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
