#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) 

#ifdef ESP32
#define D1_PIN 4
#define D2_PIN 16
#else // NodeMCU and RP2040
#define D1_PIN D1
#define D2_PIN D2
#endif

npPUBLISHONOFF enc2onoff;

class npENCODER10: public npNODE {
    public:    
        msg operator()(msg m) override {
          m.load=m.load < 1 ? 0:1;
          return enc2onoff(m);
        }
};

H4P_BinarySwitch bs(LED_BUILTIN,H4P_ASSUMED_SENSE);
h4pEncoder rotary(D1_PIN,D2_PIN,INPUT,ACTIVE_HIGH,true, new npENCODER10);