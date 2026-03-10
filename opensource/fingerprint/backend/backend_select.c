#include "fingerprint_backend.h"

#include <stdlib.h>
#include <string.h>

static const char kBackendOverrideEnv[] = "OXYGEN_FINGERPRINT_BACKEND";

const struct oxygen_fingerprint_backend *oxygen_fingerprint_select_backend(void) {
    const char *override = getenv(kBackendOverrideEnv);

    if (override != NULL && strcmp(override, "passthrough") == 0) {
        return oxygen_fingerprint_backend_passthrough();
    }

    return oxygen_fingerprint_backend_disabled();
}
