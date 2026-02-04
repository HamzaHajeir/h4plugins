/*
/* 
MIT License

Copyright (c) 2026 H4Group

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Contact Email: TBD
*/
#pragma once

#include <H4Service.h>

class H4P_EmitQ: public H4Service {
        uint32_t _f;
        uint32_t _scale;

    public:
        H4P_EmitQ(uint32_t f=1000,uint32_t scale=1): H4Service(emtqTag()),_f(f),_scale(scale){}

        virtual void svcUp() override { 
            h4.every(_f,[this](){ XEVENT(H4PE_Q,"%u",_scale * h4.size()); },nullptr,H4P_TRID_QLOG,true);
            H4Service::svcUp();
        }
        virtual void svcDown() override { 
            h4.cancelSingleton(H4P_TRID_QLOG);
            H4Service::svcDown();
        }
};