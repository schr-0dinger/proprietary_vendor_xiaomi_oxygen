#ifndef OXYGEN_CONSUMERIR_BACKEND_H
#define OXYGEN_CONSUMERIR_BACKEND_H

struct consumerir_backend {
    const char *name;
    int (*transmit)(int carrier_freq, const int pattern[], int pattern_len);
};

const struct consumerir_backend *consumerir_backend_downstream(void);
const struct consumerir_backend *consumerir_backend_mainline(void);
const struct consumerir_backend *consumerir_select_backend(void);

int consumerir_transmit_lirc_pulse(
    const char *backend_name,
    int carrier_freq,
    const int pattern[],
    int pattern_len,
    int set_send_mode,
    int use_length_ioctl);
int consumerir_validate_pattern(
    int carrier_freq,
    const int pattern[],
    int pattern_len,
    int *total_time);

#endif
