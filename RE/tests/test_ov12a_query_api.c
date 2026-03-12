#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../opensource/camera/ov12a/ov12a_sensor_layout.h"

static void expect_int(const char *label, uint32_t got, uint32_t want) {
  if (got != want) {
    fprintf(stderr, "%s mismatch: got %u want %u\n", label, got, want);
    exit(1);
  }
}

static void expect_u32_hex(const char *label, uint32_t got, uint32_t want) {
  if (got != want) {
    fprintf(stderr, "%s mismatch: got 0x%08x want 0x%08x\n", label, got, want);
    exit(1);
  }
}

static void expect_float(const char *label, float got, float want) {
  if (fabsf(got - want) > 0.0001f) {
    fprintf(stderr, "%s mismatch: got %.6f want %.6f\n", label, got, want);
    exit(1);
  }
}

int main(void) {
  struct oxygen_sensor_mode_query_result mode = {0};
  struct oxygen_sensor_output_info_query_result out = {0};
  struct oxygen_sensor_get_output_info_result sensor_out = {0};
  struct oxygen_sensor_output_info_request req = {0};
  uint32_t count = 0;
  uint32_t mode_index = 0;
  const struct msm_sensor_output_info_t *table = ov12a_get_output_info_table(&count);
  const struct ov12a_open_lib_layout *sunny = ov12a_get_open_lib_layout(OV12A_VENDOR_SUNNY);
  const struct ov12a_open_lib_layout *ofilm = ov12a_get_open_lib_layout(OV12A_VENDOR_OFILM);

  if (table == NULL || sunny == NULL || ofilm == NULL) {
    fprintf(stderr, "ov12a layout/table lookup failed\n");
    return 1;
  }

  expect_int("output_info_count", count, 6);
  expect_int("sunny_triplets_0", sunny->resolution_triplets_0x0f8[0], 6);
  expect_int("ofilm_triplets_0", ofilm->resolution_triplets_0x0f8[0], 6);
  expect_u32_hex("sunny_meta_0x228", sunny->meta_0x228.packed_pair_0x228, 0x00020002u);
  expect_u32_hex("ofilm_meta_0x228", ofilm->meta_0x228.packed_pair_0x228, 0x00020002u);

  if (ov12a_query_mode(0, &mode) != 0) {
    fprintf(stderr, "ov12a_query_mode(0) failed\n");
    return 1;
  }

  expect_int("mode0_x_output", mode.output_info->x_output, 4096);
  expect_int("mode0_y_output", mode.output_info->y_output, 3072);
  expect_int("mode0_line_length", mode.output_info->line_length_pclk, 1168);
  expect_int("mode0_frame_length", mode.output_info->frame_length_lines, 3302);
  expect_int("mode0_vt_clk", mode.output_info->vt_pixel_clk, 108000000u);
  expect_int("mode0_op_clk", mode.output_info->op_pixel_clk, 398400000u);
  expect_int("reg_x_output", mode.output_reg_addr->x_output, 0x3808);
  expect_int("reg_y_output", mode.output_reg_addr->y_output, 0x380a);
  expect_u32_hex("meta_0x22c", mode.meta_0x228->reference_0x22c, 0x00000002u);
  expect_float("pixel_size", mode.pixel_size_microns, 1.242f);

  if (ov12a_query_output_info(5, &out) != 0) {
    fprintf(stderr, "ov12a_query_output_info(5) failed\n");
    return 1;
  }

  expect_int("mode5_x_output", out.x_output, 1280);
  expect_int("mode5_y_output", out.y_output, 720);
  expect_int("mode5_line_length", out.line_length_pclk, 1064);
  expect_int("mode5_frame_length", out.frame_length_lines, 844);
  expect_int("mode5_vt_clk", out.vt_pixel_clk, 107800000u);
  expect_int("mode5_op_clk", out.op_pixel_clk, 233600000u);
  expect_int("mode5_binning", out.binning_factor, 1);
  expect_float("mode5_pixel_size", out.pixel_size_microns, 1.242f);

  if (ov12a_find_mode_by_resolution(3840, 2160, &mode_index) != 0) {
    fprintf(stderr, "ov12a_find_mode_by_resolution(3840,2160) failed\n");
    return 1;
  }
  expect_int("mode_index_3840x2160", mode_index, 3);

  if (ov12a_query_output_info_by_resolution(3840, 2160, &out) != 0) {
    fprintf(stderr, "ov12a_query_output_info_by_resolution(3840,2160) failed\n");
    return 1;
  }
  expect_int("res_query_x_output", out.x_output, 3840);
  expect_int("res_query_y_output", out.y_output, 2160);
  expect_int("res_query_op_clk", out.op_pixel_clk, 398400000u);

  if (ov12a_sensor_get_output_info_exact(2048, 1536, &sensor_out) != 0) {
    fprintf(stderr, "ov12a_sensor_get_output_info_exact(2048,1536) failed\n");
    return 1;
  }
  expect_int("sensor_get_output_info_mode", sensor_out.mode_index, 1);
  expect_int("sensor_get_output_info_op_clk", sensor_out.output.op_pixel_clk, 123360000u);

  req.requested_width = 1920;
  req.requested_height = 1080;
  req.stream_mask = 0x2;
  if (ov12a_sensor_get_output_info_request(&req, &sensor_out) != 0) {
    fprintf(stderr, "ov12a_sensor_get_output_info_request(1920x1080) failed\n");
    return 1;
  }
  expect_int("sensor_get_output_info_req_mode", sensor_out.mode_index, 4);
  expect_int("sensor_get_output_info_req_op_clk", sensor_out.output.op_pixel_clk, 233600000u);

  if (ov12a_query_mode(99, &mode) == 0) {
    fprintf(stderr, "ov12a_query_mode(99) unexpectedly succeeded\n");
    return 1;
  }
  if (ov12a_query_output_info(99, &out) == 0) {
    fprintf(stderr, "ov12a_query_output_info(99) unexpectedly succeeded\n");
    return 1;
  }
  if (ov12a_query_mode(0, NULL) == 0) {
    fprintf(stderr, "ov12a_query_mode(NULL) unexpectedly succeeded\n");
    return 1;
  }
  if (ov12a_find_mode_by_resolution(1, 1, &mode_index) == 0) {
    fprintf(stderr, "ov12a_find_mode_by_resolution(1,1) unexpectedly succeeded\n");
    return 1;
  }
  if (ov12a_sensor_get_output_info_request(NULL, &sensor_out) == 0) {
    fprintf(stderr, "ov12a_sensor_get_output_info_request(NULL) unexpectedly succeeded\n");
    return 1;
  }

  puts("OK: ov12a query helpers match expected proprietary values");
  return 0;
}
