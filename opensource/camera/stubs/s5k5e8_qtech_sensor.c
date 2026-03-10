#include "qcom_camera_stub.h"

static const struct qcom_stub_sensor_open_lib kSensorOpenLib = {
    .sensor_name = "oxygen_s5k5e8_qtech",
    .driver_params = &kQcomStubEmptySensorDriverParams,
    .abi_revision = OXYGEN_QCOM_CAMERA_STUB_ABI_REVISION,
};

__attribute__((visibility("default")))
void *sensor_open_lib(void) {
    return (void *)&kSensorOpenLib;
}
