# H4P_BLEClient

## Service shortname bleclt

Manages a BLE Client.

Currently works for ESP32's only.

---

## Contents

- [Service shortname bleclt](#service-shortname-bleclt)
- [Contents](#contents)
- [Dependencies](#dependencies)
- [Commands Added](#commands-added)
- [Callbacks and Usage](#callbacks-and-usage)
- [Example sketches](#example-sketches)
- [APIs](#api)
---
# Usage

```cpp
H4P_BLEClient h4bleclient;
```

This plugin is a "singleton" - there may be only one single instance of it in the app. 
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

## Dependencies

* No dependencies.

## Commands Added

* No commands were added

# Service Commands

`stop` will stop trying to connect to the specified the BLE services and disconnect from any connected one.
`start` will start scanning to connect to the specified service(s).

# Callbacks and Usage

To receive onConnect/onDisconnect events, one can register the callbacks via `setCallbacks()` method:

```cpp
void onConnect() {
	Serial.printf("onConnect()\n");
	bleClientConnected = true;
}
void onDisconnect() {
	Serial.printf("onDisconnect()\n");
	bleClientConnected = false;
}

void h4setup() {
	h4bleclient.setCallbacks(onConnect, onDisconnect);
}

```

One can add what he is looking for as services and clients:

```cpp
bool bleClientConnected = false;
void onNotify(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
	Serial.printf("onNotify(%s, %s, is=%d)\n", pBLERemoteCharacteristic->getUUID().toString().c_str(), std::string(reinterpret_cast<char*>(pData), length).c_str(), isNotify);
}

void h4setup() {
	h4bleclient.add({BLEUUID("9652c3ca-1231-4607-9d40-6afd67609443"), true},	  // H4_SERVICE_UUID 	, mandatory=true
					 {
						 {BLEUUID("3c1eb836-4223-4f3e-9c9d-10c5dae1d9b1"), true}, // CMD_CHAR_UUID 		, mandatory=true
						 {BLEUUID("5f1c2e8d-f531-488e-8198-0132ec230a6f"), true}, // REPLY_CHAR_UUID 	, mandatory=true
						 {BLEUUID("d684fb38-8fdc-484f-a3a5-15233de0dd9d"), true}, // ELEMENTS_CHAR_UUID , mandatory=true
						 {BLEUUID("53922702-8a3a-41c2-9e5e-d8c90609855e"), true}, // H4UIDATA_CHAR_UUID , mandatory=true
					 },
					 onNotify);

	// h4bleclient.setCallbacks(...)
	h4.queueFunction([]{ h4bleclient.start(); });
}
```
---

# API

```cpp
/*
Constructor
*/
H4P_BLEClient();

void 			start(); // Starts the scan and therefore connect when possible
void 			stop(); // Disables the scan and disconnect (if any)
void 			add(std::pair<BLEUUID,bool> service, std::initializer_list<std::pair<BLEUUID, bool>> characteristics, notify_callback onNotify = nullptr); // Adds a service tree (Service UUID & corresponding lookout characteristics), each one is linked with a boolean flag to tell H4BLEClient its mandatory to look for. onNotify callback function to receive notifies.
void 			removeService(BLEUUID service); // Remove a service, alongwith its characteristics.
void 			removeCharacteristic(BLEUUID service, const BLEUUID& characteristic); // Remove a given characteristic from a given service.
void 			removeCharacteristics(BLEUUID service, const std::vector<BLEUUID>& characteristics); // Remove several characteristics at once.
void 			clearServices(); // Clear the map.
void 			setCallbacks(H4_FN_VOID onConnect, H4_FN_VOID onDisconnect); // Sets onConnect and onDisconnect callback functions.


BLERemoteCharacteristic* getRemoteCharacteristic(BLEUUID s, BLEUUID c); // Returns the Raw BLERemoteCharacteristic object, suitable to call onConnect to retreive it.
```

## Example sketches

* [H4InteractingClient](../examples/08_BLE/H4P_BLEClientInteracting.ino/H4P_BLEClientInteracting.ino)
  

---

(c) 2024 Hamzah Hajeir 

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
