#include "sensors_backend.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char kBackendOverrideEnv[] = "OXYGEN_SENSORS_BACKEND";
static const char kVendorHalConfig[] = "/vendor/etc/sensors/hals.conf";
static const char kIioDevices[] = "/sys/bus/iio/devices";

static const struct oxygen_sensors_backend *lookup_backend_by_name(const char *name) {
    if (name == NULL || name[0] == '\0') {
        return NULL;
    }

    if (strcmp(name, "downstream") == 0) {
        return oxygen_sensors_backend_downstream();
    }

    if (strcmp(name, "mainline") == 0) {
        return oxygen_sensors_backend_mainline();
    }

    return NULL;
}

const struct oxygen_sensors_backend *oxygen_sensors_select_backend(void) {
    const char *override = getenv(kBackendOverrideEnv);
    const struct oxygen_sensors_backend *backend = lookup_backend_by_name(override);

    if (backend != NULL) {
        return backend;
    }

    if (override != NULL && strcmp(override, "auto") != 0 && override[0] != '\0') {
        return oxygen_sensors_backend_downstream();
    }

    if (access(kVendorHalConfig, R_OK) == 0) {
        return oxygen_sensors_backend_downstream();
    }

    if (access(kIioDevices, R_OK) == 0) {
        return oxygen_sensors_backend_mainline();
    }

    return oxygen_sensors_backend_downstream();
}
