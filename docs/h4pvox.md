![H4P Flyer](../assets/H4PLogo.png)

# H4P_Voice

Provides individual voice control for music playback with H4P_ToneController.

---

## Contents

* [Usage](#usage)
* [API](#api)

---

# Usage

```cpp
H4P_Voice voice(5); // Create voice on pin 5
voice.play("C4 4 G4 4"); // Play notes
```

H4P_Voice works in conjunction with H4P_ToneController for multi-voice music playback.

---

## API

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

See examples in the [examples/](../examples/) folder for voice and music playback.

---

(c) 2021 Phil Bowles
(c) 2025 Hamza Hajeir

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9BUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
* [Support me on Patreon](https://patreon.com/esparto)