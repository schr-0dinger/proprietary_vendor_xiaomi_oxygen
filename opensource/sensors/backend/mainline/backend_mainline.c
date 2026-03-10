#include "sensors_backend.h"

static const struct oxygen_sensors_backend kBackend = {
    .name = "mainline",
    .transport = "iio-userspace-placeholder",
    .notes = "future userspace path for msm8953-mainline sensor integration",
};

const struct oxygen_sensors_backend *oxygen_sensors_backend_mainline(void) {
    return &kBackend;
}
