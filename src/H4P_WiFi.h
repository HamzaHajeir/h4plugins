/*
 MIT License

Copyright (c) 2020 Phil Bowles <H48266@gmail.com>
   github     https://github.com/philbowles/H4
   blog       https://8266iot.blogspot.com
   groups     https://www.facebook.com/groups/esp8266questions/
              https://www.facebook.com/H4-Esp8266-Firmware-Support-2338535503093896/


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#pragma once

#include<H4Service.h>

#include<H4AsyncTCP.h>
#ifdef ARDUINO_ARCH_ESP8266
    #include<ESP8266WiFi.h>
    #include<ESP8266mDNS.h>
    // #include<ESPAsyncTCP.h>
    #include<ESPAsyncUDP.h>
#else
    #include<WiFi.h>
    #include<AsyncUDP.h>
    #include<ESPmDNS.h>
    #include<map> // WHY???
#endif
#include<DNSServer.h>
#include<ArduinoOTA.h>
#include<H4P_SerialCmd.h>
#include<H4AsyncWebServer.h>
#include<H4P_Signaller.h>
//
#if H4P_USE_WIFI_AP || H4P_WIFI_PROV_BY_BLE
STAG(Go);
#endif

class H4P_WiFi: public H4Service, public H4AsyncWebServer {
#if H4P_USE_WIFI_AP
            DNSServer*          _dns53=nullptr;
#endif
#ifdef ARDUINO_ARCH_ESP8266
            bool                _shouldStart=false;
#endif
//
            // bool                _discoDone;
            bool                    _connected;
            std::vector<H4_FN_VOID> _onWebserver;
            H4AW_Authenticator*     _authenticator;
            uint32_t            _evtID=0;
            H4AW_HTTPHandlerWS*      _ws;
            H4AT_NVP_MAP        _lookup = {
                {"device", h4p[deviceTag()]}};

            std::vector<std::string>      _lines={};
//
                VSCMD(_change);
                VSCMD(_msg);

                void            _hookBLEProvisioning();
                void            _unhookBLEProvisioning();
#if H4P_USE_WIFI_AP | H4P_WIFI_PROV_BY_BLE
                bool            bleserver;
                void            _startScan();
                void            _stopScan();
#endif
                void            HAL_WIFI_disconnect();
                void            HAL_WIFI_setHost(const std::string& host);
//
                void            __uiAdd(const std::string& msg);

        // static  String          _aswsReplace(const String& var);
                void            _clearUI();
                bool            _cannotConnectSTA(){ return (WiFi.SSID()==h4Tag() || WiFi.psk()==h4Tag())/*  || std::stoi(h4p[GoTag()]) == 0 */; }
                void            _commonStartup();
                void            _coreStart();
                void            _defaultSync(const std::string& svc,const std::string& msg);
                void            _gotIP();
                void            _lostIP();
                void            _rest(H4AW_HTTPHandler *handler);
                std::string     _execute(const std::string& msg);
                void            _restart();
                void            _signalBad();
                void            _startWebserver();
                void            _stopWebserver();
        static  void            _wifiEvent(WiFiEvent_t event);
#if H4P_USE_WIFI_AP
                void            _apViewers();
#endif
//      service essentials
    protected:
        virtual void            _handleEvent(const std::string& s,H4PE_TYPE t,const std::string& msg) override;
    public:
                void            HAL_WIFI_startSTA(); // Has to be static for bizarre start sequence on ESP32 FFS


#if H4P_USE_WIFI_AP
                void            _startAP();
        H4P_WiFi(const std::string& device=""): 
            H4Service(wifiTag(),H4PE_FACTORY | H4PE_VIEWERS | H4PE_GPIO | H4PE_GVCHANGE | H4PE_UIADD | H4PE_UISYNC | H4PE_UIMSG | H4PE_BLESINIT),
            H4AsyncWebServer(H4P_WEBSERVER_PORT){
            h4p.gvSetstring(deviceTag(),device,true);
#else
        explicit H4P_WiFi(): H4Service(wifiTag()),H4AsyncWebServer(H4P_WEBSERVER_PORT){}

        H4P_WiFi(std::string ssid,std::string psk,std::string device=""):
            H4Service(wifiTag(),H4PE_FACTORY | H4PE_GPIO | H4PE_GVCHANGE | H4PE_UIADD | H4PE_UISYNC | H4PE_UIMSG | H4PE_BLESINIT),
            H4AsyncWebServer(H4P_WEBSERVER_PORT){
                h4p.gvSetstring(ssidTag(),ssid,true);
                h4p.gvSetstring(pskTag(),psk,true);
                h4p.gvSetstring(deviceTag(),device,true);
#endif
                _commonStartup();
            }
                void            change(std::string ssid,std::string psk);
#if H4P_LOG_MESSAGES
                void            info() override;
#endif

                void            hookWebserver(H4_FN_VOID f) { if (f!=nullptr) _onWebserver.push_back(f); }

                void            useSecurePort() { setPort(H4P_WEBSERVER_TLS_PORT); }
                void            useUnsecurePort() { setPort(H4P_WEBSERVER_PORT); }
                void            authenticate(const std::string& username, const std::string& password);
        virtual void            svcDown() override;
        virtual void            svcUp() override;
//
                void            uiAddBoolean(const std::string& name,const std::string& section="u"){ _uiAdd(name,H4P_UI_BOOL,section); }
                void            uiAddDropdown(const std::string& name,H4P_NVP_MAP options,const std::string& section="u");
                void            uiAddGlobal(const std::string& name,const std::string& section="u"){ _uiAdd(name,H4P_UI_TEXT,section); }
                void            uiAddImg(const std::string& name,const std::string& url,const std::string& section="u"){ _uiAdd(name,H4P_UI_IMG,section,url); }
                void            uiAddImgButton(const std::string& name,const std::string& section="u"){ _uiAdd(name,H4P_UI_IMGBTN,section); }
                void            uiAddInput(const std::string& name,const std::string& section="u"){ _uiAdd(name,H4P_UI_INPUT,section); }
                void            uiAddText(const std::string& name,const std::string& v,const std::string& section="u"){ _uiAdd(name,H4P_UI_TEXT,section,v); }
                void            uiAddText(const std::string& name,int v,const std::string& section="u"){ _uiAdd(name,H4P_UI_TEXT,section,stringFromInt(v)); }
                void            uiAddAllUsrFields(const std::string& section="u");

                void            uiSetValue(const std::string& ui,const int f){ _sendWS(ui,stringFromInt(f)); }
                void            uiSetValue(const std::string& ui,const std::string& value){ _sendWS(ui,value); }
//
                template<typename... Args>
                void            uiMessage(const std::string& msg, Args... args){ // variadic T<>
                    char* buff=static_cast<char*>(malloc(H4P_REPLY_BUFFER+1));
                    snprintf(buff,H4P_REPLY_BUFFER,CSTR(msg),args...);
                    _sendWS("",buff);
                    free(buff);
                }
//          syscall only        
        virtual void            _init() override;
                void            _reply(std::string msg) override { _lines.push_back(msg); }
                std::string     _concatMsg(const std::string& type, const std::string& msg) { return std::string(type).append(",").append(msg); }
                void            _sendWS(const std::string& type,const std::string& msg);
                void            _sendSocket(H4AW_WebsocketClient* skt, const std::string& type, const std::string& msg);
                void            _signalOff(){ H4P_Signaller::signal(H4P_SIG_STOP); }
                void            _uiAdd(const std::string& name,H4P_UI_TYPE t,const std::string& h="u",const std::string& v="",uint8_t c=H4P_UILED_BI);
};