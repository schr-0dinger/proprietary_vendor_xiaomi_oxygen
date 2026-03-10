#ifndef OXYGEN_FINGERPRINT_COMPAT_H
#define OXYGEN_FINGERPRINT_COMPAT_H

#include <stdint.h>

#define HARDWARE_MODULE_TAG 0xFACA
#define HARDWARE_DEVICE_TAG 0xFACE
#define HARDWARE_HAL_API_VERSION 0
#define FINGERPRINT_HARDWARE_MODULE_ID "fingerprint"

struct hw_module_t;
struct hw_device_t;

typedef struct hw_module_methods_t {
    int (*open)(const struct hw_module_t *module, const char *id, struct hw_device_t **device);
} hw_module_methods_t;

typedef struct hw_module_t {
    uint32_t tag;
    uint16_t module_api_version;
    uint16_t hal_api_version;
    const char *id;
    const char *name;
    const char *author;
    hw_module_methods_t *methods;
    void *dso;
    uint32_t reserved[32];
} hw_module_t;

typedef struct hw_device_t {
    uint32_t tag;
    uint32_t version;
    hw_module_t *module;
    int (*close)(struct hw_device_t *device);
    uint32_t reserved[12];
} hw_device_t;

typedef enum fingerprint_msg_type_t {
    FINGERPRINT_ERROR = -1,
} fingerprint_msg_type_t;

typedef struct fingerprint_msg_t {
    fingerprint_msg_type_t type;
    int32_t data;
} fingerprint_msg_t;

typedef void (*fingerprint_notify_t)(const fingerprint_msg_t *msg);

typedef struct fingerprint_device fingerprint_device_t;

struct fingerprint_device {
    hw_device_t common;
    int (*set_notify)(fingerprint_device_t *device, fingerprint_notify_t notify);
    uint64_t (*pre_enroll)(fingerprint_device_t *device);
    int (*enroll)(fingerprint_device_t *device, const void *hat, uint32_t gid, uint32_t timeout_sec);
    int (*post_enroll)(fingerprint_device_t *device);
    uint64_t (*get_authenticator_id)(fingerprint_device_t *device);
    int (*cancel)(fingerprint_device_t *device);
    int (*enumerate)(fingerprint_device_t *device);
    int (*remove)(fingerprint_device_t *device, uint32_t gid, uint32_t fid);
    int (*set_active_group)(fingerprint_device_t *device, uint32_t gid, const char *store_path);
    int (*authenticate)(fingerprint_device_t *device, uint64_t operation_id, uint32_t gid);
};

typedef struct fingerprint_module {
    hw_module_t common;
} fingerprint_module_t;

#endif
