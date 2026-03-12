#include <stddef.h>
#include <stdint.h>

#include "ov12a_sensor_layout.h"
#include "qcom_sensor_compat.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

_Static_assert(offsetof(struct ov12a_open_lib_layout, raw_prefix_0x20_to_0x0f7) == 0x20,
    "unexpected ov12a sensor name size");
_Static_assert(offsetof(struct ov12a_open_lib_layout, resolution_triplets_0x0f8) == 0x0f8,
    "unexpected ov12a resolution triplet offset");
_Static_assert(offsetof(struct ov12a_open_lib_layout, family_0x70_header_words) == 0x1c0,
    "unexpected ov12a 0x1c0 block offset");
_Static_assert(offsetof(struct ov12a_open_lib_layout, output_reg_addr_0x1d4) == 0x1d4,
    "unexpected ov12a output_reg_addr offset");
_Static_assert(offsetof(struct ov12a_open_lib_layout, context_block_0x1e8) == 0x1e8,
    "unexpected ov12a 0x1e8 block offset");
_Static_assert(offsetof(struct ov12a_open_lib_layout, reserved_0x210_words) == 0x210,
    "unexpected ov12a 0x210 gap offset");
_Static_assert(offsetof(struct ov12a_open_lib_layout, meta_0x228) == 0x228,
    "unexpected ov12a 0x228 block offset");
_Static_assert(offsetof(struct ov12a_open_lib_layout, meta_0x2b0) == 0x2b0,
    "unexpected ov12a 0x2b0 block offset");
_Static_assert(sizeof(struct sensor_output_reg_addr_v0) == 0x10,
    "unexpected sensor_output_reg_addr_v0 size");
_Static_assert(sizeof(struct sensor_lib_context_block_v0) == 0x28,
    "unexpected sensor_lib_context_block_v0 size");
_Static_assert(sizeof(struct sensor_meta_0x228_block_v0) == 0x88,
    "unexpected sensor_meta_0x228_block_v0 size");
_Static_assert(sizeof(struct sensor_meta_0x2b0_block_v0) == 0x18,
    "unexpected sensor_meta_0x2b0_block_v0 size");

static const struct msm_sensor_output_reg_addr_t kOv12aOutputRegAddr = {
    .x_output = 0x3808,
    .y_output = 0x380a,
    .line_length_pclk = 0x380c,
    .frame_length_lines = 0x380e,
};

static const struct msm_sensor_exp_gain_info_t kOv12aExpGainInfo = {
    .coarse_int_time_addr = 0x3500,
    .global_gain_addr = 0x3508,
    .vert_offset = 0,
};

static const struct msm_sensor_output_info_t kOv12aOutputInfo[6] = {
    {.x_output = 4096, .y_output = 3072, .line_length_pclk = 1168, .frame_length_lines = 3302,
     .vt_pixel_clk = 108000000, .op_pixel_clk = 398400000, .binning_factor = 1},
    {.x_output = 2048, .y_output = 1536, .line_length_pclk = 1064, .frame_length_lines = 3346,
     .vt_pixel_clk = 106900000, .op_pixel_clk = 123360000, .binning_factor = 1},
    {.x_output = 4096, .y_output = 2304, .line_length_pclk = 1168, .frame_length_lines = 3080,
     .vt_pixel_clk = 108000000, .op_pixel_clk = 398400000, .binning_factor = 1},
    {.x_output = 3840, .y_output = 2160, .line_length_pclk = 1168, .frame_length_lines = 3080,
     .vt_pixel_clk = 108000000, .op_pixel_clk = 398400000, .binning_factor = 1},
    {.x_output = 1920, .y_output = 1080, .line_length_pclk = 1064, .frame_length_lines = 3346,
     .vt_pixel_clk = 107400000, .op_pixel_clk = 233600000, .binning_factor = 1},
    {.x_output = 1280, .y_output = 720, .line_length_pclk = 1064, .frame_length_lines = 844,
     .vt_pixel_clk = 107800000, .op_pixel_clk = 233600000, .binning_factor = 1},
};

static const struct sensor_driver_params_type kOv12aDriverParams = {
    .init_settings = NULL,
    .init_settings_size = 0,
    .mode_settings = NULL,
    .mode_settings_size = 0,
    .sensor_output_reg_addr = (struct msm_sensor_output_reg_addr_t *)&kOv12aOutputRegAddr,
    .start_settings = NULL,
    .stop_settings = NULL,
    .groupon_settings = NULL,
    .groupoff_settings = NULL,
    .sensor_exp_gain_info = (struct msm_sensor_exp_gain_info_t *)&kOv12aExpGainInfo,
    .output_info = (struct msm_sensor_output_info_t *)kOv12aOutputInfo,
};

static const struct ov12a_open_lib_layout kOv12aOpenLibSunny = {
    .sensor_name = "oxygen_ov12a_sunny",
    .raw_prefix_0x20_to_0x0f7 = {
        0x00000020, 0x00000001, 0x00000002, 0x00000000,
        0x1241d6eb, 0x00000000, 0x00000001, 0x00000000,
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
        0x00000006, 0x00000000, 0x00000000, 0x00000000,
        0x00000001, 0x00000001, 0x00000000, 0x00000000,
        0x00000001, 0x00000002, 0x00000001, 0x00000000,
        0x00000000, 0x00000002, 0x00000000, 0x00000000,
        0x00000001, 0x00000002, 0x00000002, 0x00000000,
        0x00000001, 0x00000000,
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
        0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000000,
    },
    .output_reg_addr_0x1d4 = {
        .x_output = 0x3808,
        .y_output = 0x380a,
        .line_length_pclk = 0x380c,
        .frame_length_lines = 0x380e,
        .coarse_int_time = 0x3500,
        .global_gain = 0x3508,
    },
    .reserved_0x1e4 = 0x00000000,
    .context_block_0x1e8 = {
        .field_1e8 = 0x0000,
        .field_1ea = 0x0008,
        .field_1ec = 0x3f800000,
        .field_1f0 = 0x41780000,
        .field_1f4 = 0x41780000,
        .field_1f8 = 0x00000000,
        .field_1fc = 0x00000000,
        .field_200 = 0x00000000,
        .field_204 = 0x00007ff7,
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
        .pixel_size_0x230 = 0x3f9ef9db,
        .opaque_words = {
            0x00000002, 0x3faa3d71, 0x00001240, 0x00000db0,
            0x00080008, 0x00080008, 0x004003ff, 0x00400040,
            0x00000040, 0x00000001, 0x00022b00, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000,
        },
    },
    .meta_0x2b0 = {.words = {0, 0, 0, 0, 0, 0}},
    .driver_params = &kOv12aDriverParams,
};

static const struct ov12a_open_lib_layout kOv12aOpenLibOfilm = {
    .sensor_name = "oxygen_ov12a_ofilm",
    .raw_prefix_0x20_to_0x0f7 = {
        0x00000020, 0x00000001, 0x00000002, 0x00000000,
        0x1241d6eb, 0x00000000, 0x00000001, 0x00000000,
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
        0x00000006, 0x00000000, 0x00000000, 0x00000000,
        0x00000001, 0x00000001, 0x00000000, 0x00000000,
        0x00000001, 0x00000002, 0x00000001, 0x00000000,
        0x00000000, 0x00000002, 0x00000000, 0x00000000,
        0x00000001, 0x00000002, 0x00000002, 0x00000000,
        0x00000001, 0x00000000,
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
        0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000000,
    },
    .output_reg_addr_0x1d4 = {
        .x_output = 0x3808,
        .y_output = 0x380a,
        .line_length_pclk = 0x380c,
        .frame_length_lines = 0x380e,
        .coarse_int_time = 0x3500,
        .global_gain = 0x3508,
    },
    .reserved_0x1e4 = 0x00000000,
    .context_block_0x1e8 = {
        .field_1e8 = 0x0000,
        .field_1ea = 0x0008,
        .field_1ec = 0x3f800000,
        .field_1f0 = 0x41780000,
        .field_1f4 = 0x41780000,
        .field_1f8 = 0x00000000,
        .field_1fc = 0x00000000,
        .field_200 = 0x00000000,
        .field_204 = 0x00007ff7,
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
        .pixel_size_0x230 = 0x3f9ef9db,
        .opaque_words = {
            0x00000002, 0x3faa3d71, 0x00001240, 0x00000db0,
            0x00080008, 0x00080008, 0x004003ff, 0x00400040,
            0x00000040, 0x00000001, 0x00022b00, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000,
        },
    },
    .meta_0x2b0 = {.words = {0, 0, 0, 0, 0, 0}},
    .driver_params = &kOv12aDriverParams,
};

const void *ov12a_get_driver_params(void) {
    return ov12a_get_driver_params_typed();
}

const struct ov12a_open_lib_layout *ov12a_get_open_lib_layout(enum ov12a_module_vendor vendor) {
    return vendor == OV12A_VENDOR_OFILM ? &kOv12aOpenLibOfilm : &kOv12aOpenLibSunny;
}

const struct sensor_driver_params_type *ov12a_get_driver_params_typed(void) {
    return kOv12aOpenLibSunny.driver_params;
}

const struct sensor_output_reg_addr_v0 *ov12a_get_output_reg_block(void) {
    return &kOv12aOpenLibSunny.output_reg_addr_0x1d4;
}

const struct msm_sensor_output_info_t *ov12a_get_output_info_table(uint32_t *count) {
    if (count != NULL) {
        *count = ARRAY_SIZE(kOv12aOutputInfo);
    }
    return kOv12aOutputInfo;
}

int ov12a_find_mode_by_resolution(uint16_t width, uint16_t height, uint32_t *mode_index) {
    return oxygen_sensor_find_mode_by_resolution(
        kOv12aOutputInfo, ARRAY_SIZE(kOv12aOutputInfo), width, height, mode_index);
}

const struct msm_sensor_output_info_t *ov12a_get_mode_info(uint32_t mode_index) {
    return oxygen_sensor_get_mode_info(kOv12aOutputInfo, ARRAY_SIZE(kOv12aOutputInfo), mode_index);
}

int ov12a_query_mode(uint32_t mode_index, struct oxygen_sensor_mode_query_result *out) {
    return oxygen_sensor_query_mode(
        kOv12aOutputInfo, ARRAY_SIZE(kOv12aOutputInfo), mode_index,
        ov12a_get_output_reg_block(), ov12a_get_meta_0x228_block(),
        ov12a_get_pixel_size_microns(), out);
}

int ov12a_query_output_info(uint32_t mode_index, struct oxygen_sensor_output_info_query_result *out) {
    return oxygen_sensor_query_output_info(
        kOv12aOutputInfo, ARRAY_SIZE(kOv12aOutputInfo), mode_index,
        ov12a_get_output_reg_block(), ov12a_get_meta_0x228_block(),
        ov12a_get_pixel_size_microns(), out);
}

int ov12a_query_output_info_by_resolution(uint16_t width, uint16_t height,
                                          struct oxygen_sensor_output_info_query_result *out) {
    uint32_t mode_index = 0;

    if (ov12a_find_mode_by_resolution(width, height, &mode_index) != 0) {
        return -1;
    }
    return ov12a_query_output_info(mode_index, out);
}

int ov12a_sensor_get_output_info_exact(uint16_t width, uint16_t height,
                                       struct oxygen_sensor_get_output_info_result *out) {
    return oxygen_sensor_get_output_info_exact(
        kOv12aOutputInfo, ARRAY_SIZE(kOv12aOutputInfo),
        ov12a_get_output_reg_block(), ov12a_get_meta_0x228_block(),
        ov12a_get_pixel_size_microns(), width, height, out);
}

int ov12a_sensor_get_output_info_request(
    const struct oxygen_sensor_output_info_request *req,
    struct oxygen_sensor_get_output_info_result *out) {
    return oxygen_sensor_get_output_info_request(
        kOv12aOutputInfo, ARRAY_SIZE(kOv12aOutputInfo),
        ov12a_get_output_reg_block(), ov12a_get_meta_0x228_block(),
        ov12a_get_pixel_size_microns(), req, out);
}

const struct sensor_meta_0x228_block_v0 *ov12a_get_meta_0x228_block(void) {
    return &kOv12aOpenLibSunny.meta_0x228;
}

const struct sensor_meta_0x2b0_block_v0 *ov12a_get_meta_0x2b0_block(void) {
    return &kOv12aOpenLibSunny.meta_0x2b0;
}

float ov12a_get_pixel_size_microns(void) {
    union {
        uint32_t u32;
        float f32;
    } pixel_size = {
        .u32 = kOv12aOpenLibSunny.meta_0x228.pixel_size_0x230,
    };

    return pixel_size.f32;
}

void *ov12a_sensor_open_lib_variant(enum ov12a_module_vendor vendor) {
    return (void *)ov12a_get_open_lib_layout(vendor);
}
