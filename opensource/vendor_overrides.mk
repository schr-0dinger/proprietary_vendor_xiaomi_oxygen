# Open-vendor overrides layered on top of the generated oxygen and
# mithorium-common prebuilt lists.
#
# Usage from a product or device makefile:
#   OXYGEN_OPEN_VENDOR_COMPONENTS += consumerir
#
# This keeps the generated vendor blobs as the default, and filters out only the
# prebuilts that have an open replacement module in this repo.

OXYGEN_OPEN_VENDOR_COMPONENTS ?=
MITHORIUM_COMMON_OPEN_VENDOR_COMPONENTS ?=

OXYGEN_CONSUMERIR_PREBUILT := \
    vendor/xiaomi/oxygen/proprietary/vendor/lib64/hw/consumerir.msm8953.so:$(TARGET_COPY_OUT_VENDOR)/lib64/hw/consumerir.msm8953.so

ifneq ($(filter consumerir,$(OXYGEN_OPEN_VENDOR_COMPONENTS)),)
PRODUCT_COPY_FILES := $(filter-out $(OXYGEN_CONSUMERIR_PREBUILT),$(PRODUCT_COPY_FILES))
PRODUCT_PACKAGES += \
    consumerir.msm8953.open
endif
