/* 
MIT License

Copyright (c) 2026 H4Group

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Contact Email: TBD
*/
#pragma once

#include<H4Service.h>
#include<H4P_AsyncMQTT.h>

class H4P_LinkMaster: public H4Service{
            H4P_AsyncMQTT*                  _pMQTT;
            std::unordered_set<std::string> _slaves;
            VSCMD(_slave);
    protected:
        virtual void                _handleEvent(const std::string& svc,H4PE_TYPE t,const std::string& msg) override;
    public:
        H4P_LinkMaster(std::unordered_set<std::string> slaved={}): H4Service("link",H4PE_GVCHANGE,false), _slaves(slaved) {
            _pMQTT=depend<H4P_AsyncMQTT>(mqttTag());
            _addLocals({
                {"slave",    { H4PC_H4,   0 , CMDVS(_slave)}}
            });
        }

#if H4P_LOG_MESSAGES
        virtual void        info() override;
#endif
                void        slave(const std::string& otherh4,bool inout=true);
};