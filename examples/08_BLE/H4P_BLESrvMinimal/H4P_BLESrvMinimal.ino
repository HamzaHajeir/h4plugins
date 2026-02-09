#include <Arduino.h>
#include <H4Plugins.h>
H4_USE_PLUGINS(115200, H4_Q_CAPACITY, false) // Serial baud rate, Q size, SerialCmd autostop
//
#ifdef ESP32 && H4P_BLE_AVAILABLE // Currently unavailable for ESP32S3 as BlueDroid got replaced with NimBLE

H4P_SerialLogger h4sl;
H4P_PinMachine h4gm;
H4P_BLEServer h4ble;
H4P_BinarySwitch h4onoff(4, ACTIVE_LOW, H4P_UILED_ORANGE, OFF, 10000);

bool bleConnected;

void onBLESrvConnect() {
	Serial.printf("BLE Server -> Client Connected\n");
	bleConnected = true;
}
void onBLESrvDisconnect() {
	Serial.printf("BLE Server -> Client Disconnected\n");
	bleConnected = false;
}

void h4pGlobalEventHandler(const std::string& svc,H4PE_TYPE t,const std::string& msg)
{
	switch (t)
	{
		H4P_DEFAULT_SYSTEM_HANDLER;
	case H4PE_BLESINIT:
		h4ble.elemAddBoolean("mybool");
		h4.every(1000, []{
			static bool val = false;
			val = !val;
			if (bleConnected) {
				h4ble.elemSetValue("mybool", val);
			}
		});
		break;
	case H4PE_SERVICE:
		H4P_SERVICE_ADAPTER(BLESrv);
		break;
	default:
		break;
	}
}
void h4setup()
{
}

#endif