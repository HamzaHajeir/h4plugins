![H4P Flyer](../assets/MQTTLogo.jpg)

# H4P_LinkMaster

## Shortname link

Allows "slaving" of other H4 devices to create simple groups / scenarios. Switch on/off the master and all slaves also switch on/off.

---

## Contents

* [Usage](#usage)
* [Dependencies](#dependencies)
* [Commands Added](#commands-added)
* [Service Commands](#service-commands)
* [API](#api)

---

# Usage

```cpp
H4P_LinkMaster linkmaster;
```

This plugin is a "singleton" - there may be only one single instance of it in the app.
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

# Dependencies

[H4P_AsyncMQTT](h4mqtt.md)

# Commands Added

* `h4/slave/D,N` (payload D = name of other H4 device on the network, N=1 => add slave; 0 => remove slave). Added only if [H4P_AsyncMQTT](h4mqtt.md) loaded

# Service Commands

`stop` / `start` have no effect

## Callback functions

N/A

---

# API

```cpp
/*
Constructor
slaved = initial set of slave device names
*/
H4P_LinkMaster(std::unordered_set<std::string> slaved={});

void slave(const std::string& otherh4,bool inout=true); // Add or remove a slave device
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
