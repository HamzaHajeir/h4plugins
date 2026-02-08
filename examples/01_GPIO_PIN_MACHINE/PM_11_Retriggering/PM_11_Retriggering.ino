#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) 

#ifdef ESP32
#define RET_PIN 4
#else // NodeMCU and RP2040
#define RET_PIN D2
#endif

h4pOutput builtinLED(LED_BUILTIN, H4P_ASSUMED_SENSE,OFF);

H4P_EventListener gpio(H4PE_GPIO,[](const std::string& pin,H4PE_TYPE t,const std::string& msg){
    int p=atoi(pin.c_str());
    int v=atoi(msg.c_str());
    switch(p){
      case RET_PIN:
        Serial.printf("P%d V%d\n",p,v);
        builtinLED.logicalWrite(v); // make LED match state of pin
        break;
    }
});

h4pRetriggering hair(RET_PIN,INPUT,ACTIVE_HIGH,10000);