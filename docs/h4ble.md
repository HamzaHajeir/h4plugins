![H4P Flyer](../assets/MQTTLogo.jpg) 

# H4P_BLEServer

## Service shortname blesrv

<!-- [ ] Add messaging document -->
<!-- Connects to an MQTT server and manages automatic reconnection after failure. Provides utility functions to simplify message handling / parsing. User will find essential pre-reading [Handling MQTT topics / commands](mqcmds.md) -->

---

## Contents

- [Service shortname blesrv](#service-shortname-blesrv)
- [Contents](#contents)
- [Dependencies](#dependencies)
- [Commands Added](#commands-added)
- [Example sketches](#example-sketches)

---
# Usage

```cpp
H4P_BLEServer h4ble(...
```

This plugin is a "singleton" - there may be only one single instance of it in the app. 
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

## Dependencies

* No dependencies.

## Commands Added

* 

# Service Commands

`stop` will stop advertising the BLE server and disconnect from any connected client.
`start` will start advertising the service.

# Callbacks

```cpp

```

<!-- # Topics automatically published

Publishes `h4/< your device name >/offline` when it loses the MQTT connection.

It publishes `h4/< your device name >/report` with a JSON payload of e.g.

```JSON
{
    "bin": "Generic_SONOFF.ino",
    "board": "D1_MINI",
    "h4P": "1.0.1",
    "ip": "192.168.1.118",
}
``` -->

---

# Services and Characteristics UUIDs

Here's the UUIDs of the main service and different UUIDs:

```cpp
#define H4_SERVICE_UUID        	"9652c3ca-1231-4607-9d40-6afd67609443"

// Characteristics
#define CMD_CHAR_UUID 			"3c1eb836-4223-4f3e-9c9d-10c5dae1d9b1" // Receives H4-style commands (h4/...), Writable characteristic.
#define REPLY_CHAR_UUID			"5f1c2e8d-f531-488e-8198-0132ec230a6f" // Notifyable characteristic that holds the reply from H4-style commands.
#define EVENTS_CHAR_UUID		"d684fb38-8fdc-484f-a3a5-15233de0dd9d" // Notifyable characteristic for Events sent from the device, with the format "key~value"
// UIIDENTIFY_CHAR_UUID
// USER MSG UUID ...?

// USER services/characteristics
```

---

# API

```cpp
/*
Constructor
*/
H4P_BLEServer();
void BLEAdd();

void notify(const std::string& key, const std::string& value);


BLEAdvertising* getAdvertising() { return h4Advertising; }
BLEService* getH4Service() { return h4Service; }
```

## Example sketches

<!-- * [Subtopics](../examples/07_MQTT/H4P_MQTT_Subtopics/H4P_MQTT_Subtopics.ino)
* [Wildcards](../examples/07_MQTT/H4P_MQTT_Wildcards/H4P_MQTT_Wildcards.ino)
* [Custom Last Will and Testament](../examples/07_MQTT/H4P_MQTT_CustomLWT/H4P_MQTT_CustomLWT.ino) -->
  
---

(c) 2024 Hamzah Hajeir 

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
