# H4P_BLEServer

## Service shortname blesrv

Opens BLE Server which serves H4 Commanding and Data/Events emitted from the device. Allows the user to host custom Services alongwith corresponding managed characteristics.

Works currently for ESP32's only.

---

## Contents

- [Service shortname blesrv](#service-shortname-blesrv)
- [Contents](#contents)
- [Dependencies](#dependencies)
- [Commands Added](#commands-added)
- [Callbacks and Usage](#callbacks-and-usage)
- [API](#api)
- [H4BLE Services and Characteristics](#services-and-characteristics-uuids)
- [H4-style interaction](#h4-style-interaction)
- [Extra feature](#extra-feature-wifi-mqtt-provisioning)
- [Example sketches](#example-sketches)

---
# Usage

```cpp
H4P_BLEServer h4ble;
```

This plugin is a "singleton" - there may be only one single instance of it in the app. 
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

## Dependencies

* No dependencies.

## Commands Added

* No commands were added

# Service Commands

`stop` will stop advertising the BLE server and disconnect from any connected client.
`start` will start advertising the service.

# Callbacks and Usage

To receive onConnect/onDisconnect events, one can add the the globalListener the following adapter:

```cpp

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
	case H4PE_SERVICE:
		H4P_SERVICE_ADAPTER(BLESrv);
		break;
	default:
		break;
	}
}

```


Further, one can add custom data/events into serving on initialization, the appropriate place is to listen for H4PE_BLESINIT:

```cpp
void h4pGlobalEventHandler(const std::string& svc,H4PE_TYPE t,const std::string& msg)
{
	switch (t)
	{
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
```

Other events: 
```cpp
void h4pGlobalEventHandler(const std::string& svc,H4PE_TYPE t,const std::string& msg)
{
	switch (t)
	{
    case H4PE_BLESINIT: break; // For the user to initialize his own populate services and to add to advertising, and for adding static/dynamic uidata
    case H4PE_BLESUP: break; // For the user to start his service(s) (The adapter in the first example solves it)
    case H4PE_BLESDOWN: break; // For the user to stop his service(s) (The adapter in the first exmample solves it)
    case H4PE_BLEADD: break;    // For the service itself to add a UI element, no need to interact with.
    case H4PE_BLESYNC: break;   // For the UIData/Events to sync data/events, no need to interact with.
	default:
		break;
	}
}
```


---

# Services and Characteristics UUIDs

Here's the UUIDs of the main service and different UUIDs:

```cpp
/* Service */
#define H4_SERVICE_UUID        	"9652c3ca-1231-4607-9d40-6afd67609443"


/* Characteristics */
#define CMD_CHAR_UUID 			"3c1eb836-4223-4f3e-9c9d-10c5dae1d9b1" // Receives H4-style commands (h4/...), Writable characteristic.
#define REPLY_CHAR_UUID			"5f1c2e8d-f531-488e-8198-0132ec230a6f" // Notifyable/Readable characteristic that holds the reply from H4-style commands.
#define ELEMENTS_CHAR_UUID		"d684fb38-8fdc-484f-a3a5-15233de0dd9d" // Notifyable characteristic which sends UI widgets/events configs onConnect. (Note no technical difference between both, one can consider an event as any other widget, which will have its id and value)
#define H4UIDATA_CHAR_UUID		"53922702-8a3a-41c2-9e5e-d8c90609855e" // // Notifyable characteristic which sends updated data and events.
```

---

# API

```cpp
/*
Constructor
*/
H4P_BLEServer();
void 			elemAdd(const std::string& name,H4P_UI_TYPE t,const std::string& h,H4P_FN_UIGET f,uint8_t color); // Adds custom element with callback value, name: its id, type: H4P_UI_TYPE, h: default value, f: callback that returns std::string and takes no params std::function<std::string(void)>, color: applicable for H4P_UI_BOOL that defines a LED in UI application.
void 			elemRemove(const std::string& name); // remove such element.
void 			clearElems(); // Clears the elements list.
void 			sendElems(); // Sends the elements, important after dynamic elements change.

void            elemAddBoolean(const std::string& name,const std::string& section="u"); // Adds a boolean data, to update such data you'll need to set it through H4P Globals (h4p["name"]="value")
void            elemAddDropdown(const std::string& name,H4P_NVP_MAP options,const std::string& section="u");// dropdown box from maps of options
void            elemAddGlobal(const std::string& name,const std::string& section="u"); // Take field values from h4p[name]
void            elemAddImg(const std::string& name,const std::string& url,const std::string& section="u"); // adds image from url
void            elemAddImgButton(const std::string& name,const std::string& section="u"); // adds clickable image (simulates a button)
void            elemAddInput(const std::string& name,const std::string& section="u"); // simple text input
void            elemAddText(const std::string& name,const std::string& v,const std::string& section="u"); // fixed string value
void            elemAddText(const std::string& name,int v,const std::string& section="u"); // fixed int value
void            elemAddAllUsrFields(const std::string& section="u");// searches for all global variable starting usr_... and adds them to UI

void 			elemSetValue(const std::string &name, std::vector<std::uint8_t> &raw); // change raw bytes value
void 			elemSetValue(const std::string& name, const std::string& value); // change int field value
void            elemSetValue(const std::string& ui,const int f); // change string field value

bool 			addServiceToAdvertise(BLEUUID uuid); // Add your customized and populated service to be advertized
bool 			removeAdvertisingService(BLEUUID uuid); // remove added advertising service.
BLEAdvertising* getAdvertising(); // Gets the advertising object (work on your own)
BLEService* 	getH4Service(); // Gets the H4 BLEService object (work on your own)
```

---

## H4-style interaction:

The user can interact with H4-style [command system](ccc.md) through the BLE server by writing to the command characteristic (`CMD_CHAR_UUID`), and listening for the reply through the notifyable and readable characteristic (`REPLY_CHAR_UUID`).

Example - Receiving "h4/on" on `CMD_CHAR_UUID` will result on "0" notified to `REPLY_CHAR_UUID` as the error code (0: No error). 

When there's an error or subsequent response(s), they will be sent subseuqently through the reply characteristic.


A BLE Client application can setup its UI widgets dynamically over received records through the (`ELEMENTS_CHAR_UUID`) characteristic (Notifyable and readable), and update the corresponding data/events received through (`H4UIDATA_CHAR_UUID`) one (Notifyable one).

The UI widgets/elements are of the following format:  

`ui,{name},{uitype},{section},0,{color},{value}`

Wherein: 
- {name}: The name/id of the data.
- {uitype}: The UI Type (Value of H4P_UI_TYPE enumeration).
- {section}: s-system m-mqtt b-ble u-user o-onoff h-heartbeat g-gpio t-timekeeper 
- {color}: The color of the Boolean UI LED
- {value}: default value

Example: `ui,Device,0,s,0,0,ESP32` is a Text UI type with "Device" id, that belong to System section, with default value of "ESP32"

And the data/events are of comma-seperated key:value:  

Example: Pretending the device name got changed to "MyH4Plugins_System" `device:MyH4Plugins_System`.

---
## Extra Feature: WiFi-MQTT Provisioning 

When `H4P_WIFI_PROV_BY_BLE` option is activated, the server activates sending custom UI widgets to allow for ssid/psk setting, and explicitly allowing for receiving the submit command (`Go=1`). 

However, the client should map these commands into h4-style config commands `h4/config/ssid={newssid}` and `h4/config/psk={newpsk}`.

The MQTT can be provisioned also, which can be done by setting `h4/config/broker={newbroker}`, `h4/config/mQuser={user}`, `h4/config/mQpass={pass}`. Note that broker should contain the prefix `http`/`https` and the port number, such as "http://some.broker:1883".

To submit all changes, you'll need to submit by commanding `h4/config/Go=1`.

Note that these commands are technically receivable even if there's no related UI widgets are sent, except that the submission command can not be received because it's not recognized, except if the AP mode is active (Hence the AP code handles the submission). One can also use the direct `h4/wifi/change` command instead.  

And regarding MQTT config, any setting would take an effect in the next trial of connection to the MQTT server, so not sending the `Go=1` command would not cancel the change, as these are directly mapped h4 commands to change the globals.

= Why WiFi may be not affected by such? someone asks..

- This is because after booting, H4P waits to an event of either Disconnect (cant connect) or Connect. On disconnect H4P tries with credentials on globals, and on connect the H4P overrides the existing WiFi credentials at globals by ones of the succeeded connection. Therefore the `Go=1` submission is mandatory for enforcing WiFi to change.

---

## Example sketches

* [MinimalServer](../examples/08_BLE/H4P_BLESrvMinimal/H4P_BLESrvMinimal.ino)

(c) 2024 Hamzah Hajeir 

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
