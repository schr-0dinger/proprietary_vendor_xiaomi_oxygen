#include <stddef.h>
#include <stdint.h>

#include "imx386_reg_data.h"
#include "imx386_sensor_layout.h"
#include "qcom_sensor_compat.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/*
 * This is a source skeleton for libmmcamera_oxygen_imx386_sunny.so.
 *
 * It preserves the exported symbol name and the recovered inline data offsets
 * from RE/libmmcamera_oxygen_imx386_sunny.so, but it is not a runtime-ready
 * replacement for the proprietary blob yet.
 */

_Static_assert(offsetof(struct imx386_open_lib_layout, raw_prefix_0x20_to_0x0f7) == 0x20,
    "unexpected imx386 sensor name size");
_Static_assert(offsetof(struct imx386_open_lib_layout, resolution_triplets_0x0f8) == 0x0f8,
    "unexpected imx386 resolution triplet offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, family_0x70_header_words) == 0x1c0,
    "unexpected imx386 0x61c8 block offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, output_reg_addr_0x1d4) == 0x1d4,
    "unexpected imx386 output_reg_addr offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, context_block_0x1e8) == 0x1e8,
    "unexpected imx386 0x61f0 block offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, reserved_0x210_words) == 0x210,
    "unexpected imx386 0x210 gap offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, meta_0x228) == 0x228,
    "unexpected imx386 0x6230 block offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, meta_0x2b0) == 0x2b0,
    "unexpected imx386 0x62b8 block offset");
_Static_assert(sizeof(struct sensor_output_reg_addr_v0) == 0x10,
    "unexpected sensor_output_reg_addr_v0 size");
_Static_assert(sizeof(struct sensor_lib_context_block_v0) == 0x28,
    "unexpected sensor_lib_context_block_v0 size");
_Static_assert(sizeof(struct sensor_meta_0x228_block_v0) == 0x88,
    "unexpected sensor_meta_0x228_block_v0 size");
_Static_assert(sizeof(struct sensor_meta_0x2b0_block_v0) == 0x18,
    "unexpected sensor_meta_0x2b0_block_v0 size");

static const struct msm_sensor_output_reg_addr_t kImx386OutputRegAddr = {
    .x_output = 0x034c,
    .y_output = 0x034e,
    .line_length_pclk = 0x0342,
    .frame_length_lines = 0x0340,
};

static const struct msm_sensor_exp_gain_info_t kImx386ExpGainInfo = {
    .coarse_int_time_addr = 0x0202,
    .global_gain_addr = 0x0204,
    .vert_offset = 0,
};

/*
 * These mode entries are kept as explicit decoded values because they are
 * already validated against the proprietary blob by RE/tests/test_sensor_blocks.py.
 * They are not derived from the uncertain 0x1e8 copied context block.
 */
static const struct msm_sensor_output_info_t kImx386OutputInfo[6] = {
    {
        .x_output = 4032,
        .y_output = 3016,
        .line_length_pclk = 4296,
        .frame_length_lines = 3070,
        .vt_pixel_clk = 388000000,
        .op_pixel_clk = 398400000,
        .binning_factor = 1,
    },
    {
        .x_output = 2016,
        .y_output = 1508,
        .line_length_pclk = 2256,
        .frame_length_lines = 1692,
        .vt_pixel_clk = 114670000,
        .op_pixel_clk = 137600000,
        .binning_factor = 1,
    },
    {
        .x_output = 4032,
        .y_output = 2256,
        .line_length_pclk = 4296,
        .frame_length_lines = 2310,
        .vt_pixel_clk = 298000000,
        .op_pixel_clk = 308000000,
        .binning_factor = 1,
    },
    {
        .x_output = 3840,
        .y_output = 2160,
        .line_length_pclk = 4296,
        .frame_length_lines = 2360,
        .vt_pixel_clk = 297330000,
        .op_pixel_clk = 356800000,
        .binning_factor = 1,
    },
    {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 2256,
        .frame_length_lines = 1692,
        .vt_pixel_clk = 114670000,
        .op_pixel_clk = 137600000,
        .binning_factor = 1,
    },
    {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 2256,
        .frame_length_lines = 1174,
        .vt_pixel_clk = 318000000,
        .op_pixel_clk = 381600000,
        .binning_factor = 1,
    },
};

static const struct sensor_driver_params_type kImx386DriverParams = {
    .init_settings = NULL,
    .init_settings_size = 0,
    .mode_settings = (struct msm_camera_i2c_reg_setting *)kImx386ModeSettings,
    .mode_settings_size = ARRAY_SIZE(kImx386ModeSettings),
    .sensor_output_reg_addr = (struct msm_sensor_output_reg_addr_t *)&kImx386OutputRegAddr,
    .start_settings = NULL,
    .stop_settings = NULL,
    .groupon_settings = NULL,
    .groupoff_settings = NULL,
    .sensor_exp_gain_info = (struct msm_sensor_exp_gain_info_t *)&kImx386ExpGainInfo,
    .output_info = (struct msm_sensor_output_info_t *)kImx386OutputInfo,
};

static const struct imx386_open_lib_layout kImx386OpenLib = {
    .sensor_name = "oxygen_imx386_sunny",
    .raw_prefix_0x20_to_0x0f7 = {
        /* Family 0x100 is an indexed view over these compact header pairs. */
        0x00000020, 0x00000001, 0x00000002, 0x00000000,
        0x03860016, 0x00000000, 0x00000001, 0x00000000,
        0x00000000, 0x00000001, 0x00000002, 0x00000002,
        0x00000000, 0x00000001, 0x00000002, 0x00000000,
        0x00000000, 0x00000001, 0x00000002, 0x00000001,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x016e3600, 0x00000001, 0x00000001, 0x00000000,
        0x00000002, 0x0000000b, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000,
    },
    .resolution_triplets_0x0f8 = {
        /* Family 0x110 lands in this compact mode-metadata region. */
        0x00000006, 0x00000000, 0x00000000, 0x00000000,
        0x00000001, 0x00000001, 0x00000000,
        0x00000000, 0x00000001, 0x00000002,
        0x00000001, 0x00000000, 0x00000000,
        0x00000002, 0x00000000, 0x00000000,
        0x00000001, 0x00000002, 0x00000002,
        0x00000000, 0x00000001, 0x00000000,
    },
    .reserved_0x150_words = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000005,
    },
    .family_0x70_header_words = {
        /* Family 0x70 high entries walk into this static register-info tail. */
        0x00000001, 0x00000000, 0x00000001, 0x00000001, 0x00000003,
    },
    .output_reg_addr_0x1d4 = {
        .x_output = 0x034c,
        .y_output = 0x034e,
        .line_length_pclk = 0x0342,
        .frame_length_lines = 0x0340,
        .coarse_int_time = 0x0202,
        .global_gain = 0x0204,
    },
    .reserved_0x1e4 = 0x00000000,
    .context_block_0x1e8 = {
        .field_1e8 = 0x0000,
        .field_1ea = 0x000a,
        .field_1ec = 0x3f800000,
        .field_1f0 = 0x41800000,
        .field_1f4 = 0x41800000,
        .field_1f8 = 0x00000000,
        .field_1fc = 0x00000000,
        .field_200 = 0x00000000,
        .field_204 = 0x0000fff5,
        .field_208 = 0x00000000,
        .field_20c = 0x00000000,
    },
    .reserved_0x210_words = {
        0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000,
    },
    .meta_0x228 = {
        .packed_pair_0x228 = 0x00020002,
        .reference_0x22c = 0x00000002,
        .pixel_size_0x230 = 0x3fa00000, /* 1.25f */
        .opaque_words = {
            0x00000002, 0x40b8f5c3, 0x00000fc0, 0x00000bc8,
            0x000c000c, 0x00100010, 0x004003ff, 0x00400040,
            0x00000040, 0x00000003, 0x00022b00, 0x00000000,
            0x00000000, 0x00000000, 0x00023601, 0x00000000,
            0x00000000, 0x00000000, 0x00023502, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000,
        },
    },
    .meta_0x2b0 = {
        .words = {
            0x00000003, 0x00000003, 0x00000000,
            0x00000000, 0x00000001, 0x00021203,
        },
    },
    .driver_params = &kImx386DriverParams,
};

const void *imx386_get_driver_params(void) {
    return imx386_get_driver_params_typed();
}

const struct imx386_open_lib_layout *imx386_get_open_lib_layout(void) {
    return &kImx386OpenLib;
}

const struct sensor_driver_params_type *imx386_get_driver_params_typed(void) {
    return kImx386OpenLib.driver_params;
}

const struct sensor_output_reg_addr_v0 *imx386_get_output_reg_block(void) {
    return &kImx386OpenLib.output_reg_addr_0x1d4;
}

const struct msm_sensor_output_info_t *imx386_get_output_info_table(uint32_t *count) {
    if (count != NULL) {
        *count = ARRAY_SIZE(kImx386OutputInfo);
    }
    return kImx386OutputInfo;
}

int imx386_find_mode_by_resolution(uint16_t width, uint16_t height, uint32_t *mode_index) {
    return oxygen_sensor_find_mode_by_resolution(
        kImx386OutputInfo, ARRAY_SIZE(kImx386OutputInfo), width, height, mode_index);
}

const struct msm_sensor_output_info_t *imx386_get_mode_info(uint32_t mode_index) {
    return oxygen_sensor_get_mode_info(kImx386OutputInfo, ARRAY_SIZE(kImx386OutputInfo), mode_index);
}

int imx386_query_mode(uint32_t mode_index, struct oxygen_sensor_mode_query_result *out) {
    return oxygen_sensor_query_mode(
        kImx386OutputInfo, ARRAY_SIZE(kImx386OutputInfo), mode_index,
        imx386_get_output_reg_block(), imx386_get_meta_0x228_block(),
        imx386_get_pixel_size_microns(), out);
}

int imx386_query_output_info(uint32_t mode_index, struct oxygen_sensor_output_info_query_result *out) {
    return oxygen_sensor_query_output_info(
        kImx386OutputInfo, ARRAY_SIZE(kImx386OutputInfo), mode_index,
        imx386_get_output_reg_block(), imx386_get_meta_0x228_block(),
        imx386_get_pixel_size_microns(), out);
}

int imx386_query_output_info_by_resolution(uint16_t width, uint16_t height,
                                           struct oxygen_sensor_output_info_query_result *out) {
    uint32_t mode_index = 0;

    if (imx386_find_mode_by_resolution(width, height, &mode_index) != 0) {
        return -1;
    }
    return imx386_query_output_info(mode_index, out);
}

int imx386_sensor_get_output_info_exact(uint16_t width, uint16_t height,
                                        struct oxygen_sensor_get_output_info_result *out) {
    return oxygen_sensor_get_output_info_exact(
        kImx386OutputInfo, ARRAY_SIZE(kImx386OutputInfo),
        imx386_get_output_reg_block(), imx386_get_meta_0x228_block(),
        imx386_get_pixel_size_microns(), width, height, out);
}

int imx386_sensor_get_output_info_request(
    const struct oxygen_sensor_output_info_request *req,
    struct oxygen_sensor_get_output_info_result *out) {
    return oxygen_sensor_get_output_info_request(
        kImx386OutputInfo, ARRAY_SIZE(kImx386OutputInfo),
        imx386_get_output_reg_block(), imx386_get_meta_0x228_block(),
        imx386_get_pixel_size_microns(), req, out);
}

const struct sensor_meta_0x228_block_v0 *imx386_get_meta_0x228_block(void) {
    return &kImx386OpenLib.meta_0x228;
}

const struct sensor_meta_0x2b0_block_v0 *imx386_get_meta_0x2b0_block(void) {
    return &kImx386OpenLib.meta_0x2b0;
}

float imx386_get_pixel_size_microns(void) {
    union {
        uint32_t u32;
        float f32;
    } pixel_size = {
        .u32 = kImx386OpenLib.meta_0x228.pixel_size_0x230,
    };

    return pixel_size.f32;
}

__attribute__((visibility("default")))
void *sensor_open_lib(void) {
    return (void *)imx386_get_open_lib_layout();
}
