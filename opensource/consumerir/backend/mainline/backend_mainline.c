#include "consumerir_backend.h"

static int consumerir_mainline_transmit(
    int carrier_freq,
    const int pattern[],
    int pattern_len) {
    return consumerir_transmit_lirc_pulse(
        "mainline",
        carrier_freq,
        pattern,
        pattern_len,
        1,
        0);
}

static const struct consumerir_backend kBackend = {
    .name = "mainline",
    .transmit = consumerir_mainline_transmit,
};

const struct consumerir_backend *consumerir_backend_mainline(void) {
    return &kBackend;
}
