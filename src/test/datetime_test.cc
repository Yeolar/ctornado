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

using namespace ctornado;

int main()
{
    log_stderr("current microsecond:     %ld", usec_now());
    log_stderr("current millisecond:     %ld", msec_now());
    log_stderr("current second:          %ld", sec_now());

    log_stderr("current time (RFC 2822): %s",
            format_email_date(sec_now()).tos().c_str());

    return 0;
}
