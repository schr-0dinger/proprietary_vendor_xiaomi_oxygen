#include <stdio.h>
#include <string.h>

#include "sensors_backend.h"

static int print_description(const struct oxygen_sensors_backend *backend) {
    printf("sensors.qti open skeleton\n");
    printf("backend: %s\n", backend->name);
    printf("transport: %s\n", backend->transport);
    printf("notes: %s\n", backend->notes);
    return 0;
}

int main(int argc, char **argv) {
    const struct oxygen_sensors_backend *backend = oxygen_sensors_select_backend();

    if (argc > 1 && strcmp(argv[1], "--describe") == 0) {
        return print_description(backend);
    }

    fprintf(stderr, "sensors.qti open skeleton selected backend=%s\n", backend->name);
    fprintf(stderr, "sensors.qti open skeleton is not runtime-ready yet\n");
    return 64;
}
