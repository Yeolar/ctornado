//
// Copyright (C) 2013 Yeolar <yeolar@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef __TIMER_H
#define __TIMER_H

#include "ctornado.h"

namespace ctornado {

class ClockTimer
{
public:
    ClockTimer()
        : begin_(), end_(), running_() {}

    void start()
    {
        ASSERT(!running_);

        begin_ = clock();
        end_ = 0;
        running_ = true;
    }

    void stop()
    {
        if (running_) {
            end_ = clock();
            running_ = false;
        }
    }

    double seconds() const
    {
        return double(end_ - begin_) / CLOCKS_PER_SEC;
    }

private:
    clock_t begin_;
    clock_t end_;
    bool running_;
};

} // namespace

#endif // __TIMER_H

