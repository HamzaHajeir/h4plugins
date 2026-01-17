![H4P Flyer](../assets/H4PLogo.png)

# H4P_Timekeeper

## Service shortname tk

Provides NTP time synchronisation, timezone and DST support, scheduling and time-related utilities.

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
H4P_Timekeeper tk("pool.ntp.org","time.nist.gov",0,H4P_Timekeeper::H4P_DST_EU);
```

This plugin is a "singleton" - there may be only one single instance of it in the app. 
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

# Dependencies

* H4P_EmitTick
* H4P_WiFi

# Commands Added

| Command | Parameters | Purpose |
|---------|------------|---------|
| at | time,onoff | Schedule a one-shot alarm at `time` (HH:MM:SS) to set state to `onoff` (0=OFF,1=ON) |
| change | ntp1,ntp2 | Change NTP servers to `ntp1` and `ntp2` |
| daily | time,onoff | Schedule a daily alarm at `time` (HH:MM:SS) to set state to `onoff` (0=OFF,1=ON) |
| sync | - | Force immediate NTP synchronisation |
| tz | offset | Set timezone offset in minutes from UTC |

# Service Commands

`stop` will disconnect from Timekeeper and initiate closedown of all Plugins the depend on Timekeeper
`start` will connect to Timekeeper and start NTP sync and all dependent services

---

## API

```cpp
/*
Constructor
*/
H4P_Timekeeper(const std::string& ntp1, const std::string& ntp2, int tzOffset=0, H4_FN_DST fDST=nullptr);

/*
Types
*/
using H4P_DURATION = std::pair<std::string,std::string>; // pair of start and end times as strings
using H4P_SCHEDULE = std::vector<H4P_DURATION>; // vector of durations for scheduling
using H4_FN_DST = std::function<int(uint32_t)>; // function to calculate DST offset in seconds

/*
Time functions
*/
uint32_t clockEPOCH(); // Current number of seconds since EPOCH (0 if not synced)
uint32_t clockEPOCHLocal(); // Current number of seconds since EPOCH plus TZ offset
std::string clockStrTimeLocal(); // Current time + TZ as std::string HH:MM:SS
std::string strfTime(uint32_t t); // Format seconds t as HH:MM:SS
std::string strfDate(uint32_t t); // Format seconds t as YYYY-MM-DD
std::string strfDateTime(const char* fmt = "%a %Y-%m-%d %H:%M:%S", uint32_t t=0); // Format seconds t with custom format
std::string clockTime(); // Current time as HH:MM:SS or "0" if not synced
uint32_t msSinceMidnight(); // Milliseconds since midnight
std::string strTime(uint32_t t); // Format milliseconds t as HH:MM:SS
std::string upTime(); // Uptime as DD:HH:MM:SS

/*
DST functions
*/
static int H4P_DST_EU(uint32_t t); // DST offset for EU (inc UK) in seconds
static int H4P_DST_USA(uint32_t t); // DST offset for USA in seconds

/*
Scheduling
*/
void at(const std::string& when, bool onoff); // Schedule one-shot alarm
void daily(const std::string& when, bool onoff); // Schedule daily alarm
void setSchedule(H4P_SCHEDULE); // Set multiple on/off times

/*
Utilities
*/
int parseTime(const std::string& ts); // Parse HH:MM:SS to milliseconds, -1 on error
std::string minutesFromNow(uint32_t m); // Time m minutes from now as HH:MM

/*
Configuration
*/
void change(const std::string& ntp1, const std::string& ntp2); // Change NTP servers
void sync(); // Force NTP sync
void tz(int tzOffset); // Set timezone offset in minutes
```

# Example sketches

See examples in the [examples/](../examples/) folder for NTP and scheduling usage.

---

(c) 2021 Phil Bowles
(c) 2025 Hamza Hajeir

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9BUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
* [Support me on Patreon](https://patreon.com/esparto)