![H4P Flyer](../assets/H4PLogo.png)

# H4P_ToneController

## Service shortname tune

Provides tone generation, music playback, and siren sounds using PWM.

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
H4P_ToneController tc(120); // Set tempo to 120 BPM
```

This plugin is a "singleton" - there may be only one single instance of it in the app. 
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

# Dependencies

N/A

# Commands Added

N/A

# Service Commands

N/A
<!-- 
`stop` will disconnect from ToneController and initiate closedown of all Plugins the depend on ToneController
`start` will connect to ToneController and initialise tone system

* 'stop' and 'start' APIs were not verified or tested. -->
---

## API

```cpp
/*
Constructor
*/
H4P_ToneController(uint32_t tempo=60); // Set default tempo in BPM

/*
Enums
*/
enum H4P_SIREN {
    H4P_SIREN_BUZZ,
    H4P_SIREN_CHIRP,
    H4P_SIREN_HILO,
    H4P_SIREN_SCREECH,
    H4P_SIREN_WOOPWOOP,
    H4P_SIREN_MAX
};

/*
Types
*/
using H4P_STAVE = std::pair<H4P_Voice&, const std::string&>; // Voice and tune pair
using H4P_TUNE = std::vector<H4P_STAVE>; // Multi-voice tune

/*
Static methods
*/
static uint32_t length(const std::string& tune); // Calculate tune duration in ms
static void metronome(uint32_t tempo); // Set global tempo
static void multiVox(H4P_TUNE tune, uint32_t tempo, int transpose=0); // Play multi-voice tune
static std::string setVolume(const std::string& s, uint8_t volume); // Adjust tune volume
static void siren(H4P_SIREN S, uint8_t pin, uint32_t duration); // Play siren on pin
static void tone(uint8_t pin, uint32_t freq, uint32_t duration=0, uint8_t volume=8); // Play tone

/*
Macros
*/
#define sirenBuzz(p,d) siren(H4P_SIREN_BUZZ,p,d)
#define sirenChirp(p,d) siren(H4P_SIREN_CHIRP,p,d)
#define sirenHiLo(p,d) siren(H4P_SIREN_HILO,p,d)
#define sirenScreech(p,d) siren(H4P_SIREN_SCREECH,p,d)
#define sirenWoopWoop(p,d) siren(H4P_SIREN_WOOPWOOP,p,d)
```

## H4P_Voice

```cpp
/*
Constructor
*/
H4P_Voice(uint8_t pin, uint8_t col=H4P_UILED_BI); // Create voice on pin with UI color

/*
Methods
*/
void play(const std::string& tune, int transpose=0); // Play tune with optional transpose
void rest(const char duration); // Play rest of given duration
```

# Example sketches

See examples in the [examples/](../examples/) folder for tone and music playback.

---

(c) 2021 Phil Bowles
(c) 2025 Hamza Hajeir

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9BUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
* [Support me on Patreon](https://patreon.com/esparto)
