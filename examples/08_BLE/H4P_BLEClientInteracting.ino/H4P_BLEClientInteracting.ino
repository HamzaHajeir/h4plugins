#include <Arduino.h>
#include <H4Plugins.h>
H4_USE_PLUGINS(115200, H4_Q_CAPACITY, false) // Serial baud rate, Q size, SerialCmd autostop

H4P_BLEClient h4bleclient;

void h4pGlobalEventHandler(const std::string& svc,H4PE_TYPE t,const std::string& msg)
{
	switch (t)
	{
		H4P_DEFAULT_SYSTEM_HANDLER;
	}
}

bool bleClientConnected = false;
BLERemoteCharacteristic* cmdCharacteristic = nullptr;
BLERemoteCharacteristic* replyCharacteristic = nullptr;
H4_TIMER requestor;
void onNotify(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
	Serial.printf("onNotify(%s, %s, is=%d)\n", pBLERemoteCharacteristic->getUUID().toString().c_str(), std::string(reinterpret_cast<char*>(pData), length).c_str(), isNotify);
}
void onConnect() {
	Serial.printf("onConnect()\n");
	bleClientConnected = true;
	cmdCharacteristic = h4bleclient.getRemoteCharacteristic(BLEUUID("9652c3ca-1231-4607-9d40-6afd67609443"), BLEUUID("3c1eb836-4223-4f3e-9c9d-10c5dae1d9b1"));
	replyCharacteristic = h4bleclient.getRemoteCharacteristic(BLEUUID("9652c3ca-1231-4607-9d40-6afd67609443"), BLEUUID("5f1c2e8d-f531-488e-8198-0132ec230a6f"));
	Serial.printf("cmdChar %p {%s} replyChar %p {%s}\n", 
								cmdCharacteristic, 
								cmdCharacteristic ? cmdCharacteristic->getUUID().toString().c_str() : "", 
								replyCharacteristic,
								replyCharacteristic ? replyCharacteristic->getUUID().toString().c_str() : ""
								);

	requestor = h4.every(5000, []{
		Serial.printf("Commanding/Requesting the H4 Server...\n");
		if(cmdCharacteristic->canWrite()){
			Serial.printf("Writing to command characteristic\n");
			cmdCharacteristic->writeValue("h4/toggle");
			cmdCharacteristic->writeValue("help");
		} else {
			Serial.printf("Can not write! %d\n", cmdCharacteristic->canWrite());
		}
	});
}
void onDisconnect() {
	Serial.printf("onDisconnect()\n");
	bleClientConnected = false;

	h4.cancel(requestor);
	requestor = nullptr;
}


void h4setup()
{
	h4bleclient.add({BLEUUID("9652c3ca-1231-4607-9d40-6afd67609443"), true},	  // H4_SERVICE_UUID 	, mandatory=true
					 {
						 {BLEUUID("3c1eb836-4223-4f3e-9c9d-10c5dae1d9b1"), true}, // CMD_CHAR_UUID 		, mandatory=true
						 {BLEUUID("5f1c2e8d-f531-488e-8198-0132ec230a6f"), true}, // REPLY_CHAR_UUID 	, mandatory=true
						 {BLEUUID("d684fb38-8fdc-484f-a3a5-15233de0dd9d"), true}, // ELEMENTS_CHAR_UUID , mandatory=true
						 {BLEUUID("53922702-8a3a-41c2-9e5e-d8c90609855e"), true}, // H4UIDATA_CHAR_UUID , mandatory=true
					 },
					 onNotify);
	h4bleclient.setCallbacks(onConnect, onDisconnect);
	h4.queueFunction([]{h4bleclient.start();});
}
