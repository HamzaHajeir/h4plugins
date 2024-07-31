#pragma once

#include <H4Service.h>

#if H4P_BLE_AVAILABLE

#include <BLEDevice.h>
#define H4_BLE_MTU 			512

namespace H4P_BLE {

extern bool initialized;
void init();
};
#endif