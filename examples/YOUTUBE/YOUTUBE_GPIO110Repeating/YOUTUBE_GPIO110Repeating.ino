#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) 

h4pRepeating dream(0,INPUT_PULLUP,ACTIVE_LOW,20,1000);

void onGPIO(int pin,int value){ 
    Serial.printf("P%d V%d\n",pin,value);
}

void h4pGlobalEventHandler(const string& svc,H4PE_TYPE t,const string& msg){
    switch(t){
        H4P_FUNCTION_ADAPTER_GPIO;
    }
}