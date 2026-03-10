#include "consumerir_backend.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char kBackendOverrideEnv[] = "OXYGEN_CONSUMERIR_BACKEND";
static const char kSysfsRcCompatible[] = "/sys/class/rc/rc0/device/of_node/compatible";
static const char kSysfsRcProtocols[] = "/sys/class/rc/rc0/protocols";
static const char kSysfsIrSpiDriver[] = "/sys/bus/spi/drivers/ir-spi";

static int read_text_file(const char *path, char *buffer, size_t buffer_size) {
    int fd = -1;
    ssize_t len = 0;

    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    len = read(fd, buffer, buffer_size - 1);
    close(fd);
    if (len <= 0) {
        return -1;
    }

    buffer[len] = '\0';
    return 0;
}

static int file_contains_text(const char *path, const char *needle) {
    char buffer[256];

    if (read_text_file(path, buffer, sizeof(buffer)) < 0) {
        return 0;
    }

    return strstr(buffer, needle) != NULL;
}

static const struct consumerir_backend *lookup_backend_by_name(const char *name) {
    if (name == NULL || name[0] == '\0') {
        return NULL;
    }

    if (strcmp(name, "downstream") == 0) {
        return consumerir_backend_downstream();
    }

    if (strcmp(name, "mainline") == 0) {
        return consumerir_backend_mainline();
    }

    return NULL;
}

static const struct consumerir_backend *select_override_backend(void) {
    const char *override = getenv(kBackendOverrideEnv);

    if (override == NULL || override[0] == '\0' || strcmp(override, "auto") == 0) {
        return NULL;
    }

    return lookup_backend_by_name(override);
}

static int probe_downstream_backend(void) {
    if (access(kSysfsIrSpiDriver, F_OK) == 0) {
        return 1;
    }

    return file_contains_text(kSysfsRcCompatible, "ir-spi");
}

static int probe_mainline_backend(void) {
    return access(kSysfsRcProtocols, R_OK) == 0;
}

const struct consumerir_backend *consumerir_select_backend(void) {
    const struct consumerir_backend *backend = select_override_backend();

    if (backend != NULL) {
        return backend;
    }

    if (probe_downstream_backend()) {
        return consumerir_backend_downstream();
    }

    if (probe_mainline_backend()) {
        return consumerir_backend_mainline();
    }

    return consumerir_backend_downstream();
}
