![H4P Logo](../assets/DiagLogo.jpg)

# Is it actually an "issue"?

"Issues" should be restricted to code not behaving as described, crashes, confusing / unclear / broken documentation etc. Keep it to things that simply *just don't work*, **not** "Things that don't work the way *you* would like them to or think they *should*".

# Genuine Issues

First checkout issues section for similar, if it's a geniune issue, raise one providing complete set of information needed to know and duplicate the issue in our side. 

---

# Mandatory steps

1. Read *all* of the relevant documentation for the plugins you are using.

2. Check the [Known Issues / FAQs](faq.md)

3. Provide the type / name of the development board you are using and the major Tools / Build settings from ArduinoIDE.  
![settings](../assets/settings.jpg)

Alternatively, provide complete configuration file of your PlatformIO project `platformio.ini`.

4. If the hardware is a homebuild, include schematic or circuit diagram.


5. Tick the `H4P_LOG_MESSAGES` macro through either editing the active [config_plugins.h](../src/config_plugins.h) and ensuring `H4P_LOG_MESSAGES` is set to 1 like so:  
```cpp
#define H4P_LOG_MESSAGES        1
```
- OR by inserting an environment build flag passed to the compiler, as stating in `platformio.ini` the following:
```ini
build_flags = -DH4P_LOG_MESSAGES=1
```
- And finally enable the serial logger:

```cpp
#define H4P_VERBOSE 1
H4_USE_PLUGINS(115200,30,false)
H4P_SerialLogger sl;
```

7. Include the cut and pasted *text* of the entire serial monitor output from the start of the boot.

8. Include the *entire code* of the **MVCE** (see below) that shows the error. 

9. If the problem includes a crash, you ***must*** provide a ***decoded*** stack trace. 
  Hint: you can monitor the serial using platformio, automatically decoding traces within the log by inserting a certain filter in you monitoring command `pio device monitor -f esp32_exception_decoder` or `esp8266_exception_decoder`. Alternatively you can have these filters within your configuration file:
  ```ini
  monitor_filters = esp32_exception_decoder
	time
	log2file
  ```

---

# Your code - MVCE

Your code should be a **M**inimal, **C**omplete and **V**erifiable **E**xample (**MCVE**).

This means that I need to see *everything* ("complete") to be able to reproduce the problem, but I need "*everything*" to be as small as possible ("minimal"). Of course it must also actually show the problem clearly ("Verifiable").


## What's actually in an MCVE?

More important is what is *not* in it: and that is *anything* that does not contribute or work towards causing the problem. If I can take out *any* code and still compile the sketch and still have it show the problem, then it is *not* "minimal" and you should have already removed that code. 

---

(c) 2026 H4Group

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
