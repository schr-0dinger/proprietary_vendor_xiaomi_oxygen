#define LOG_TAG "ConsumerIrHal"

#include <errno.h>

#include <log/log.h>

#include "consumerir_backend.h"

static int consumerir_downstream_transmit(
    int carrier_freq,
    const int pattern[],
    int pattern_len) {
    int ret = consumerir_transmit_lirc_pulse(
        "downstream",
        carrier_freq,
        pattern,
        pattern_len,
        0,
        1);
    if (ret == -ENOTTY || ret == -EINVAL || ret == -ENOSYS || ret == -EOPNOTSUPP) {
        ALOGI("downstream IR backend falling back to standard LIRC pulse mode");
        ret = consumerir_transmit_lirc_pulse(
            "downstream-fallback",
            carrier_freq,
            pattern,
            pattern_len,
            1,
            0);
    }
    return ret;
}

static const struct consumerir_backend kBackend = {
    .name = "downstream",
    .transmit = consumerir_downstream_transmit,
};

const struct consumerir_backend *consumerir_backend_downstream(void) {
    return &kBackend;
}
