#include "qcom_camera_stub.h"

static const struct qcom_stub_named_module kEepromOpenLib = {
    .name = "oxygen_ov12a_ofilm_eeprom",
    .kind = "eeprom",
    .abi_revision = OXYGEN_QCOM_CAMERA_STUB_ABI_REVISION,
};

__attribute__((visibility("default")))
void *oxygen_ov12a_ofilm_eeprom_open_lib(void) {
    return (void *)&kEepromOpenLib;
}
