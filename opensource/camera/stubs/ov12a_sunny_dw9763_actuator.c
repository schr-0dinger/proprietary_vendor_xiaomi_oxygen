#include "qcom_camera_stub.h"

static const struct qcom_stub_named_module kActuatorOpenLib = {
    .name = "oxygen_ov12a_sunny_dw9763",
    .kind = "actuator",
    .abi_revision = OXYGEN_QCOM_CAMERA_STUB_ABI_REVISION,
};

__attribute__((visibility("default")))
void *actuator_driver_open_lib(void) {
    return (void *)&kActuatorOpenLib;
}
