#include "sensors_backend.h"

static const struct oxygen_sensors_backend kBackend = {
    .name = "downstream",
    .transport = "ssc-dsp-placeholder",
    .notes = "matches the Qualcomm downstream sensors stack shape",
};

const struct oxygen_sensors_backend *oxygen_sensors_backend_downstream(void) {
    return &kBackend;
}
