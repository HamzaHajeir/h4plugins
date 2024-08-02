#pragma once

#include <H4Service.h>

#if H4P_BLE_AVAILABLE

#include <BLEDevice.h>
#define H4_BLE_MTU 			512

#define H4P_BLECLIENT_DEBUG 0
#define H4P_BLESERVER_DEBUG 0

#if H4P_BLECLIENT_DEBUG
#define H4PBC_PRINTF(x,...) Serial.printf((x),##__VA_ARGS__);
#else
#define H4PBC_PRINTF(x,...)
#endif

#if H4P_BLECLIENT_DEBUG
#define H4PBS_PRINTF(x,...) Serial.printf((x),##__VA_ARGS__);
#else
#define H4PBS_PRINTF(x,...)
#endif

namespace H4P_BLE {

extern bool initialized;
void init();
};
#endif