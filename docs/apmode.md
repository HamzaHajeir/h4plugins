
![H4P Flyer](../assets/WiFiLogo.jpg)

# WiFi AP Mode

H4Plugins supports WiFi Access Point (AP) mode for initial device configuration, allowing users to connect and set up WiFi credentials without pre-programming them.

## Enabling AP Mode

AP mode is enabled by default when `H4P_USE_WIFI_AP` is defined (which it is by default in `config_plugins.h`). No additional code changes are required.

## Entering AP Mode

The device enters AP mode automatically under these conditions:

1. **Factory Reset**: When the device is factory reset (SPIFFS wiped), it defaults to AP mode
2. **Invalid WiFi Credentials**: If the stored SSID is set to `"H4"` or password is `"H4"`
3. **GPIO Button**: Long press on a configured GPIO button (if using `H4P_MultifunctionButton`)
4. **Command**: Send `h4/factory` via Serial, MQTT, or HTTP REST

## AP Mode Operation

When in AP mode:

- The device creates a WiFi access point named after the device (default: `"H4"`)
- IP address: `192.168.4.1`
- A DNS server redirects all requests to the device's web interface
- The web UI provides configuration fields for:
  - WiFi SSID selection (scanned networks)
  - WiFi password
  - Device name
  - MQTT broker (if applicable)
  - Remote update server (if applicable)

## Configuration Process

1. Connect to the AP using a phone, tablet, or computer
2. Open a web browser and navigate to any URL (DNS redirects to `192.168.4.1`)
3. Fill in the configuration fields:
   - Select your WiFi network from the dropdown
   - Enter the WiFi password
   - Optionally change the device name
   - Configure MQTT and update server if needed
4. Click the "Go" button to save settings and reboot
5. The device will connect to your WiFi network and exit AP mode

## Programmatic Control

You can force AP mode programmatically:

```cpp
// Set SSID to "H4" to trigger AP mode on next boot
h4p.gvSetString(ssidTag(), "H4");

// Or use the factory reset command
h4.once(0, [](){ h4psysevent("factory"); });
```

## Security Notes

- AP mode uses an open network by default
- The web interface is served over HTTP (not HTTPS)
- After configuration, the device switches to STA mode and AP mode is disabled until triggered again

---

## Example Sketch

```cpp
#include <H4Plugins.h>

H4_USE_PLUGINS(115200, 20, false); // Serial baud, Q size, SerialCmd autostop

H4P_WiFi wifi; // No credentials - will enter AP mode if none stored

void h4setup() {
    // Device will automatically enter AP mode if no valid WiFi credentials
}
```

---

(c) 2021 Phil Bowles
(c) 2025 Hamza Hajeir

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
* [Support me on Patreon](https://patreon.com/esparto)