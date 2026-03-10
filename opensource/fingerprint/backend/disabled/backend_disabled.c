#include <errno.h>

#include "fingerprint_backend.h"

static int disabled_set_active_group(uint32_t gid, const char *store_path) {
    (void)gid;
    (void)store_path;
    return -ENOSYS;
}

static uint64_t disabled_pre_enroll(void) {
    return 0;
}

static int disabled_enroll(const void *hat, uint32_t gid, uint32_t timeout_sec) {
    (void)hat;
    (void)gid;
    (void)timeout_sec;
    return -ENOSYS;
}

static int disabled_post_enroll(void) {
    return -ENOSYS;
}

static uint64_t disabled_get_authenticator_id(void) {
    return 0;
}

static int disabled_cancel(void) {
    return -ENOSYS;
}

static int disabled_enumerate(void) {
    return -ENOSYS;
}

static int disabled_remove(uint32_t gid, uint32_t fid) {
    (void)gid;
    (void)fid;
    return -ENOSYS;
}

static int disabled_authenticate(uint64_t operation_id, uint32_t gid) {
    (void)operation_id;
    (void)gid;
    return -ENOSYS;
}

static const struct oxygen_fingerprint_backend kBackend = {
    .name = "disabled",
    .set_active_group = disabled_set_active_group,
    .pre_enroll = disabled_pre_enroll,
    .enroll = disabled_enroll,
    .post_enroll = disabled_post_enroll,
    .get_authenticator_id = disabled_get_authenticator_id,
    .cancel = disabled_cancel,
    .enumerate = disabled_enumerate,
    .remove = disabled_remove,
    .authenticate = disabled_authenticate,
};

const struct oxygen_fingerprint_backend *oxygen_fingerprint_backend_disabled(void) {
    return &kBackend;
}
