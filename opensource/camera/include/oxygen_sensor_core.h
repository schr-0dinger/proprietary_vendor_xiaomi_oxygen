#ifndef OXYGEN_SENSOR_CORE_H
#define OXYGEN_SENSOR_CORE_H

#include <stddef.h>
#include <stdint.h>

#include "../../../RE/camera_runtime_structs.h"
#include "qcom_sensor_compat.h"

struct oxygen_sensor_mode_query_result {
    const struct msm_sensor_output_info_t *output_info;
    const struct sensor_output_reg_addr_v0 *output_reg_addr;
    const struct sensor_meta_0x228_block_v0 *meta_0x228;
    float pixel_size_microns;
};

struct oxygen_sensor_output_info_query_result {
    uint16_t x_output;
    uint16_t y_output;
    uint16_t line_length_pclk;
    uint16_t frame_length_lines;
    uint32_t vt_pixel_clk;
    uint32_t op_pixel_clk;
    uint16_t binning_factor;
    uint32_t packed_pair_0x228;
    uint32_t reference_0x22c;
    float pixel_size_microns;
};

struct oxygen_sensor_get_output_info_result {
    uint32_t mode_index;
    struct oxygen_sensor_output_info_query_result output;
};

struct oxygen_sensor_output_info_request {
    uint16_t requested_width;
    uint16_t requested_height;
    uint32_t stream_mask;
};

static inline const struct msm_sensor_output_info_t *oxygen_sensor_get_mode_info(
    const struct msm_sensor_output_info_t *table, uint32_t count, uint32_t mode_index) {
    if (table == NULL || mode_index >= count) {
        return NULL;
    }
    return &table[mode_index];
}

static inline int oxygen_sensor_find_mode_by_resolution(
    const struct msm_sensor_output_info_t *table, uint32_t count,
    uint16_t width, uint16_t height, uint32_t *mode_index) {
    uint32_t idx;

    if (table == NULL || mode_index == NULL) {
        return -1;
    }

    for (idx = 0; idx < count; ++idx) {
        const struct msm_sensor_output_info_t *info = &table[idx];
        if (info->x_output == width && info->y_output == height) {
            *mode_index = idx;
            return 0;
        }
    }

    *mode_index = 0;
    return -1;
}

static inline int oxygen_sensor_query_mode(
    const struct msm_sensor_output_info_t *table, uint32_t count, uint32_t mode_index,
    const struct sensor_output_reg_addr_v0 *output_reg_addr,
    const struct sensor_meta_0x228_block_v0 *meta_0x228, float pixel_size_microns,
    struct oxygen_sensor_mode_query_result *out) {
    if (out == NULL) {
        return -1;
    }

    out->output_info = oxygen_sensor_get_mode_info(table, count, mode_index);
    if (out->output_info == NULL) {
        out->output_reg_addr = NULL;
        out->meta_0x228 = NULL;
        out->pixel_size_microns = 0.0f;
        return -1;
    }

    out->output_reg_addr = output_reg_addr;
    out->meta_0x228 = meta_0x228;
    out->pixel_size_microns = pixel_size_microns;
    return 0;
}

static inline int oxygen_sensor_query_output_info(
    const struct msm_sensor_output_info_t *table, uint32_t count, uint32_t mode_index,
    const struct sensor_output_reg_addr_v0 *output_reg_addr,
    const struct sensor_meta_0x228_block_v0 *meta_0x228, float pixel_size_microns,
    struct oxygen_sensor_output_info_query_result *out) {
    struct oxygen_sensor_mode_query_result mode = {0};

    if (out == NULL) {
        return -1;
    }
    if (oxygen_sensor_query_mode(
            table, count, mode_index, output_reg_addr, meta_0x228,
            pixel_size_microns, &mode) != 0) {
        return -1;
    }

    out->x_output = mode.output_info->x_output;
    out->y_output = mode.output_info->y_output;
    out->line_length_pclk = mode.output_info->line_length_pclk;
    out->frame_length_lines = mode.output_info->frame_length_lines;
    out->vt_pixel_clk = mode.output_info->vt_pixel_clk;
    out->op_pixel_clk = mode.output_info->op_pixel_clk;
    out->binning_factor = mode.output_info->binning_factor;
    out->packed_pair_0x228 = mode.meta_0x228->packed_pair_0x228;
    out->reference_0x22c = mode.meta_0x228->reference_0x22c;
    out->pixel_size_microns = mode.pixel_size_microns;
    return 0;
}

static inline int oxygen_sensor_get_output_info_exact(
    const struct msm_sensor_output_info_t *table, uint32_t count,
    const struct sensor_output_reg_addr_v0 *output_reg_addr,
    const struct sensor_meta_0x228_block_v0 *meta_0x228, float pixel_size_microns,
    uint16_t width, uint16_t height, struct oxygen_sensor_get_output_info_result *out) {
    uint32_t mode_index = 0;

    if (out == NULL) {
        return -1;
    }
    if (oxygen_sensor_find_mode_by_resolution(table, count, width, height, &mode_index) != 0) {
        return -1;
    }
    if (oxygen_sensor_query_output_info(
            table, count, mode_index, output_reg_addr, meta_0x228,
            pixel_size_microns, &out->output) != 0) {
        return -1;
    }

    out->mode_index = mode_index;
    return 0;
}

static inline int oxygen_sensor_get_output_info_request(
    const struct msm_sensor_output_info_t *table, uint32_t count,
    const struct sensor_output_reg_addr_v0 *output_reg_addr,
    const struct sensor_meta_0x228_block_v0 *meta_0x228, float pixel_size_microns,
    const struct oxygen_sensor_output_info_request *req,
    struct oxygen_sensor_get_output_info_result *out) {
    if (req == NULL || out == NULL) {
        return -1;
    }

    /*
     * Conservative shared policy: exact resolution match only.
     * The proprietary chooser also uses stream-mask / policy inputs, but that
     * is not generalized here until runtime or a second family proves it.
     */
    (void)req->stream_mask;
    return oxygen_sensor_get_output_info_exact(
        table, count, output_reg_addr, meta_0x228, pixel_size_microns,
        req->requested_width, req->requested_height, out);
}

#endif /* OXYGEN_SENSOR_CORE_H */
