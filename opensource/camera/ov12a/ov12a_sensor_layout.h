#ifndef OXYGEN_OV12A_SENSOR_LAYOUT_H
#define OXYGEN_OV12A_SENSOR_LAYOUT_H

#include <stdint.h>

#include "../../../RE/camera_runtime_structs.h"
#include "../include/oxygen_sensor_core.h"
#include "qcom_sensor_compat.h"

enum ov12a_module_vendor {
    OV12A_VENDOR_SUNNY = 0,
    OV12A_VENDOR_OFILM = 1,
};

struct ov12a_open_lib_layout {
    char sensor_name[32];
    uint32_t raw_prefix_0x20_to_0x0f7[54];
    uint32_t resolution_triplets_0x0f8[22];
    uint32_t reserved_0x150_words[28];
    uint32_t family_0x70_header_words[5];
    struct sensor_output_reg_addr_v0 output_reg_addr_0x1d4;
    uint32_t reserved_0x1e4;
    struct sensor_lib_context_block_v0 context_block_0x1e8;
    uint32_t reserved_0x210_words[6];
    struct sensor_meta_0x228_block_v0 meta_0x228;
    struct sensor_meta_0x2b0_block_v0 meta_0x2b0;
    const struct sensor_driver_params_type *driver_params;
};

const struct ov12a_open_lib_layout *ov12a_get_open_lib_layout(enum ov12a_module_vendor vendor);
const struct sensor_driver_params_type *ov12a_get_driver_params_typed(void);
const struct sensor_output_reg_addr_v0 *ov12a_get_output_reg_block(void);
const struct msm_sensor_output_info_t *ov12a_get_output_info_table(uint32_t *count);
int ov12a_find_mode_by_resolution(uint16_t width, uint16_t height, uint32_t *mode_index);
const struct msm_sensor_output_info_t *ov12a_get_mode_info(uint32_t mode_index);
int ov12a_query_mode(uint32_t mode_index, struct oxygen_sensor_mode_query_result *out);
int ov12a_query_output_info(uint32_t mode_index, struct oxygen_sensor_output_info_query_result *out);
int ov12a_query_output_info_by_resolution(uint16_t width, uint16_t height,
                                          struct oxygen_sensor_output_info_query_result *out);
int ov12a_sensor_get_output_info_exact(uint16_t width, uint16_t height,
                                       struct oxygen_sensor_get_output_info_result *out);
int ov12a_sensor_get_output_info_request(
    const struct oxygen_sensor_output_info_request *req,
    struct oxygen_sensor_get_output_info_result *out);
const struct sensor_meta_0x228_block_v0 *ov12a_get_meta_0x228_block(void);
const struct sensor_meta_0x2b0_block_v0 *ov12a_get_meta_0x2b0_block(void);
float ov12a_get_pixel_size_microns(void);
void *ov12a_sensor_open_lib_variant(enum ov12a_module_vendor vendor);

#endif /* OXYGEN_OV12A_SENSOR_LAYOUT_H */
