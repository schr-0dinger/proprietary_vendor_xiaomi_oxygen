#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../opensource/camera/imx386/imx386_sensor_layout.h"

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
  const struct msm_sensor_output_info_t *table = imx386_get_output_info_table(&count);

  if (table == NULL) {
    fprintf(stderr, "output info table is NULL\n");
    return 1;
  }

  expect_int("output_info_count", count, 6);

  if (imx386_query_mode(0, &mode) != 0) {
    fprintf(stderr, "imx386_query_mode(0) failed\n");
    return 1;
  }

  expect_int("mode0_x_output", mode.output_info->x_output, 4032);
  expect_int("mode0_y_output", mode.output_info->y_output, 3016);
  expect_int("mode0_line_length", mode.output_info->line_length_pclk, 4296);
  expect_int("mode0_frame_length", mode.output_info->frame_length_lines, 3070);
  expect_int("mode0_vt_clk", mode.output_info->vt_pixel_clk, 388000000u);
  expect_int("mode0_op_clk", mode.output_info->op_pixel_clk, 398400000u);
  expect_u32_hex("meta_0x228", mode.meta_0x228->packed_pair_0x228, 0x00020002u);
  expect_u32_hex("meta_0x22c", mode.meta_0x228->reference_0x22c, 0x00000002u);
  expect_float("pixel_size", mode.pixel_size_microns, 1.25f);
  expect_int("reg_x_output", mode.output_reg_addr->x_output, 0x034c);
  expect_int("reg_y_output", mode.output_reg_addr->y_output, 0x034e);

  if (imx386_query_output_info(5, &out) != 0) {
    fprintf(stderr, "imx386_query_output_info(5) failed\n");
    return 1;
  }

  expect_int("mode5_x_output", out.x_output, 1920);
  expect_int("mode5_y_output", out.y_output, 1080);
  expect_int("mode5_line_length", out.line_length_pclk, 2256);
  expect_int("mode5_frame_length", out.frame_length_lines, 1174);
  expect_int("mode5_vt_clk", out.vt_pixel_clk, 318000000u);
  expect_int("mode5_op_clk", out.op_pixel_clk, 381600000u);
  expect_int("mode5_binning", out.binning_factor, 1);
  expect_u32_hex("out_meta_0x228", out.packed_pair_0x228, 0x00020002u);
  expect_u32_hex("out_meta_0x22c", out.reference_0x22c, 0x00000002u);
  expect_float("out_pixel_size", out.pixel_size_microns, 1.25f);

  if (imx386_find_mode_by_resolution(3840, 2160, &mode_index) != 0) {
    fprintf(stderr, "imx386_find_mode_by_resolution(3840,2160) failed\n");
    return 1;
  }
  expect_int("mode_index_3840x2160", mode_index, 3);

  if (imx386_query_output_info_by_resolution(3840, 2160, &out) != 0) {
    fprintf(stderr, "imx386_query_output_info_by_resolution(3840,2160) failed\n");
    return 1;
  }
  expect_int("res_query_x_output", out.x_output, 3840);
  expect_int("res_query_y_output", out.y_output, 2160);
  expect_int("res_query_op_clk", out.op_pixel_clk, 356800000u);

  if (imx386_sensor_get_output_info_exact(3840, 2160, &sensor_out) != 0) {
    fprintf(stderr, "imx386_sensor_get_output_info_exact(3840,2160) failed\n");
    return 1;
  }
  expect_int("sensor_get_output_info_mode_4k", sensor_out.mode_index, 3);
  expect_int("sensor_get_output_info_4k_op_clk", sensor_out.output.op_pixel_clk, 356800000u);

  if (imx386_sensor_get_output_info_exact(1920, 1080, &sensor_out) != 0) {
    fprintf(stderr, "imx386_sensor_get_output_info_exact(1920,1080) failed\n");
    return 1;
  }
  /*
   * Current shim policy is "first exact match wins". That means 1920x1080
   * currently resolves to mode 4, not mode 5.
   */
  expect_int("sensor_get_output_info_mode_1080p", sensor_out.mode_index, 4);
  expect_int("sensor_get_output_info_1080p_op_clk", sensor_out.output.op_pixel_clk, 137600000u);

  req.requested_width = 4032;
  req.requested_height = 3016;
  req.stream_mask = 0x1;
  if (imx386_sensor_get_output_info_request(&req, &sensor_out) != 0) {
    fprintf(stderr, "imx386_sensor_get_output_info_request(4032x3016) failed\n");
    return 1;
  }
  expect_int("sensor_get_output_info_req_mode", sensor_out.mode_index, 0);
  expect_int("sensor_get_output_info_req_op_clk", sensor_out.output.op_pixel_clk, 398400000u);

  if (imx386_query_mode(99, &mode) == 0) {
    fprintf(stderr, "imx386_query_mode(99) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_query_output_info(99, &out) == 0) {
    fprintf(stderr, "imx386_query_output_info(99) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_query_mode(0, NULL) == 0) {
    fprintf(stderr, "imx386_query_mode(NULL) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_query_output_info(0, NULL) == 0) {
    fprintf(stderr, "imx386_query_output_info(NULL) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_find_mode_by_resolution(1, 1, &mode_index) == 0) {
    fprintf(stderr, "imx386_find_mode_by_resolution(1,1) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_query_output_info_by_resolution(1, 1, &out) == 0) {
    fprintf(stderr, "imx386_query_output_info_by_resolution(1,1) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_sensor_get_output_info_exact(1, 1, &sensor_out) == 0) {
    fprintf(stderr, "imx386_sensor_get_output_info_exact(1,1) unexpectedly succeeded\n");
    return 1;
  }
  if (imx386_sensor_get_output_info_request(NULL, &sensor_out) == 0) {
    fprintf(stderr, "imx386_sensor_get_output_info_request(NULL) unexpectedly succeeded\n");
    return 1;
  }

  puts("OK: imx386 query helpers match expected proprietary values");
  return 0;
}
