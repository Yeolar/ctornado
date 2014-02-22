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

#include "ctornado.h"

namespace ctornado {

#define DATE_BUF_SIZE   64

int64_t usec_now()
{
    struct timeval t;

    if (gettimeofday(&t, nullptr) < 0) {
        log_error("gettimeofday failed: %s", strerror(errno));
        return -1;
    }
    return static_cast<int64_t>(t.tv_sec) * 1000000LL +
        static_cast<int64_t>(t.tv_usec);
}

int64_t msec_now()
{
    return usec_now() / 1000LL;
}

int64_t sec_now()
{
    return usec_now() / 1000000LL;
}

Str format_date(time_t *timer, const char *fmt)
{
    str_buffer_t *buffer;
    size_t n;

    buffer = Str::alloc(DATE_BUF_SIZE);
    n = strftime(buffer->data, DATE_BUF_SIZE, fmt, localtime(timer));
    if (n == 0) {
        log_error("format_date exceeds the temp buffer size");
    }
    return Str(buffer, n);
}

Str format_date(int64_t timestamp, const char *fmt)
{
    return format_date(reinterpret_cast<time_t *>(&timestamp), fmt);
}

Str format_email_date(int64_t timestamp)
{
    return format_date(timestamp, "%a, %d %b %Y %H:%M:%S GMT");
}

} // namespace
