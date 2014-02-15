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

#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#include "ctornado.h"

namespace ctornado {

class Error : public std::exception
{
public:
    Error() : err_(0), msg_(nullptr) {}
    Error(int err) : err_(err), msg_(nullptr) {}
    Error(int err, const char *msg) : err_(err), msg_((char *) msg) {}
    Error(const char *msg) : err_(0), msg_((char *) msg) {}

    virtual const char *what() const noexcept
    {
        return msg_ != nullptr ? msg_ : (
                err_ != 0 ? strerror(err_) : "No description");
    }

    virtual int no() const noexcept
    {
        return err_;
    }

protected:
    int err_;
    char *msg_;
};


class VError : public Error
{
public:
    VError() : Error() {}
    VError(int err) : Error(err) {}
    VError(int err, const char *msg, ...);
    VError(const char *msg, ...);
    virtual ~VError() noexcept;

protected:
    virtual void format_message(const char *fmt, va_list varg);
};


class NotImplError : public Error
{
public:
    NotImplError() : Error("Not implemented") {}
};


class IOError : public Error
{
public:
    IOError(int err) : Error(err) {}
    IOError(int err, const char *msg) : Error(err, msg) {}
    IOError(const char *msg) : Error(msg) {}
};


class SocketError : public IOError
{
public:
    SocketError(int err) : IOError(err) {}
    SocketError(const char *msg) : IOError(msg) {}
};


class SocketGaiError : public IOError
{
public:
    SocketGaiError(int err) : IOError(err) {}

    virtual const char *what() const noexcept
    {
        return gai_strerror(err_);
    }
};


class ValueError : public VError
{
public:
    ValueError(const char *msg, ...);
};


class RegexError : public VError
{
public:
    RegexError(const char *msg, ...);
};


class HTTPError : public VError
{
public:
    HTTPError(int code);
    HTTPError(int code, const char *msg, ...);

protected:
    virtual void format_message(const char *fmt, va_list varg);
};

} // namespace

#endif // __EXCEPTION_H

