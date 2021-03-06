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
#ifndef H4P_Skeleton_H
#define H4P_Skeleton_H

#include<H4PCommon.h>

STAG(skel);

class H4P_Skeleton: public H4Plugin {
        VSCMD(_bones);

                void        _hookIn() override;
                void        _greenLight() override;
                void        _start() override {
                    h4._hookLoop([this](){ _run(); },_subCmd);
                    H4Plugin::_start();
                }
                void        _stop() override {
                    h4._unHook(_subCmd);
                    H4Plugin::_stop();
                }
                bool        _state() override;
                void        _run(); // rare
    public:
        H4P_Skeleton(const string& name,H4_FN_VOID onStart=nullptr,H4_FN_VOID onStop=nullptr);

        void rattle();
        
        void start();
        void stop();
};

#endif // H4P_Skeleton_H
