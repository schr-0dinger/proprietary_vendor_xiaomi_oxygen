#ifndef OXYGEN_FINGERPRINT_BACKEND_H
#define OXYGEN_FINGERPRINT_BACKEND_H

#include <stdint.h>

struct oxygen_fingerprint_backend {
    const char *name;
    int (*set_active_group)(uint32_t gid, const char *store_path);
    uint64_t (*pre_enroll)(void);
    int (*enroll)(const void *hat, uint32_t gid, uint32_t timeout_sec);
    int (*post_enroll)(void);
    uint64_t (*get_authenticator_id)(void);
    int (*cancel)(void);
    int (*enumerate)(void);
    int (*remove)(uint32_t gid, uint32_t fid);
    int (*authenticate)(uint64_t operation_id, uint32_t gid);
};

const struct oxygen_fingerprint_backend *oxygen_fingerprint_backend_disabled(void);
const struct oxygen_fingerprint_backend *oxygen_fingerprint_backend_passthrough(void);
const struct oxygen_fingerprint_backend *oxygen_fingerprint_select_backend(void);

#endif
