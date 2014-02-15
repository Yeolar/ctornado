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

#define ERROR_MSG_LEN       256

VError::VError(int err, const char *msg, ...)
    : Error(err)
{
    va_list args;

    va_start(args, msg);
    format_message(msg, args);
    va_end(args);
}

VError::VError(const char *msg, ...)
    : Error()
{
    va_list args;

    va_start(args, msg);
    format_message(msg, args);
    va_end(args);
}

VError::~VError() noexcept
{
    if (msg_)
        free_w(msg_);
}

void VError::format_message(const char *fmt, va_list varg)
{
    msg_ = reinterpret_cast<char *>(alloc_w(ERROR_MSG_LEN));

    vscnprintf(msg_, ERROR_MSG_LEN, fmt, varg);
}

ValueError::ValueError(const char *msg, ...)
    : VError()
{
    va_list args;

    va_start(args, msg);
    format_message(msg, args);
    va_end(args);
}

RegexError::RegexError(const char *msg, ...)
    : VError()
{
    va_list args;

    va_start(args, msg);
    format_message(msg, args);
    va_end(args);
}

HTTPError::HTTPError(int code)
    : VError(code)
{
    va_list args;

    format_message(nullptr, args);
}

HTTPError::HTTPError(int code, const char *msg, ...)
    : VError(code)
{
    va_list args;

    va_start(args, msg);
    format_message(msg, args);
    va_end(args);
}

void HTTPError::format_message(const char *fmt, va_list varg)
{
    int n;

    msg_ = reinterpret_cast<char *>(alloc_w(ERROR_MSG_LEN));

    n = scnprintf(msg_, ERROR_MSG_LEN, "HTTP %d: %s (",
            err_, get_response_w3c_name(err_));

    if (fmt != nullptr) {
        n += vscnprintf(msg_ + n, ERROR_MSG_LEN - n, fmt, varg);
        n += scnprintf(msg_ + n, ERROR_MSG_LEN - n, "(");
    }
}

} // namespace
