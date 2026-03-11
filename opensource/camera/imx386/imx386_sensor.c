#include <stddef.h>
#include <stdint.h>

#include "imx386_reg_data.h"
#include "qcom_sensor_compat.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/*
 * This is a source skeleton for libmmcamera_oxygen_imx386_sunny.so.
 *
 * It preserves the exported symbol name and the recovered inline data offsets
 * from RE/libmmcamera_oxygen_imx386_sunny.so, but it is not a runtime-ready
 * replacement for the proprietary blob yet.
 */

struct imx386_open_lib_layout {
    char sensor_name[32];
    uint32_t raw_prefix_0x20_to_0x0f7[54];
    uint32_t resolution_triplets_0x0f8[22];
    uint32_t reserved_0x150_words[28];
    uint32_t meta_0x1c0_words[10];
    uint32_t meta_0x1e8_words[16];
    uint32_t meta_0x228_words[34];
    uint32_t meta_0x2b0_words[6];
    const struct sensor_driver_params_type *driver_params;
};

_Static_assert(offsetof(struct imx386_open_lib_layout, raw_prefix_0x20_to_0x0f7) == 0x20,
    "unexpected imx386 sensor name size");
_Static_assert(offsetof(struct imx386_open_lib_layout, resolution_triplets_0x0f8) == 0x0f8,
    "unexpected imx386 resolution triplet offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, meta_0x1c0_words) == 0x1c0,
    "unexpected imx386 0x61c8 block offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, meta_0x1e8_words) == 0x1e8,
    "unexpected imx386 0x61f0 block offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, meta_0x228_words) == 0x228,
    "unexpected imx386 0x6230 block offset");
_Static_assert(offsetof(struct imx386_open_lib_layout, meta_0x2b0_words) == 0x2b0,
    "unexpected imx386 0x62b8 block offset");

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
 * Only the first entry has partially recovered geometry today.
 * The remaining output_info entries stay zero until the packed 0x6230 region is
 * mapped field-by-field.
 */
static const struct msm_sensor_output_info_t kImx386OutputInfo[6] = {
    {
        .x_output = 4032,
        .y_output = 3016,
        .line_length_pclk = 0,
        .frame_length_lines = 0,
        .vt_pixel_clk = 0,
        .op_pixel_clk = 0,
        .binning_factor = 1,
    },
    {0},
    {0},
    {0},
    {0},
    {0},
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
    .meta_0x1c0_words = {
        0x00000001, 0x00000000, 0x00000001, 0x00000001,
        0x00000003, 0x034e034c, 0x03400342, 0x00000202,
        0x00000204, 0x00000000,
    },
    .meta_0x1e8_words = {
        0x00000000, 0x0000000a, 0x3f800000, 0x41800000,
        0x41800000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x0000fff5, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
    },
    .meta_0x228_words = {
        0x00020002, 0x00000002, 0x3fa00000, 0x00000002,
        0x40b8f5c3, 0x00000fc0, 0x00000bc8, /* active 4032x3016 */
        0x000c000c,
        0x00100010, 0x004003ff, 0x00400040, 0x00000040,
        0x00000003, 0x00022b00, 0x00000000, 0x00000000,
        0x00000000, 0x00023601, 0x00000000, 0x00000000,
        0x00000000, 0x00023502, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000,
    },
    .meta_0x2b0_words = {
        0x00000003, 0x00000003, 0x00000000,
        0x00000000, 0x00000001, 0x00021203,
    },
    .driver_params = &kImx386DriverParams,
};

const void *imx386_get_driver_params(void) {
    return &kImx386DriverParams;
}

__attribute__((visibility("default")))
void *sensor_open_lib(void) {
    return (void *)&kImx386OpenLib;
}
