#include "ov12a_sensor_layout.h"

__attribute__((visibility("default")))
void *sensor_open_lib(void) {
    return ov12a_sensor_open_lib_variant(OV12A_VENDOR_OFILM);
}
