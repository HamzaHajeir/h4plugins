/*
MIT License

Copyright (c) 2026 H4Group

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Contact Email: TBD
*/
#include<H4P_SerialCmd.h>
#include<H4P_AsyncMQTT.h>
#include<H4P_BLEServer.h>

// payload: scheme,broker,uname,pws,port e.g. https,192.168.1.4,,,1883
uint32_t H4P_AsyncMQTT::_change(std::vector<std::string> vs){  // broker,uname,pword,port
    return _guard1(vs,[this](std::vector<std::string> vs){
        auto vg=split(H4PAYLOAD,",");
        if(vg.size()!=5) return H4_CMD_PAYLOAD_FORMAT;
        std::string url=vg[0]+"://"+vg[1]+":"+vg[4];
        change(url,vg[2],vg[3]); // change this to a vs?
        return H4_CMD_OK;
    });
}

void H4P_AsyncMQTT::_handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg){ 
    switch(t){
        case H4PE_BLESINIT:
            {
                if (h4p[brokerTag()].get().empty()){
                    h4pbleAdd(brokerTag(), H4P_UI_INPUT,"m");
                    h4pbleAdd(mQuserTag(), H4P_UI_INPUT,"m");
                    h4pbleAdd(mQpassTag(), H4P_UI_INPUT,"m");
                } else {
                    h4pbleAdd(brokerTag(),H4P_UI_TEXT,"m",h4p[brokerTag()]);
                }
                
                h4pbleAdd(_me,H4P_UI_BOOL,"m","",H4P_UILED_BI);
                h4pbleAdd(nDCXTag(),H4P_UI_TEXT,"m"); // cos we don't know it yet
            }
        break;
        case H4PE_VIEWERS:
            {
                uint32_t mode=STOI(msg);
                if(mode) {
                #if H4P_USE_WIFI_AP
                    if((WiFi.getMode()==WIFI_AP || WiFi.getMode()==WIFI_AP_STA) && h4p.gvExists(GoTag())){
                        h4puiAdd(brokerTag(),H4P_UI_INPUT,"m");
                        h4puiAdd(mQuserTag(),H4P_UI_INPUT,"m");
                        h4puiAdd(mQpassTag(),H4P_UI_INPUT,"m");
                    }
                    else {
                        h4puiAdd(_me,H4P_UI_BOOL,"m","",H4P_UILED_BI);
                        h4puiAdd(brokerTag(),H4P_UI_TEXT,"m");
                        h4puiAdd(nDCXTag(),H4P_UI_TEXT,"m"); // cos we don't know it yet
                    }
                #else
                    h4puiAdd(_me,H4P_UI_BOOL,"m","",H4P_UILED_BI);
                    h4puiAdd(brokerTag(),H4P_UI_TEXT,"m");
                    h4puiAdd(nDCXTag(),H4P_UI_TEXT,"m"); // cos we don't know it yet
                #endif
                }
            }
            break;
        case H4PE_GVCHANGE:
//            if((svc==brokerTag()) && _running) {
            if(svc==brokerTag()) {
//                Serial.printf("RESTARTING DUE TO GVCHANGE\n");
                restart();
                break;
            }
            if(svc==stateTag()) {
                publishDevice(stateTag(),msg);
                break;
            }
        default:
            break;
    }
}

void H4P_AsyncMQTT::_init() {
    h4p.gvSetInt(_me,0,false);
    onError([this](int e,int info){ XEVENT(H4PE_SYSWARN,"%d,%d",e,info); });

    std::string device=h4p[deviceTag()];
    if(_lwt.topic=="") {
        _lwt.topic=std::string(prefix+device+"/offline");
        _lwt.payload=h4p[chipTag()];
    }

    setWill(_lwt.topic.c_str(),_lwt.QOS,_lwt.payload.c_str(),_lwt.retain);
    prefix+=device+"/";

    onMessage([=](const char* topic, const uint8_t* payload, size_t length, H4AMC_MessageOptions opts){
        std::string top(topic);
        std::string pload((const char*) payload,length);
#if MQTT5
        auto props = opts.getProperties();
        // props.xxx
#endif
        h4.queueFunction([top,pload](){ h4p._executeCmd(CSTR(std::string(mqttTag()).append("/").append(top)),pload); },nullptr,H4P_TRID_MQMS);
    });

    onConnect([=, this](H4AMC_ConnackParam params){
        // Serial.printf("MQTT::onConnect()\n");
        _connected =true;
        h4.queueFunction([=, this](){
            // Serial.printf("MQTT::queuedonConnect()\n");
            _signalOff();
            h4.cancelSingleton(H4P_TRID_MQRC);
            if (!params.session) {
                subscribe(CSTR(std::string(allTag()).append(cmdhash())),0);
                subscribe(CSTR(std::string(device+cmdhash())),0);
                subscribe(CSTR(std::string(h4p[chipTag()]+cmdhash())),0);
                subscribe(CSTR(std::string(h4p[boardTag()]+cmdhash())),0);
            }
            report();
            h4p[_me]=stringFromInt(_running=true);
            SYSINFO("CNX %s",CSTR(h4p[brokerTag()]));
            H4Service::svcUp();

#if H4P_BLE_AVAILABLE
            auto blesrv = h4puncheckedcall<H4P_BLEServer>(blesrvTag());
            if (blesrv) {
                blesrv->elemRemove(brokerTag());
                blesrv->elemRemove(mQuserTag());
                blesrv->elemRemove(mQpassTag());
                h4pbleAdd(brokerTag(),H4P_UI_TEXT,"m",h4p[brokerTag()]);

                blesrv->sendElems();
            }
#endif
        });
    });

    onDisconnect([this](){
        // Serial.printf("MQTT::onDisconnect()\n");
        _connected = false;
        h4.queueFunction([this](){
            // Serial.printf("MQTT::queuedonDisconnect()\n");
            h4p[_me]=stringFromInt(_running=false);
            h4p.gvInc(nDCXTag());
            _signalBad();
            SYSINFO("%s", "DCX");
            H4Service::svcDown();
            if(autorestart) { 
                h4.every(H4MQ_RETRY,[this](){
                    XLOG("MQTT hasn't reconnected yet WiFi.status()=%d", WiFi.status());
                    if (WiFi.status()==WL_CONNECTED)
                        _signalBad(); // have to repeat to override hb if present: easiest to NIKE
                    }
                    ,nullptr,H4P_TRID_MQRC,true); 
        }});
    });
}

/* void H4P_AsyncMQTT::_setup(){ // allow for TLS
    // if(h4p[brokerTag()]!="") setServer(CSTR(h4p[brokerTag()]),CSTR(h4p[mQuserTag()]),CSTR(h4p[mQpassTag()])); // optimise tag()
//    else SYSWARN("NO MQTT DETAILS","");
} */

void H4P_AsyncMQTT::change(const std::string& broker,const std::string& user,const std::string& passwd){ // add creds
    XLOG("MQTT change to %s user=%s",CSTR(broker),CSTR(user));
    h4p[mQuserTag()]=user;
    h4p[mQpassTag()]=passwd;
    h4p[brokerTag()]=broker;
}

#if H4P_LOG_MESSAGES
void H4P_AsyncMQTT::info(){
    H4Service::info();
    reply(" Server: %s, %s",CSTR(h4p[brokerTag()]),_connected ? "CNX":"DCX");
    std::string reporting;
    for(auto const r:_reportList) reporting+=r+",";
    reporting.pop_back();
    reply(" Report: %s",CSTR(reporting));
}
#endif

void H4P_AsyncMQTT::report(){
    std::string j="{";
    for(auto const &r:_reportList) {
        j+="\""+r+"\":\"";
        j.append(h4p[r]).append("\",");
    }
    j.pop_back();
    publishDevice(reportTag(),j+"}", 1, true);
    XLOG("Reporting %s}",CSTR(j));
}

void H4P_AsyncMQTT::subscribeDevice(std::string topic,H4_FN_MSG f,H4PC_CMD_ID root){
    std::string fullTopic=std::string(h4p[deviceTag()])+"/"+topic; // refac
    if(topic.back()=='#' || topic.back()=='+'){
        topic.pop_back();
        topic.pop_back();
    }
    h4p.addCmd(topic,root,0,f);
    subscribe(CSTR(fullTopic),0);
    XLOG("Subscribed to %s",CSTR(fullTopic));
}

void H4P_AsyncMQTT::subscribeDevice(std::initializer_list<std::string> topic,H4_FN_MSG f,H4PC_CMD_ID root){ for(auto const& t:topic) subscribeDevice(t,f,root); }

void H4P_AsyncMQTT::svcUp(){
#if H4P_USE_WIFI_AP
    if(WiFi.getMode()==WIFI_AP) return;
#endif    
    _signalBad();
    // _setup();
    autorestart=true;
    informNetworkState(H4AMC_NETWORK_CONNECTED);
    connect(CSTR(h4p[brokerTag()]), CSTR(h4p[mQuserTag()]), CSTR(h4p[mQpassTag()]), CSTR(h4p[deviceTag()]));
}

void H4P_AsyncMQTT::svcDown(){
    autorestart=false;
    if (_connected)
        disconnect();
    informNetworkState(H4AMC_NETWORK_DISCONNECTED); // May introduce issues regarding conflicting H4AMC state and the network state.
}

void H4P_AsyncMQTT::unsubscribeDevice(std::string topic){
    std::string fullTopic=std::string(h4p[deviceTag()])+"/"+topic; // refac
    if(topic.back()=='#'){
        topic.pop_back();
        topic.pop_back();
    }
    h4p.removeCmd(topic);
    unsubscribe(CSTR(fullTopic));
    XLOG("Unsubscribed from %s\n",CSTR(topic));
}

void H4P_AsyncMQTT::unsubscribeDevice(std::initializer_list<std::string> topic){ for(auto const& t:topic) unsubscribeDevice(t); }