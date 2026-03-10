#ifndef OXYGEN_QCOM_CAMERA_STUB_H
#define OXYGEN_QCOM_CAMERA_STUB_H

#include <stdint.h>

#include "qcom_sensor_compat.h"

#define OXYGEN_QCOM_CAMERA_STUB_ABI_REVISION 1u

struct qcom_stub_sensor_open_lib {
    char sensor_name[32];
    const struct sensor_driver_params_type *driver_params;
    uint32_t abi_revision;
    uint32_t reserved[7];
};

struct qcom_stub_named_module {
    char name[64];
    char kind[16];
    uint32_t abi_revision;
    uint32_t reserved[5];
};

static const struct sensor_driver_params_type kQcomStubEmptySensorDriverParams = {0};

#endif
