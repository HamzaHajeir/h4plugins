/* 
MIT License

Copyright (c) 2026 H4Group

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Contact Email: TBD
*/
#pragma once

#include <H4Service.h>

#if H4P_BLE_AVAILABLE

#include <BLEDevice.h>
#define H4_BLE_MTU 			512

#define H4P_BLECLIENT_DEBUG 0
#define H4P_BLESERVER_DEBUG 0

#if H4P_BLECLIENT_DEBUG
#define H4PBC_PRINTF(x,...) _H4P_PRINTF((x),##__VA_ARGS__);
#else
#define H4PBC_PRINTF(x,...)
#endif

#if H4P_BLESERVER_DEBUG
#define H4PBS_PRINTF(x,...) _H4P_PRINTF((x),##__VA_ARGS__);
#else
#define H4PBS_PRINTF(x,...)
#endif

namespace H4P_BLE {

extern bool initialized;
void init();
void end();
};
#endif