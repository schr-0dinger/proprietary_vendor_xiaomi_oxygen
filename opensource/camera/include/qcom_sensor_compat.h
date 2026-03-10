#ifndef OXYGEN_QCOM_SENSOR_COMPAT_H
#define OXYGEN_QCOM_SENSOR_COMPAT_H

#include <stdint.h>

enum msm_camera_i2c_reg_addr_type {
    MSM_CAMERA_I2C_BYTE_ADDR = 1,
    MSM_CAMERA_I2C_WORD_ADDR,
    MSM_CAMERA_I2C_3B_ADDR,
    MSM_CAMERA_I2C_DWORD_ADDR,
};

struct msm_camera_i2c_reg_array {
    uint16_t reg_addr;
    uint16_t reg_data;
};

enum msm_camera_i2c_data_type {
    MSM_CAMERA_I2C_BYTE_DATA = 1,
    MSM_CAMERA_I2C_WORD_DATA,
    MSM_CAMERA_I2C_SET_BYTE_MASK,
    MSM_CAMERA_I2C_UNSET_BYTE_MASK,
    MSM_CAMERA_I2C_SET_WORD_MASK,
    MSM_CAMERA_I2C_UNSET_WORD_MASK,
    MSM_CAMERA_I2C_SET_BYTE_WRITE_MASK_DATA,
};

struct msm_camera_i2c_reg_setting {
    struct msm_camera_i2c_reg_array *reg_setting;
    uint16_t size;
    enum msm_camera_i2c_reg_addr_type addr_type;
    enum msm_camera_i2c_data_type data_type;
    uint16_t delay;
};

enum msm_sensor_resolution_t {
    MSM_SENSOR_RES_FULL,
    MSM_SENSOR_RES_QTR,
    MSM_SENSOR_RES_2,
    MSM_SENSOR_RES_3,
    MSM_SENSOR_RES_4,
    MSM_SENSOR_RES_5,
    MSM_SENSOR_RES_6,
    MSM_SENSOR_RES_7,
    MSM_SENSOR_INVALID_RES,
};

struct msm_sensor_output_info_t {
    uint16_t x_output;
    uint16_t y_output;
    uint16_t line_length_pclk;
    uint16_t frame_length_lines;
    uint32_t vt_pixel_clk;
    uint32_t op_pixel_clk;
    uint16_t binning_factor;
};

struct msm_sensor_exp_gain_info_t {
    uint16_t coarse_int_time_addr;
    uint16_t global_gain_addr;
    uint16_t vert_offset;
};

struct msm_sensor_output_reg_addr_t {
    uint16_t x_output;
    uint16_t y_output;
    uint16_t line_length_pclk;
    uint16_t frame_length_lines;
};

struct sensor_driver_params_type {
    struct msm_camera_i2c_reg_setting *init_settings;
    uint16_t init_settings_size;
    struct msm_camera_i2c_reg_setting *mode_settings;
    uint16_t mode_settings_size;
    struct msm_sensor_output_reg_addr_t *sensor_output_reg_addr;
    struct msm_camera_i2c_reg_setting *start_settings;
    struct msm_camera_i2c_reg_setting *stop_settings;
    struct msm_camera_i2c_reg_setting *groupon_settings;
    struct msm_camera_i2c_reg_setting *groupoff_settings;
    struct msm_sensor_exp_gain_info_t *sensor_exp_gain_info;
    struct msm_sensor_output_info_t *output_info;
};

#endif
