![H4P Logo](../assets/DiagLogo.jpg)

# Known Issues / Frequently-asked Questions

---

# Known issues

## webUI

There are a number of issues with the web user interface. They are due to 2 factors:

1. On a tiny ESP8266, simply having a dynamic real-time updating webUI is pushing the limits of physics.

As a result there are a number of factors to take into account:

* The UI can take a little time to load. It does this deliberately to avoid overloading H4AsyncWebserver and causing a crash. If the user has added many extra user fields, be patient - hitting refresh will just double the workload, making a crash even *more* likely
  
* If you leave the browser open during development, and you re-upload a different or updated sketch - the browser and sketch are then "out-of-step" and this can (and frequently does) lead to "random" crashes. Simple answer: shut the browser down, and reopen on the newly loaded sketch.
  
* Try to minimise the number of dynamically updating fields you add.
  
  
Regular updates e.g. 1x per second "Up Time" are anything but smooth.. they will jump / stutter / lag etc as AsyncWebserver fills its queue and the webUI has to buffer (or ultimately abandon) its updates to avoid a crash.

## H4P_AsyncHTTP

Is known to be very pernickety about the type of JSON it will decode  correctly without crashing. It is a deliberately "cheap n cheezy" implemention so thaty your own internal websuites can send small, simple data cheaply and rapidly. For anything more serious, use a proper library like [ArduinoJson](https://arduinojson.org/) 

There are also some sites that behave oddly when chunking data, so incomplete / strange results may occur ion some sites. This is a "feature" of the underlying library [ArmadilloHTTP](https://github.com/hamzahajeir/ArmadilloHTTP) and will be fixed in a later release.

That library also does not follow redirects - another item on its "TODO" list.

---

# Frequently-asked questions

## How do I get the device into AP mode?

Read [WiFi AP mode](apmode.md) H4Plugins does not need AP mode.

---

(c) 2026 H4Group

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)