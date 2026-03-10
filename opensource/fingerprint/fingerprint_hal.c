#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "fingerprint_backend.h"
#include "fingerprint_compat.h"

struct oxygen_fingerprint_device {
    fingerprint_device_t device;
    fingerprint_notify_t notify;
    const struct oxygen_fingerprint_backend *backend;
};

static int fingerprint_set_notify(fingerprint_device_t *device, fingerprint_notify_t notify) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;

    oxygen_device->notify = notify;
    return 0;
}

static uint64_t fingerprint_pre_enroll(fingerprint_device_t *device) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->pre_enroll();
}

static int fingerprint_enroll(
    fingerprint_device_t *device,
    const void *hat,
    uint32_t gid,
    uint32_t timeout_sec) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->enroll(hat, gid, timeout_sec);
}

static int fingerprint_post_enroll(fingerprint_device_t *device) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->post_enroll();
}

static uint64_t fingerprint_get_authenticator_id(fingerprint_device_t *device) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->get_authenticator_id();
}

static int fingerprint_cancel(fingerprint_device_t *device) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->cancel();
}

static int fingerprint_enumerate(fingerprint_device_t *device) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->enumerate();
}

static int fingerprint_remove(fingerprint_device_t *device, uint32_t gid, uint32_t fid) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->remove(gid, fid);
}

static int fingerprint_set_active_group(
    fingerprint_device_t *device,
    uint32_t gid,
    const char *store_path) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->set_active_group(gid, store_path);
}

static int fingerprint_authenticate(
    fingerprint_device_t *device,
    uint64_t operation_id,
    uint32_t gid) {
    struct oxygen_fingerprint_device *oxygen_device = (struct oxygen_fingerprint_device *)device;
    return oxygen_device->backend->authenticate(operation_id, gid);
}

static int fingerprint_close(hw_device_t *device) {
    free(device);
    return 0;
}

static int fingerprint_open(const hw_module_t *module, const char *id, hw_device_t **device) {
    struct oxygen_fingerprint_device *oxygen_device = NULL;

    if (device == NULL) {
        return -EINVAL;
    }

    if (id != NULL && strcmp(id, FINGERPRINT_HARDWARE_MODULE_ID) != 0) {
        return -EINVAL;
    }

    oxygen_device = calloc(1, sizeof(*oxygen_device));
    if (oxygen_device == NULL) {
        return -ENOMEM;
    }

    oxygen_device->backend = oxygen_fingerprint_select_backend();
    oxygen_device->device.common.tag = HARDWARE_DEVICE_TAG;
    oxygen_device->device.common.version = 0;
    oxygen_device->device.common.module = (hw_module_t *)module;
    oxygen_device->device.common.close = fingerprint_close;
    oxygen_device->device.set_notify = fingerprint_set_notify;
    oxygen_device->device.pre_enroll = fingerprint_pre_enroll;
    oxygen_device->device.enroll = fingerprint_enroll;
    oxygen_device->device.post_enroll = fingerprint_post_enroll;
    oxygen_device->device.get_authenticator_id = fingerprint_get_authenticator_id;
    oxygen_device->device.cancel = fingerprint_cancel;
    oxygen_device->device.enumerate = fingerprint_enumerate;
    oxygen_device->device.remove = fingerprint_remove;
    oxygen_device->device.set_active_group = fingerprint_set_active_group;
    oxygen_device->device.authenticate = fingerprint_authenticate;

    *device = &oxygen_device->device.common;
    return 0;
}

static hw_module_methods_t kFingerprintModuleMethods = {
    .open = fingerprint_open,
};

__attribute__((visibility("default")))
fingerprint_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = 1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = FINGERPRINT_HARDWARE_MODULE_ID,
        .name = "Oxygen Open Fingerprint Skeleton",
        .author = "oxygen open-vendor",
        .methods = &kFingerprintModuleMethods,
    },
};
