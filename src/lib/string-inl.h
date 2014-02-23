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

#ifndef __STRING_INL_H
#define __STRING_INL_H

#include "ctornado.h"

namespace ctornado {

#define STR_VALID_POS(_p, _n)   \
    (_p) != -1 ? (_p) : (_n);   \
    ASSERT((_p) >= -1 && (_p) <= static_cast<int>(_n))

//
// empty string
//
inline Str::Str()
{
    len_ = 0;
    data_ = nullptr;
    buffer_ = nullptr;
}

inline Str::Str(const Str& str)
{
    len_ = str.len_;
    data_ = str.data_;
    buffer_ = str.buffer_;

    if (buffer_ != nullptr) {
        buffer_->cnt++;
    }
}

inline Str::Str(Str&& str)
{
    len_ = str.len_;
    data_ = str.data_;
    buffer_ = str.buffer_;

    str.len_ = 0;
    str.data_ = nullptr;
    str.buffer_ = nullptr;
}

//
// build from buffer, used by create/copy
//
inline Str::Str(str_buffer_t *buffer, size_t n)
{
    len_ = n;
    data_ = buffer->data;
    buffer_ = buffer;
}

//
// construct from C-string, uncopy
//
inline Str::Str(const char *str, size_t n)
{
    buffer_ = nullptr;

    if (str == nullptr) {
        len_ = 0;
        data_ = nullptr;
    }
    else {
        len_ = n > 0 ? n : strlen(str);
        data_ = str;
    }
}

//
// construct from another string, shared
//
inline Str::Str(const Str& str, const char *pos, size_t n)
{
    len_ = n;
    data_ = pos;
    buffer_ = str.buffer_;

    if (buffer_ != nullptr) {
        buffer_->cnt++;
    }
}

inline Str::~Str()
{
    if (buffer_ != nullptr) {
        buffer_->cnt--;

        if (buffer_->cnt == 0)
            FREE(buffer_);
    }
}

inline str_buffer_t *Str::alloc(size_t n)
{
    str_buffer_t *buffer;

    buffer = reinterpret_cast<str_buffer_t *>(ALLOC(n + sizeof(uint16_t)));
    buffer->cnt = 1;

    return buffer;
}

//
// construct from C-string, copy
//
inline Str Str::create(const char *str)
{
    str_buffer_t *buffer;
    size_t n;

    n = strlen(str);

    buffer = alloc(n);
    memcpy(buffer->data, str, n);

    return Str(buffer, n);
}

//
// construct from self, shared
//
inline Str Str::share(const char *pos, size_t n) const
{
    return Str(*this, pos, n);
}

//
// construct from self, duplicate
//
inline Str Str::copy() const
{
    str_buffer_t *buffer;

    buffer = alloc(len_);
    memcpy(buffer->data, data_, len_);

    return Str(buffer, len_);
}

inline Str& Str::operator=(const char *str)
{
    buffer_ = nullptr;

    if (str == nullptr) {
        len_ = 0;
        data_ = nullptr;
    }
    else {
        len_ = strlen(str);
        data_ = str;
    }
    return *this;
}

inline Str& Str::operator=(const Str& str)
{
    if (this != &str) {
        len_ = str.len_;
        data_ = str.data_;
        buffer_ = str.buffer_;

        if (buffer_ != nullptr) {
            buffer_->cnt++;
        }
    }
    return *this;
}

inline Str& Str::operator=(Str&& str)
{
    if (this != &str) {
        //
        // With the same buffer, e.g.:
        //  s = s.substr(1, -1)
        //
        if (buffer_ != nullptr && buffer_ == str.buffer_)
            str.buffer_->cnt--;

        len_ = str.len_;
        data_ = str.data_;
        buffer_ = str.buffer_;

        str.len_ = 0;
        str.data_ = nullptr;
        str.buffer_ = nullptr;
    }
    return *this;
}

inline char Str::operator[](int i) const
{
    if (i >= static_cast<int>(len_))
        throw out_of_range("Invalid position");

    i += len_;

    if (i < 0)
        throw out_of_range("Invalid position");

    return *(data_ + i % len_);
}

inline string Str::tos() const
{
    return string(data_, len_);
}

inline const char *Str::begin() const
{
    return data_;
}

inline const char *Str::end() const
{
    return data_ + len_;
}

inline bool Str::eq(const Str& str) const
{
    if (data_ == nullptr && str.data_ == nullptr)   // nullstr
        return true;

    if (data_ == nullptr || str.data_ == nullptr || len_ != str.len_)
        return false;

    return memcmp(data_, str.data_, len_) == 0;
}

inline bool Str::null() const
{
    return len_ == 0 && data_ == nullptr;
}

inline bool Str::empty() const
{
    return len_ == 0;
}

inline int Str::find(char c, int start, int end) const
{
    const char *pos;

    if (data_ == nullptr)
        return -1;

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    pos = strnfind(data_ + start, end - start, c);

    return pos != nullptr ? pos - data_ : -1;
}

inline int Str::find(const Str& str, int start, int end) const
{
    const char *pos;

    if (data_ == nullptr || str.data_ == nullptr)
        return -1;

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    pos = strnfind(data_ + start, end - start, str.data_, str.len_);

    return pos != nullptr ? pos - data_ : -1;
}

inline int Str::rfind(char c, int start, int end) const
{
    const char *pos;

    if (data_ == nullptr)
        return -1;

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    pos = strnrfind(data_ + start, end - start, c);

    return pos != nullptr ? pos - data_ : -1;
}

inline int Str::rfind(const Str& str, int start, int end) const
{
    const char *pos;

    if (data_ == nullptr || str.data_ == nullptr)
        return -1;

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    pos = strnrfind(data_ + start, end - start, str.data_, str.len_);

    return pos != nullptr ? pos - data_ : -1;
}

inline bool Str::starts_with(const Str& prefix) const
{
    if (data_ == nullptr || prefix.data_ == nullptr)
        return false;

    if (len_ < prefix.len_)
        return false;

    return memcmp(data_, prefix.data_, prefix.len_) == 0;
}

inline bool Str::ends_with(const Str& suffix) const
{
    if (data_ == nullptr || suffix.data_ == nullptr)
        return false;

    if (len_ < suffix.len_)
        return false;

    return memcmp(data_ + len_ - suffix.len_, suffix.data_, suffix.len_) == 0;
}

inline Str Str::substr(int start, int end) const
{
    ASSERT(data_ != nullptr);

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    return share(data_ + start, end - start);
}

inline Str Str::join(const char *sep, const StrList& strs)
{
    return Str(sep).join(strs);
}

inline Str Str::replace(const char *oldstr, const char *newstr) const
{
    return Str(newstr).join(split(oldstr));
}

inline void Str::println() const
{
    print();
    putchar('\n');
}

} // namespace

#endif // __STRING_INL_H
