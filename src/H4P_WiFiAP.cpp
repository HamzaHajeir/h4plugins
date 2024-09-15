/*
 MIT License

Copyright (c) 2020 Phil Bowles <h4plugins@gmail.com>
   github     https://github.com/philbowles/esparto
   blog       https://8266iot.blogspot.com     
   groups     https://www.facebook.com/groups/esp8266questions/
              https://www.facebook.com/Esparto-Esp8266-Firmware-Support-2338535503093896/
                			  

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
#include<H4Service.h>
#include<H4P_WiFi.h>

#if H4P_USE_WIFI_AP | H4P_WIFI_PROV_BY_BLE
H4_TIMER scanner;
constexpr const char* opts_base = " Select SSID..." UNIT_SEPARATOR "dummy" RECORD_SEPARATOR;
std::vector<std::string> wf_ssids;
void appendSSIDs(std::string& base) {
    for (auto& ssid : wf_ssids){
        std::string ss=ssid;
        base.append(ss).append(UNIT_SEPARATOR).append(ss).append(RECORD_SEPARATOR);
    }
    base.pop_back();
}
void H4P_WiFi::_stopScan() {
    if (scanner) 
        h4.cancel(scanner);
    scanner = nullptr;
}
void H4P_WiFi::_startScan() {
    if (scanner) {
        H4P_PRINTF("Already scanning is activated\n");
        return;
    }
    auto scan = [this](){
        if (WiFi.getMode() == WIFI_OFF){
            // _wf_ssids.clear();
            return;
        }
        
        auto result = WiFi.scanComplete();
        XLOG("Scan result=%d", result);
        switch (result){
        case -1:
            XLOG("WiFi scan in progress", result);
            break;
        case 0:
            QLOG("no networks found");
        default:
            wf_ssids.clear();
#ifdef ARDUINO_ARCH_RP2040
            for (int i = 0; i < result; ++i)
                wf_ssids.emplace_back(WiFi.SSID(i));
#else
            for (int i = 0; i < result; ++i)
                wf_ssids.emplace_back(WiFi.SSID(i).c_str());
#endif
            WiFi.scanDelete();
            removeDuplicates(wf_ssids); // So WiFi repeaters won't repeat a SSID.
            [[fallthrough]];
        case -2:
#ifdef ARDUINO_ARCH_RP2040
            WiFi.scanNetworks(true);
#else
            WiFi.scanNetworks(true, true);
#endif
            break;
        }
        std::string msg{opts_base};
        appendSSIDs(msg);
        h4puiSync(ssidTag(), msg);
        h4pbleSync(ssidTag(), msg);
    };
    scan();
    scanner = h4.every(H4P_AP_SCAN_RATE, scan);
}

#endif

#if H4P_USE_WIFI_AP

void H4P_WiFi::_apViewers() {
    std::string msg {opts_base};
    appendSSIDs(msg);
    _uiAdd(ssidTag(),H4P_UI_DROPDOWN,"s",msg);
    _uiAdd(pskTag(),H4P_UI_INPUT,"s");
    _uiAdd(deviceTag(),H4P_UI_INPUT,"s");
    _uiAdd(GoTag(),H4P_UI_IMGBTN,"o");
}
void H4P_WiFi::_startAP(){
    h4p.gvSetInt(GoTag(),0,false);
    _apconfig = true;

    H4P_Signaller::signal(H4P_SIG_MORSE,"-    ,250");
    _dns53=new DNSServer;

    HAL_WIFI_disconnect();
    
    _startScan();

    WiFi.mode(WIFI_AP);
    SYSINFO("ENTER AP MODE %s MAC=%s",CSTR(h4p[deviceTag()]),CSTR(WiFi.softAPmacAddress()));

    WiFi.softAP(CSTR(h4p[deviceTag()]));
    _dns53->start(53, "*", WiFi.softAPIP());
    h4.every(H4WF_AP_RATE,[this](){ _dns53->processNextRequest(); });

    _startWebserver();
}
#endif