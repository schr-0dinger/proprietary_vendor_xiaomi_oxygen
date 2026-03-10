#define LOG_TAG "ConsumerIrHal"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <log/log.h>
#include <hardware/consumerir.h>
#include <hardware/hardware.h>

#include "consumerir_backend.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

struct oxygen_consumerir_device {
    consumerir_device_t device;
    const struct consumerir_backend *backend;
};

static const consumerir_freq_range_t kCarrierFreqs[] = {
    { .min = 30000, .max = 30000 },
    { .min = 33000, .max = 33000 },
    { .min = 36000, .max = 36000 },
    { .min = 38000, .max = 38000 },
    { .min = 40000, .max = 40000 },
    { .min = 56000, .max = 56000 },
};

static int consumerir_transmit(
    struct consumerir_device *dev,
    int carrier_freq,
    const int pattern[],
    int pattern_len) {
    struct oxygen_consumerir_device *oxygen_dev = (struct oxygen_consumerir_device *)dev;
    return oxygen_dev->backend->transmit(carrier_freq, pattern, pattern_len);
}

static int consumerir_get_num_carrier_freqs(struct consumerir_device *dev) {
    (void)dev;
    return (int)ARRAY_SIZE(kCarrierFreqs);
}

static int consumerir_get_carrier_freqs(
    struct consumerir_device *dev,
    size_t len,
    consumerir_freq_range_t *ranges) {
    size_t to_copy = ARRAY_SIZE(kCarrierFreqs);
    (void)dev;

    if (ranges == NULL) {
        return -EINVAL;
    }

    if (len < to_copy) {
        to_copy = len;
    }

    memcpy(ranges, kCarrierFreqs, to_copy * sizeof(*ranges));
    return (int)to_copy;
}

static int consumerir_close(hw_device_t *dev) {
    free(dev);
    return 0;
}

static int consumerir_open(
    const hw_module_t *module,
    const char *name,
    hw_device_t **device) {
    struct oxygen_consumerir_device *dev = NULL;

    if (strcmp(name, CONSUMERIR_TRANSMITTER) != 0) {
        return -EINVAL;
    }

    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    dev = calloc(1, sizeof(*dev));
    if (dev == NULL) {
        return -ENOMEM;
    }

    dev->backend = consumerir_select_backend();
    dev->device.common.tag = HARDWARE_DEVICE_TAG;
    dev->device.common.version = 0;
    dev->device.common.module = (struct hw_module_t *)module;
    dev->device.common.close = consumerir_close;
    dev->device.transmit = consumerir_transmit;
    dev->device.get_num_carrier_freqs = consumerir_get_num_carrier_freqs;
    dev->device.get_carrier_freqs = consumerir_get_carrier_freqs;

    *device = &dev->device.common;

    ALOGI("opened open consumerir HAL with %s backend", dev->backend->name);
    return 0;
}

static struct hw_module_methods_t consumerir_module_methods = {
    .open = consumerir_open,
};

consumerir_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = CONSUMERIR_MODULE_API_VERSION_1_0,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = CONSUMERIR_HARDWARE_MODULE_ID,
        .name = "Oxygen Open Consumer IR HAL",
        .author = "oxygen open-vendor",
        .methods = &consumerir_module_methods,
    },
};
