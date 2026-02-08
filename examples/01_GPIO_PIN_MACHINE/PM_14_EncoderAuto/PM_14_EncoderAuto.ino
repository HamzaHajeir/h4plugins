#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) 

#define USER_BTN  0

#ifdef ESP32
#define D1_PIN 4
#define D2_PIN 16
#else // NodeMCU and RP2040
#define D1_PIN D1
#define D2_PIN D2
#endif

// Range 0-100 in increments of 1, start position 50% NO WRAP AROUND (hard stop @ end)
h4pEncoderAuto rotary(D1_PIN,D2_PIN,INPUT,ACTIVE_HIGH,0,100,1,50,false);
h4pDebounced centerbutton(USER_BTN,INPUT,H4P_ASSUMED_SENSE,15); // 15 ms debounce time

H4P_EventListener gpio(H4PE_GPIO,[](const std::string& pin,H4PE_TYPE t,const std::string& msg){
    int p=atoi(pin.c_str());
    int v=atoi(msg.c_str());
    switch(p){
      case USER_BTN:
        rotary.center();
        Serial.printf("CENTERED: value is %d\n",rotary.getValue());
        break;
      case D1_PIN:
        Serial.printf("P%d V = %d\n",p,v);
        break;
    }
});