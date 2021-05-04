/*
 MIT License

Copyright (c) 2020 Phil Bowles <H4Services@gmail.com>
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
//
//vector<string> h4pnames={};
H4P_EVENT_HANDLERS h4pevt;

void H4Service::_addLocals(H4P_CMDMAP local){
    h4pCmdMap.insert(local.begin(),local.end());
    local.clear();
}

void H4Service::_envoi(const string& s){
    auto pp=h4puncheckedcall<H4Service>(h4pSrc);
    if(pp) pp->_reply(CSTR(s)); // send reply back to originating source
    else Serial.printf("%s\n",CSTR(s));
}

vector<uint32_t> H4Service::_expectInt(string pl,const char* delim){
    vector<uint32_t> results;
    vector<string> tmp=split(pl,delim);
    for(auto const& t:tmp){
        if(!stringIsNumeric(t)) return {};
        results.push_back(STOI(t));
    }
    return results;
}

uint32_t H4Service::_guard1(vector<string> vs,H4_FN_MSG f){
    if(!vs.size()) return H4_CMD_TOO_FEW_PARAMS;
    return vs.size()>1 ? H4_CMD_TOO_MANY_PARAMS:f(vs);
}

uint32_t H4Service::_guardInt1(vector<string> vs,function<void(uint32_t)> f){
    return _guard1(vs,[f,this](vector<string> vs){
        auto vi=_expectInt(H4PAYLOAD);
        if(vi.size()==1) return ([f](uint32_t v){ f(v); return H4_CMD_OK; })(vi[0]);
        return H4_CMD_NOT_NUMERIC;
    });
}

uint32_t H4Service::_guardString2(vector<string> vs,function<H4_CMD_ERROR(string,string)> f){
    return _guard1(vs,[f,this](vector<string> vs){
        auto vg=split(H4PAYLOAD,",");
        if(vg.size()<3){ 
            if(vg.size()>1) return ([f](string s1,string s2){ return f(s1,s2); })(vg[0],vg[1]);
            return H4_CMD_TOO_FEW_PARAMS;
        }
        return H4_CMD_TOO_MANY_PARAMS;
    });
}

void H4Service::_sysHandleEvent(const string& svc,H4PE_TYPE t,const string& msg){
    switch(t){
        case H4PE_BOOT:
            if(_filter & H4PE_BOOT){
                Serial.printf("%s BOOT calls _init()\n",CSTR(_me));
                _init();
            } else Serial.printf("%s BOOT PREVENTS DOUBLE DIP _init()\n",CSTR(_me)); 
            break;
        case H4PE_STAGE2:
            Serial.printf("%s STAGE2 %s %s\n",CSTR(_me),CSTR(svc),CSTR(msg));
            if(!(_filter & H4PE_SERVICE)) svcUp(); // not waiting to be started
            break;
        case H4PE_SERVICE:
            Serial.printf("%s SERVICE %s %s parent=%s\n",CSTR(_me),CSTR(svc),CSTR(msg),CSTR(_parent));
            if(_parent==svc){
                if(STOI(msg)) svcUp();
                else svcDown();
            } 
            else _handleEvent(svc,H4PE_SYSINFO,string("Svc").append(STOI(msg) ? "Up":"Down"));
            break;
        default:
            _handleEvent(svc,t,msg);
            break;
    }
}

string H4Service::_uniquify(const string& name,uint32_t uqf){
    string tmp=name+(uqf ? stringFromInt(uqf):"");
    return h4pmap.count(tmp) ? _uniquify(name,uqf+1):tmp;
}
#if H4P_LOG_MESSAGES
    #if H4P_VERBOSE
        extern H4_INT_MAP eventTypes;
    #endif
void H4Service::info(){ 
    reply("SVC: %s PID=%d %sRunning",CSTR(_me),_pid,_running ? "":"Not ");
    if(_parent!="") reply(" Depends on %s",CSTR(_parent));
    //
    vector<string> dees;
    for(auto const& s:h4pmap) if(s.second->_parent==_me) dees.push_back(s.first);
    if(dees.size()){
        reply(" Dependees");
        for(auto const& d:dees) reply("  %s",CSTR(d));
    }
    //
    #if H4P_VERBOSE
    if(_filter){
        reply(" Listens for");
        for(int i=0;i<32;i++){
            auto shift=1 << i;
            if( eventTypes.count(shift) && (_filter & shift)) reply("  %s",CSTR(h4pGetEventName(static_cast<H4PE_TYPE>(shift))));
        }
    }
    #endif // verbose
}
#endif // log messages