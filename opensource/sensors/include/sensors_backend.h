#ifndef OXYGEN_SENSORS_BACKEND_H
#define OXYGEN_SENSORS_BACKEND_H

struct oxygen_sensors_backend {
    const char *name;
    const char *transport;
    const char *notes;
};

const struct oxygen_sensors_backend *oxygen_sensors_backend_downstream(void);
const struct oxygen_sensors_backend *oxygen_sensors_backend_mainline(void);
const struct oxygen_sensors_backend *oxygen_sensors_select_backend(void);

#endif
