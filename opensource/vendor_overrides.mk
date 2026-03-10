# Open-vendor overrides layered on top of the generated oxygen and
# mithorium-common prebuilt lists.
#
# Usage from a product or device makefile:
#   OXYGEN_OPEN_VENDOR_COMPONENTS += consumerir
#   OXYGEN_OPEN_VENDOR_COMPONENTS += sensors fingerprint
#   OXYGEN_OPEN_VENDOR_COMPONENTS += camera-imx386 camera-stubs
#
# This keeps the generated vendor blobs as the default, and filters out only the
# prebuilts that have an open replacement module in this repo.

OXYGEN_OPEN_VENDOR_COMPONENTS ?=
MITHORIUM_COMMON_OPEN_VENDOR_COMPONENTS ?=

OXYGEN_CONSUMERIR_PREBUILT := \
    vendor/xiaomi/oxygen/proprietary/vendor/lib64/hw/consumerir.msm8953.so:$(TARGET_COPY_OUT_VENDOR)/lib64/hw/consumerir.msm8953.so
OXYGEN_SENSORS_PREBUILT := \
    vendor/xiaomi/oxygen/proprietary/vendor/bin/sensors.qti:$(TARGET_COPY_OUT_VENDOR)/bin/sensors.qti
OXYGEN_FINGERPRINT_PREBUILT := \
    vendor/xiaomi/oxygen/proprietary/vendor/lib64/hw/fingerprint.msm8953.so:$(TARGET_COPY_OUT_VENDOR)/lib64/hw/fingerprint.msm8953.so
OXYGEN_CAMERA_IMX386_PREBUILTS := \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_imx386_sunny.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_imx386_sunny.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_imx386_sunny_eeprom.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_imx386_sunny_eeprom.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libactuator_oxygen_dw9763_sunny.so:$(TARGET_COPY_OUT_VENDOR)/lib/libactuator_oxygen_dw9763_sunny.so
OXYGEN_CAMERA_STUB_PREBUILTS := \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_ov12a_sunny.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_ov12a_sunny.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_ov12a_sunny_eeprom.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_ov12a_sunny_eeprom.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libactuator_oxygen_ov12a_sunny_dw9763.so:$(TARGET_COPY_OUT_VENDOR)/lib/libactuator_oxygen_ov12a_sunny_dw9763.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_ov12a_ofilm.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_ov12a_ofilm.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_ov12a_ofilm_eeprom.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_ov12a_ofilm_eeprom.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libactuator_oxygen_ov12a_ofilm_dw9718.so:$(TARGET_COPY_OUT_VENDOR)/lib/libactuator_oxygen_ov12a_ofilm_dw9718.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_qtech.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_s5k5e8_qtech.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_qtech_eeprom.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_s5k5e8_qtech_eeprom.so \
    vendor/xiaomi/oxygen/proprietary/vendor/lib/libmmcamera_oxygen_s5k5e8_ofilm.so:$(TARGET_COPY_OUT_VENDOR)/lib/libmmcamera_oxygen_s5k5e8_ofilm.so

OXYGEN_CAMERA_IMX386_PACKAGES := \
    libmmcamera_oxygen_imx386_sunny.open \
    libmmcamera_oxygen_imx386_sunny_eeprom.open \
    libactuator_oxygen_dw9763_sunny.open
OXYGEN_CAMERA_STUB_PACKAGES := \
    libmmcamera_oxygen_ov12a_sunny.open \
    libmmcamera_oxygen_ov12a_sunny_eeprom.open \
    libactuator_oxygen_ov12a_sunny_dw9763.open \
    libmmcamera_oxygen_ov12a_ofilm.open \
    libmmcamera_oxygen_ov12a_ofilm_eeprom.open \
    libactuator_oxygen_ov12a_ofilm_dw9718.open \
    libmmcamera_oxygen_s5k5e8_qtech.open \
    libmmcamera_oxygen_s5k5e8_qtech_eeprom.open \
    libmmcamera_oxygen_s5k5e8_ofilm.open

ifneq ($(filter consumerir,$(OXYGEN_OPEN_VENDOR_COMPONENTS)),)
PRODUCT_COPY_FILES := $(filter-out $(OXYGEN_CONSUMERIR_PREBUILT),$(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES += consumerir.msm8953.open
endif

ifneq ($(filter sensors,$(OXYGEN_OPEN_VENDOR_COMPONENTS)),)
PRODUCT_COPY_FILES := $(filter-out $(OXYGEN_SENSORS_PREBUILT),$(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES += sensors.qti.open
endif

ifneq ($(filter fingerprint,$(OXYGEN_OPEN_VENDOR_COMPONENTS)),)
PRODUCT_COPY_FILES := $(filter-out $(OXYGEN_FINGERPRINT_PREBUILT),$(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES += fingerprint.msm8953.open
endif

ifneq ($(filter camera-imx386 camera-all,$(OXYGEN_OPEN_VENDOR_COMPONENTS)),)
PRODUCT_COPY_FILES := $(filter-out $(OXYGEN_CAMERA_IMX386_PREBUILTS),$(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES += $(OXYGEN_CAMERA_IMX386_PACKAGES)
endif

ifneq ($(filter camera-stubs camera-all,$(OXYGEN_OPEN_VENDOR_COMPONENTS)),)
PRODUCT_COPY_FILES := $(filter-out $(OXYGEN_CAMERA_STUB_PREBUILTS),$(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES += $(OXYGEN_CAMERA_STUB_PACKAGES)
endif
