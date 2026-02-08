#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) 

#ifdef ESP32
#define D1_PIN 4
#define D2_PIN 16
#else // NodeMCU and RP2040
#define D1_PIN D1
#define D2_PIN D2
#endif

H4P_EventListener gpio(H4PE_GPIO,[](const std::string& pin,H4PE_TYPE t,const std::string& msg){
    int p=atoi(pin.c_str());
    int v=atoi(msg.c_str());
    switch(p){
      case D1_PIN:
        Serial.printf("P%d V = %d\n",p,v);
        break;
    }
});

class npINCREMENT: public npNODE {
        int&        _i;
    public:    
        npINCREMENT(int& I): _i(I){}

        msg operator()(msg m) override {
          _i+=m.load;
          return m;
        }
};

int position=0;

h4pEncoder rotary(D1_PIN,D2_PIN,INPUT,ACTIVE_HIGH,true, new npINCREMENT{position});

void h4setup(){
  h4.every(1000,[]{ Serial.printf("Position=%d\n",position); });
}