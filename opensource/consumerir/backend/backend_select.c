#include "consumerir_backend.h"

const struct consumerir_backend *consumerir_select_backend(void) {
    return consumerir_backend_downstream();
}
