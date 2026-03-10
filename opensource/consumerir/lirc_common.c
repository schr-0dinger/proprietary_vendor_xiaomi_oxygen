#define LOG_TAG "ConsumerIrHal"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/lirc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <log/log.h>

#include "consumerir_backend.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char kLircDevice[] = "/dev/lirc0";
/*
 * Xiaomi's consumerir blob uses ioctl number 0x4004691c before write().
 * The upstream LIRC header names that slot differently, so treat this as a
 * downstream reconstruction point until the kernel-side contract is verified.
 */
static const uint32_t kDownstreamLengthIoctl = _IOW('i', 0x1c, __u32);

static int open_lirc_device(const char *backend_name) {
    int fd = open(kLircDevice, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        ALOGE("[%s] open(%s) failed: %s", backend_name, kLircDevice, strerror(errno));
        return -errno;
    }
    return fd;
}

static int set_ioctl_u32(
    int fd,
    unsigned long request,
    uint32_t value,
    const char *backend_name,
    const char *label,
    int required) {
    if (ioctl(fd, request, &value) == 0) {
        return 0;
    }
    if (!required) {
        ALOGW("[%s] %s ioctl failed: %s", backend_name, label, strerror(errno));
        return 0;
    }
    ALOGE("[%s] %s ioctl failed: %s", backend_name, label, strerror(errno));
    return -errno;
}

static int write_all(int fd, const void *data, size_t size) {
    const uint8_t *cursor = (const uint8_t *)data;
    size_t written = 0;
    while (written < size) {
        ssize_t ret = write(fd, cursor + written, size - written);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -errno;
        }
        if (ret == 0) {
            return -EIO;
        }
        written += (size_t)ret;
    }
    return 0;
}

int consumerir_validate_pattern(
    int carrier_freq,
    const int pattern[],
    int pattern_len,
    int *total_time) {
    int total = 0;
    int i = 0;

    if (carrier_freq <= 0 || pattern == NULL || pattern_len <= 0) {
        return -EINVAL;
    }

    for (i = 0; i < pattern_len; ++i) {
        if (pattern[i] <= 0) {
            return -EINVAL;
        }
        if (total > INT_MAX - pattern[i]) {
            return -EOVERFLOW;
        }
        total += pattern[i];
    }

    if (total_time != NULL) {
        *total_time = total;
    }
    return 0;
}

int consumerir_transmit_lirc_pulse(
    const char *backend_name,
    int carrier_freq,
    const int pattern[],
    int pattern_len,
    int set_send_mode,
    int use_length_ioctl) {
    int fd = -1;
    int ret = 0;
    int total_time = 0;
    lirc_t *buffer = NULL;
    uint32_t length_hint = 0;
    size_t buffer_size = 0;
    int i = 0;

    ret = consumerir_validate_pattern(carrier_freq, pattern, pattern_len, &total_time);
    if (ret < 0) {
        return ret;
    }

    fd = open_lirc_device(backend_name);
    if (fd < 0) {
        return fd;
    }

    if (set_send_mode) {
        ret = set_ioctl_u32(
            fd,
            LIRC_SET_SEND_MODE,
            LIRC_MODE_PULSE,
            backend_name,
            "LIRC_SET_SEND_MODE",
            1);
        if (ret < 0) {
            goto out;
        }
    }

    ret = set_ioctl_u32(
        fd,
        LIRC_SET_SEND_CARRIER,
        (uint32_t)carrier_freq,
        backend_name,
        "LIRC_SET_SEND_CARRIER",
        1);
    if (ret < 0) {
        goto out;
    }

    (void)set_ioctl_u32(
        fd,
        LIRC_SET_SEND_DUTY_CYCLE,
        50,
        backend_name,
        "LIRC_SET_SEND_DUTY_CYCLE",
        0);

    buffer_size = (size_t)pattern_len * sizeof(*buffer);
    buffer = calloc((size_t)pattern_len, sizeof(*buffer));
    if (buffer == NULL) {
        ret = -ENOMEM;
        goto out;
    }

    for (i = 0; i < pattern_len; ++i) {
        buffer[i] = (lirc_t)pattern[i];
    }

    if (use_length_ioctl) {
        length_hint = (uint32_t)buffer_size;
        ret = set_ioctl_u32(
            fd,
            kDownstreamLengthIoctl,
            length_hint,
            backend_name,
            "downstream_length_hint",
            1);
        if (ret < 0) {
            goto out;
        }
    }

    ret = write_all(fd, buffer, buffer_size);
    if (ret < 0) {
        ALOGE("[%s] write(%s) failed: %s", backend_name, kLircDevice, strerror(-ret));
        goto out;
    }

    ALOGD("[%s] transmitted %d usec at %d Hz via %s", backend_name, total_time, carrier_freq, kLircDevice);
    ret = 0;

out:
    free(buffer);
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}
