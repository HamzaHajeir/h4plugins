![NODE PINK](/assets/nodepink.jpg)
![H4P Flyer](/assets/GPIOLogo.jpg)


# GPIO Handling and "NODE-PINK" with analog inputs

# h4pRaw

More "an absence of flow", it has a single node, `npPUBLISHVALUE` giving the effect that every single transition sends a `H4PE_GPIO` event to the user's code, i.e. the signal is "raw" and unprocessed.

## Pipeline

 `npPUBLISHVALUE`

## Simplified Declaration

```cpp
h4pRaw(uint8_t p,uint8_t m,H4PM_SENSE s);

```

[Example Sketch](../examples/01_GPIO_PIN_MACHINE/PM_01_Raw/PM_01_Raw.ino)

---

## AnalogAverage

Averages analog readings over a specified number of samples before publishing.

### Pipeline

`npPassTimer` -> `npAVERAGE{n}` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogAverage(uint8_t p, uint32_t f, uint32_t n, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `n`: Number of samples to average
- `d`: Destination node (default: npPublishValue)

---

## AnalogAvgChanged

Averages analog readings and only publishes when the average changes by more than a hysteresis value.

### Pipeline

`npPassTimer` -> `npAVERAGE{n}` -> `npVALUEDIFF{h}` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogAvgChanged(uint8_t p, uint32_t f, uint32_t n, uint32_t h, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `n`: Number of samples to average
- `h`: Hysteresis value
- `d`: Destination node (default: npPublishValue)

---

## AnalogAvgRolling

Maintains a rolling average of analog readings.

### Pipeline

`npPassTimer` -> `npROLLINGAVERAGE` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogAvgRolling(uint8_t p, uint32_t f, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `d`: Destination node (default: npPublishValue)

---

## AnalogAvgWindow

Averages analog readings over a sliding window of samples.

### Pipeline

`npPassTimer` -> `npAVGWINDOW{w}` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogAvgWindow(uint8_t p, uint32_t f, uint32_t w, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `w`: Window size
- `d`: Destination node (default: npPublishValue)

---

## AnalogPolled

Polls analog input and publishes only when value changes by more than hysteresis.

### Pipeline

`npPassTimer` -> `npVALUEDIFF{h}` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogPolled(uint8_t p, uint32_t f, uint32_t h, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `h`: Hysteresis value
- `d`: Destination node (default: npPublishValue)

---

## AnalogThreshold

Polls analog input and publishes only when value crosses a threshold.

### Pipeline

`npPassTimer` -> `npVALUEDIFF{h}` -> `npTHRESHOLD{lim,cmp}` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogThreshold(uint8_t p, uint32_t f, H4PM_COMPARE cmp, uint32_t lim, uint32_t h, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `cmp`: Comparison operator (H4PM_GREATER, H4PM_LESS, etc.)
- `lim`: Threshold value
- `h`: Hysteresis value
- `d`: Destination node (default: npPublishValue)

---

## AnalogTMP36

Reads TMP36 temperature sensor and converts to Celsius (tenths of degrees).

### Pipeline

`npPassTimer` -> `npVALUEDIFF{h}` -> `npANALOGTMP36` -> `npPUBLISHVALUE`

### Simplified Declaration

```cpp
h4pAnalogTMP36(uint8_t p, uint32_t f, uint32_t h, npNODE* d = npPublishValue);
```

Parameters:
- `p`: Pin number
- `f`: Polling frequency in ms
- `h`: Hysteresis value
- `d`: Destination node (default: npPublishValue)

Note: Output is in tenths of degrees Celsius (e.g., 250 = 25.0°C)

---

(c) 2021 Phil Bowles
(c) 2025 Hamza Hajeir

* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
* [Support me on Patreon](https://patreon.com/esparto)