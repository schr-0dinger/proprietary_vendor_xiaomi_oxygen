#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../opensource/camera/imx386/imx386_sensor_layout.h"
#include "../../opensource/camera/ov12a/ov12a_sensor_layout.h"

static void expect_success(const char *label, int rc) {
  if (rc != 0) {
    fprintf(stderr, "%s failed unexpectedly: %d\n", label, rc);
    exit(1);
  }
}

static void expect_failure(const char *label, int rc) {
  if (rc == 0) {
    fprintf(stderr, "%s succeeded unexpectedly\n", label);
    exit(1);
  }
}

int main(void) {
  struct oxygen_sensor_get_output_info_result out = {0};
  struct oxygen_sensor_output_info_request req = {0};
  uint32_t mode_index = 0;

  expect_success("imx386_find_mode", imx386_find_mode_by_resolution(4032, 3016, &mode_index));
  expect_failure("imx386_find_mode_invalid", imx386_find_mode_by_resolution(1, 1, &mode_index));
  req.requested_width = 4032;
  req.requested_height = 3016;
  req.stream_mask = 0;
  expect_success("imx386_request", imx386_sensor_get_output_info_request(&req, &out));
  expect_failure("imx386_request_null", imx386_sensor_get_output_info_request(NULL, &out));

  expect_success("ov12a_find_mode", ov12a_find_mode_by_resolution(4096, 3072, &mode_index));
  expect_failure("ov12a_find_mode_invalid", ov12a_find_mode_by_resolution(1, 1, &mode_index));
  req.requested_width = 4096;
  req.requested_height = 3072;
  req.stream_mask = 0;
  expect_success("ov12a_request", ov12a_sensor_get_output_info_request(&req, &out));
  expect_failure("ov12a_request_null", ov12a_sensor_get_output_info_request(NULL, &out));

  puts("OK: shared sensor query contract holds for IMX386 and OV12A");
  return 0;
}
