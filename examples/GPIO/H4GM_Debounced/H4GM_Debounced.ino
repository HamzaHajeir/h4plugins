#include<H4Plugins.h>
H4_USE_PLUGINS(115200,H4_Q_CAPACITY,false) // Serial baud rate, Q size, SerialCmd autostop

 //auto-start Serial @ 115200, Q size=20 

H4P_GPIOManager h4gm;
/*
My major testing devices were nodeMCU which has builtin button on GPIO0 which is ACTIVE_LOW
and STM32NUCLEO-F429ZI whuch has a user button that is ACTIVE_HIGH

You will probably need to adjust these values for you own device
*/
#define USER_BTN 0
#define UB_ACTIVE ACTIVE_LOW
/*
    ALL GPIO strategies are derived from H4GPIOPin: the following members are available
    inside ALL GPIO pin callbacks, once you have a valid pointer for the pin type using 
    H4GM_PIN( type ) with specific underlying type Raw needs H4GM_PIN(Raw);

    uint8_t         pin=0;                  // GPIO hardware pin number
    uint8_t         gpioType=0;             // INPUT, INPUT_PULLUP, OUTPUT etc
    H4GM_STYLE      style;                  // Strategy: Raw, debounced, retriggering etc
    uint8_t         sense=0;                // active HIGH or LOW
    unsigned long   Tevt=0;                 // uS time of last event
    int             state=0;                // 32 wide as it holds analog value as well as digital 0/1
                                            // and not uint because encoder returns -1 or +1
    uint32_t        delta=0;                // uS since last change
    uint32_t        rate=0;                 // instantaenous rate cps
    uint32_t        Rpeak=0;                // peak rate   
    uint32_t        cps=0;                  // sigma changes/sec (used internally, frequently re-set)
    uint32_t        cMax=UINT_MAX;          // max permitted cps (see "throttling");
    uint32_t        nEvents=UINT_MAX;       // sigma "Events" (meaning depends on inheriting strategy)
            
    Additional members for Debounced:

    NONE

        Debounced passes only "stable" LOGICAL transitions to the callback
        "Stable" means here that any transtions after the first are ignored if they last for less than dbTimeMs
        Choosing the correct vale for dbTimeMs depends on the switch in use. Getting the best balance between
        a low value for the smallest latency and a high value for "noisy" buttons can be trial-and-error if you 
        don't have a 'scope and EE experience. 

        Now though, you can just tweak dbTimeMs until you only ever get 1x ON and 1x OFF per button press

*/
// Debouncing time in mS
#define U_DBTIME_MS    12

void h4setup() { // H4 constructor starts Serial
    Serial.println("H4P_GPIOManager Debounced Example v"H4P_VERSION);
    Serial.print("GPIO ");Serial.print(USER_BTN);Serial.print(" ACTIVE ");Serial.println(UB_ACTIVE ? "HIGH":"LOW");

    h4gm.Debounced(USER_BTN,INPUT,UB_ACTIVE,U_DBTIME_MS,[](H4GPIOPin* ptr){
        H4GM_PIN(Debounced); // Required! turns ptr into correct pin-> pointer
        Serial.print("GPIO ");Serial.print(pin->pin);Serial.print(" state ");Serial.print(pin->state);
        Serial.print(" @ uS ");Serial.print(pin->Tevt);Serial.print(" delta=");Serial.print(pin->delta);
        Serial.print(" rate=");Serial.print(pin->cps);Serial.println("/sec");
    });
}