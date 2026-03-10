#include <errno.h>

#include "fingerprint_backend.h"

/*
 * Placeholder for future forwarding into the proprietary Goodix stack.
 * The symbol and directory exist now so the repo has a stable place for the
 * real passthrough implementation once the wrapper contract is mapped out.
 */

static int passthrough_set_active_group(uint32_t gid, const char *store_path) {
    (void)gid;
    (void)store_path;
    return -ENOSYS;
}

static uint64_t passthrough_pre_enroll(void) {
    return 0;
}

static int passthrough_enroll(const void *hat, uint32_t gid, uint32_t timeout_sec) {
    (void)hat;
    (void)gid;
    (void)timeout_sec;
    return -ENOSYS;
}

static int passthrough_post_enroll(void) {
    return -ENOSYS;
}

static uint64_t passthrough_get_authenticator_id(void) {
    return 0;
}

static int passthrough_cancel(void) {
    return -ENOSYS;
}

static int passthrough_enumerate(void) {
    return -ENOSYS;
}

static int passthrough_remove(uint32_t gid, uint32_t fid) {
    (void)gid;
    (void)fid;
    return -ENOSYS;
}

static int passthrough_authenticate(uint64_t operation_id, uint32_t gid) {
    (void)operation_id;
    (void)gid;
    return -ENOSYS;
}

static const struct oxygen_fingerprint_backend kBackend = {
    .name = "passthrough",
    .set_active_group = passthrough_set_active_group,
    .pre_enroll = passthrough_pre_enroll,
    .enroll = passthrough_enroll,
    .post_enroll = passthrough_post_enroll,
    .get_authenticator_id = passthrough_get_authenticator_id,
    .cancel = passthrough_cancel,
    .enumerate = passthrough_enumerate,
    .remove = passthrough_remove,
    .authenticate = passthrough_authenticate,
};

const struct oxygen_fingerprint_backend *oxygen_fingerprint_backend_passthrough(void) {
    return &kBackend;
}
