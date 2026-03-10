#include "qcom_camera_stub.h"

static const struct qcom_stub_named_module kEepromOpenLib = {
    .name = "oxygen_s5k5e8_qtech_eeprom",
    .kind = "eeprom",
    .abi_revision = OXYGEN_QCOM_CAMERA_STUB_ABI_REVISION,
};

__attribute__((visibility("default")))
void *oxygen_s5k5e8_qtech_eeprom_open_lib(void) {
    return (void *)&kEepromOpenLib;
}
