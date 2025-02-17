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

#include<H4P_UPNPServer.h>
#include<H4P_SerialCmd.h>

using H4P_UPNP_TAGMAP = std::unordered_map<std::string,std::unordered_set<std::string>>;
H4P_UPNP_TAGMAP h4pUPNPMap;

std::string H4P_UPNPServer::__makeUSN(const std::string& s){
	std::string full=_uuid+_udn;
	return s.size() ? full+="::"+s:full;
}

std::string H4P_UPNPServer::__upnpCommon(const std::string& usn){
	h4p.gvSetstring("usn",__makeUSN(usn));
	std::string rv=h4preplaceparams(_ucom);
	return rv+"\r\n";
}

void H4P_UPNPServer::__upnpSend(uint32_t mx,const std::string s,IPAddress ip,uint16_t port){
	h4.nTimesRandom(H4P_UDP_REPEAT,0,mx,[s,ip,port,this]() {
#ifdef ARDUINO_ARCH_RP2040
		H4P_PRINTF("__upnpSend\n");
		if (_udp.beginPacket(ip, port)) {
			H4P_PRINTF("BEGIN PACKET\n");
			_udp.print(s.c_str());
			auto end = _udp.endPacket();
			H4P_PRINTF("END PACKET %d\n", end);
		}
#else
		_udp.writeTo((uint8_t *)CSTR(s), s.size(), ip, port);
#endif
	},nullptr,H4P_TRID_UDPS); // name this!!
}

void H4P_UPNPServer::_handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg){
    switch(t){
        case H4PE_VIEWERS:
            {
                if(STOI(msg)) {
                    auto mode = WiFi.getMode();
                #if H4P_USE_WIFI_AP
                    if((mode==WIFI_AP || mode==WIFI_AP_STA) && h4p.gvExists(GoTag()))
                        h4puiAdd(nameTag(),H4P_UI_INPUT,"s");
                    else h4puiAdd(nameTag(),H4P_UI_TEXT,"s");
                #else
                    h4puiAdd(nameTag(),H4P_UI_TEXT,"s");
                #endif
                }
            }
            break;
        case H4PE_GVCHANGE:
            if(_running && svc==nameTag()){
//            if(svc==nameTag()){
                svcDown(); // shut down old name, send bye bye etc
                svcUp();
            }
        default:
            break;
    }
}

void H4P_UPNPServer::_handlePacket(std::string p,IPAddress ip,uint16_t port){
    XLOG("_handlePacket \"%s\"\t from %s:%d", p.c_str(), ip.toString().c_str(), port);
    H4P_NVP_MAP uhdrs;
    std::vector<std::string> hdrs = split(p, "\r\n");
    while (hdrs.back().empty()) hdrs.pop_back();
    if(hdrs.size() > 4){
        for (auto const &h: std::vector<std::string>(++hdrs.begin(), hdrs.end())) {
            std::vector<std::string> parts=split(h,":");
            if(parts.size()) uhdrs[parts[0]]=ltrim(join(std::vector<std::string>(++parts.begin(),parts.end()),":")); // y no u/case???
        }
    }
    uint32_t mx=1000 * atoi(CSTR(replaceAll(uhdrs["CACHE-CONTROL"],"max-age=","")));
    switch(p[0]){
        case 'M':
            {
                std::string ST = uhdrs["ST"];
                if (ST==_pups[1]) { // make tag
                    std::string tail=((ST==_pups[1]) ? ST:"");
                    __upnpSend(mx, "HTTP/1.1 200 OK\r\nST:" + ST +"\r\n" +__upnpCommon(tail), ip,port);
                }
            }
            break;
        case 'N':
            for(auto &L:h4pUPNPMap){
                if(uhdrs.count(L.first)){
                    std::string v=uhdrs[L.first];
                    if(L.second.count("*") || L.second.count(v)) h4psysevent(v,H4PE_UPNP,"%s",(replaceAll(uhdrs["NTS"],"ssdp:","")==aliveTag()) ? CSTR(ip.toString()):"");
                }
            }
    }
}

void H4P_UPNPServer::_init(){
    _pups.push_back(_urn+"device:controllee:1");
    _pups.push_back(_urn+"service:basicevent:1");
}

void H4P_UPNPServer::_listenTag(const std::string& tag,const std::string& value){ h4pUPNPMap[tag].insert(value); }

void H4P_UPNPServer::_listenUDP(){ 
    QLOG("_listenUDP");
#ifdef ARDUINO_ARCH_RP2040
    static H4_TIMER UDPListen = nullptr;
    if (!_udp.beginMulticast(_ubIP, 1900)){
        if (UDPListen){
            h4.cancel(UDPListen);
            UDPListen = nullptr;
        }
        return;
    }
    if (!UDPListen)
        UDPListen = h4.every(250, [this](){
            std::string pkt;
            auto avail = _udp.available();
            if (avail) {
                char p[avail];
                _udp.read(p,avail);
                // pkt.append(reinterpret_cast<char>(_udp.read()))
                pkt = std::string(p,avail);
                XLOG("_udp.onPacket %s", pkt.c_str());
            }
            if (pkt.length()) {
                auto ip = _udp.remoteIP();
                auto port = _udp.remotePort();
                h4.queueFunction([=, this](){ _handlePacket(pkt,ip,port); },nullptr,H4P_TRID_UPKT); // shud be named etc

            }
        });
#else
    if(!_udp.listenMulticast(_ubIP, 1900)) return; // some kinda error?
    _udp.onPacket([this](AsyncUDPPacket packet){
        std::string pkt((const char*)packet.data(),packet.length());
        XLOG("_udp.onPacket %s", pkt.c_str());
        IPAddress ip=packet.remoteIP();
        uint16_t port=packet.remotePort();
        h4.queueFunction([=, this](){ _handlePacket(pkt,ip,port); },nullptr,H4P_TRID_UPKT); // shud be named etc
    }); 
#endif
}

void H4P_UPNPServer::_notify(const std::string& phase){
    XLOG("_notify(%s)", phase.c_str());
    h4Chunker<std::vector<std::string>>(_pups,[this, phase](std::vector<std::string>::iterator i){ 
        std::string NT=(*i).size() ? (*i):__makeUSN("");
        std::string nfy="NOTIFY * HTTP/1.1\r\nHOST:"+std::string(_ubIP.toString().c_str())+":1900\r\nNTS:ssdp:"+phase+"\r\nNT:"+NT+"\r\n"+__upnpCommon((*i));
        XLOG("_broadcast %s", nfy.c_str());
        _broadcast(H4P_UDP_JITTER,CSTR(nfy));
    },H4_JITTER_LO,H4_JITTER_HI,[=]{ h4p.gvErase("usn"); });
/*
    h4.worker<std::vector<std::string>>(_pups,[=](std::string s){
        std::string NT=s.size() ? s:__makeUSN("");
        std::string nfy="NOTIFY * HTTP/1.1\r\nHOST:"+std::string(_ubIP.toString().c_str())+":1900\r\nNTS:ssdp:"+phase+"\r\nNT:"+NT+"\r\n"+__upnpCommon(s);
        _broadcast(H4P_UDP_JITTER,CSTR(nfy));
    },H4_JITTER_LO,H4_JITTER_HI,[=]{ h4p.gvErase("usn"); });
*/
}

void H4P_UPNPServer::_upnp(H4AW_HTTPHandler *handler){ // redo
    h4.queueFunction([this, handler]() {
        std::string soap(handler->url());
       	h4p.gvSetstring("gs",(soap.find("Get")==std::string::npos) ? "Set":"Get"); 
        uint32_t _set=soap.find(">1<")==std::string::npos ? 0:1;
        if(h4p["gs"]=="Set") h4p.gvSetInt(stateTag(),_set); //=//XEVENT(H4PE_ONOF,"%d",_set);
//        Serial.printf("XML RESPONSE %s\n",h4preplaceparams(_soap).data());
        auto body = h4preplaceparams(_soap);
        handler->send(200, "text/xml", body.length(), CSTR(body)); // refac
        h4p.gvErase("gs");
    },nullptr, H4P_TRID_SOAP);
}

#if H4P_LOG_MESSAGES
void H4P_UPNPServer::info(){ 
    H4Service::info();
    reply(" Name: %s UDN=%s",CSTR(h4p[nameTag()]),CSTR(_udn));
    if(h4pUPNPMap.size()){
        reply(" Listening for tags:");
        for(auto const& t:h4pUPNPMap){
            reply("  Tag: %s",t.first.data());
            for(auto const& s:t.second){
                reply("   %s",s.data());
            }
        }
    }
}
#endif

void H4P_UPNPServer::svcDown(){
    h4.cancelSingleton(H4P_TRID_NTFY);
    if (_running)
        _notify("byebye");
#ifdef ARDUINO_ARCH_RP2040
    _udp.stop();
#else
    _udp.close(); // delay this RW?
#endif
    H4Service::svcDown();
}

void H4P_UPNPServer::svcUp(){
#if H4P_USE_WIFI_AP
    if(WiFi.getMode()==WIFI_AP) return;
#endif
    _udn=std::string("Socket-1_0-upnp").append(h4p[chipTag()]);
    h4p.gvSetstring("udn",_udn);
    h4p.gvSetstring(ageTag(),stringFromInt(H4P_UDP_REFRESH/1000));
    h4p.gvSetstring("updt",_pups[2]);
    h4p.gvSetstring("umfr","Belkin International Inc.");
    h4p.gvSetstring("usvc",_pups[3]);
    h4p.gvSetstring("usid",_urn+"serviceId:basicevent1");

    _xml=replaceParamsFile("/up.xml");
    _ucom=replaceParamsFile("/ucom.txt");
    _soap=H4P_SerialCmd::read("/soap.xml");

    h4p.gvErase({"udn",ageTag(),"updt","umfr","usvc","usid"});
//
    _listenUDP();
    _notify(aliveTag()); // TAG
    h4.every(H4P_UDP_REFRESH / 2,[this](){ _notify(aliveTag()); },nullptr,H4P_TRID_NTFY,true); // TAG
    H4Service::svcUp();
}