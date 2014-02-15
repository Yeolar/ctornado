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

#ifndef __DATETIME_H
#define __DATETIME_H

#include "ctornado.h"

namespace ctornado {

int64_t usec_now();
int64_t msec_now();
int64_t sec_now();

Str format_date(time_t *timer, const char *fmt);
Str format_date(int64_t timestamp, const char *fmt);

//
// Format date as RFC 2822 described: "%a, %d %b %Y %H:%M:%S GMT"
//
Str format_email_date(int64_t timestamp);

} // namespace

#endif // __DATETIME_H

