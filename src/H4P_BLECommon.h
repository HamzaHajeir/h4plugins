#pragma once

#include <H4Service.h>

#if H4P_BLE_AVAILABLE

#include <BLEDevice.h>

namespace H4P_BLE {

extern bool initialized;
void init();
};
#endif