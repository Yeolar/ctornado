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

static char _hex[] = "0123456789abcdef";
static char _HEX[] = "0123456789ABCDEF";

//
// set uint value in little-endian
//
void *memsetu(void *ptr, uint64_t value, size_t n)
{
    uint8_t *p;
    size_t i;

    ASSERT(n > 0 && n <= 8);

    p = reinterpret_cast<uint8_t *>(ptr);
    i = 0;

    do {
        *p++ = (value >> (i++ * 8)) & 0xff;
    } while (i < n);

    return ptr;
}

const char *strnfind(const char *s, size_t n, char c)
{
    char c1;

    do {
        if (n-- == 0)
            return nullptr;
        c1 = *s++;

    } while (c1 != c);

    return --s;
}

const char *strnrfind(const char *s, size_t n, char c)
{
    const char *p;
    char c1;

    p = s + n;

    do {
        if (n-- == 0)
            return nullptr;
        c1 = *--p;

    } while (c1 != c);

    return p;
}

const char *strnfind(const char *s, size_t n, const char *s2, size_t n2)
{
#ifdef _GNU_SOURCE
    return reinterpret_cast<char *>(memmem(s, n, s2, n2));
#else
    char c1, c2;

    c2 = *s2++;
    n2--;

    do {
        do {
            if (n-- == 0)
                return nullptr;
            c1 = *s++;

        } while (c1 != c2);

        if (n < n2)
            return nullptr;

    } while (memcmp(s, s2, n2) != 0);

    return --s;
#endif
}

const char *strnrfind(const char *s, size_t n, const char *s2, size_t n2)
{
    const char *p;
    char c1, c2;

    p = s + n - n2;
    c2 = *s2++;

    if (n < n2)
        return nullptr;

    n2--;

    do {
        do {
            if (n-- == n2)
                return nullptr;
            c1 = *p--;

        } while (c1 != c2);

    } while (memcmp(p + 2, s2, n2) != 0);

    return ++p;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int n;

    va_start(args, fmt);
    n = vscnprintf(buf, size, fmt, args);
    va_end(args);

    return n;
}

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    int n;

    n = vsnprintf(buf, size, fmt, args);

    //
    // The return value is the number of characters which would be written
    // into buf not including the trailing '\0'. If size is == 0 the
    // function returns 0.
    //
    // On error, the function also returns 0. This is to allow idiom such
    // as len += _vscnprintf(...)
    //
    // See: http://lwn.net/Articles/69419/
    //
    if (n <= 0) {
        return 0;
    }
    if (n < static_cast<int>(size)) {
        return n;
    }
    return size - 1;
}

static char *_sprintf_num(char *buf, char *end, uint64_t ui64, char zero,
    int hexadecimal, size_t width);
static size_t _sprintf_num_len(uint64_t ui64, int hexadecimal, size_t width);

//
// supported formats:
//    %[0][width]T              time_t
//    %[0][width][u][x|X]z      ssize_t/size_t
//    %[0][width][u][x|X]d      int/u_int
//    %[0][width][u][x|X]l      long
//    %[0][width][u][x|X]D      int32_t/uint32_t
//    %[0][width][u][x|X]L      int64_t/uint64_t
//    %[0][width][.width]f      double, max valid number fits to %18.15f
//    %p                        void *
//    %S                        Str *
//    %s                        null-terminated string
//    %*s                       length and string
//    %Z                        '\0'
//    %N                        '\n'
//    %c                        char
//    %%                        %
//
char *slprintf(char *buf, char *end, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vslprintf(buf, end, fmt, args);
    va_end(args);

    return buf;
}

char *vslprintf(char *buf, char *end, const char *fmt, va_list args)
{
    char *p, zero;
    int d;
    double f;
    size_t len, slen;
    int64_t i64;
    uint64_t ui64, frac, scale;
    int width, sign, hex, frac_width, k;
    Str *s;

    while (*fmt && buf < end) {
        //
        // "buf < end" means that we could copy at least one character:
        // the plain character, "%%", "%c", and minus without the checking
        //
        if (*fmt == '%') {
            i64 = 0;
            ui64 = 0;

            zero = (*++fmt == '0') ? '0' : ' ';
            width = 0;
            sign = 1;
            hex = 0;
            frac_width = 0;
            slen = SIZE_MAX;

            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt++ - '0');
            }
            for ( ;; ) {
                switch (*fmt) {
                case 'u':
                    sign = 0;
                    fmt++;
                    continue;
                case 'X':
                    hex = 2;
                    sign = 0;
                    fmt++;
                    continue;
                case 'x':
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;
                case '.':
                    fmt++;
                    while (*fmt >= '0' && *fmt <= '9') {
                        frac_width = frac_width * 10 + (*fmt++ - '0');
                    }
                    break;
                case '*':
                    slen = va_arg(args, size_t);
                    fmt++;
                    continue;
                default:
                    break;
                }
                break;
            }

            switch (*fmt) {
            case 'S':
                s = va_arg(args, Str *);
                len = min(static_cast<size_t>(end - buf), s->len());
                memcpy(buf, s->data(), len);
                buf += len;
                fmt++;
                continue;
            case 's':
                p = va_arg(args, char *);
                if (slen == SIZE_MAX) {
                    while (*p && buf < end) {
                        *buf++ = *p++;
                    }
                }
                else {
                    len = min(static_cast<size_t>(end - buf), slen);
                    memcpy(buf, p, len);
                    buf += len;
                }
                fmt++;
                continue;
            case 'T':
                i64 = va_arg(args, time_t);
                sign = 1;
                break;
            case 'z':
                if (sign) {
                    i64 = va_arg(args, ssize_t);
                }
                else {
                    ui64 = va_arg(args, size_t);
                }
                break;
            case 'd':
                if (sign) {
                    i64 = va_arg(args, int);
                }
                else {
                    ui64 = va_arg(args, unsigned int);
                }
                break;
            case 'l':
                if (sign) {
                    i64 = va_arg(args, long);
                }
                else {
                    ui64 = va_arg(args, unsigned long);
                }
                break;
            case 'D':
                if (sign) {
                    i64 = va_arg(args, int32_t);
                }
                else {
                    ui64 = va_arg(args, uint32_t);
                }
                break;
            case 'L':
                if (sign) {
                    i64 = va_arg(args, int64_t);
                }
                else {
                    ui64 = va_arg(args, uint64_t);
                }
                break;
            case 'f':
                f = va_arg(args, double);
                if (f < 0) {
                    *buf++ = '-';
                    f = -f;
                }
                ui64 = f;
                frac = 0;
                if (frac_width) {
                    scale = 1;
                    for (k = frac_width; k; k--) {
                        scale *= 10;
                    }
                    frac = (f - ui64) * scale + 0.5;
                    if (frac == scale) {
                        ui64++;
                        frac = 0;
                    }
                }
                buf = _sprintf_num(buf, end, ui64, zero, 0, width);
                if (frac_width) {
                    if (buf < end) {
                        *buf++ = '.';
                    }
                    buf = _sprintf_num(buf, end, frac, '0', 0, frac_width);
                }
                fmt++;
                continue;
            case 'p':
                ui64 = reinterpret_cast<uintptr_t>(va_arg(args, void *));
                hex = 2;
                sign = 0;
                zero = '0';
                width = PTR_SIZE * 2;
                break;
            case 'c':
                d = va_arg(args, int);
                *buf++ = d & 0xff;
                fmt++;
                continue;
            case 'Z':
                *buf++ = '\0';
                fmt++;
                continue;
            case 'N':
                *buf++ = '\n';
                fmt++;
                continue;
            case '%':
                *buf++ = '%';
                fmt++;
                continue;
            default:
                *buf++ = *fmt++;
                continue;
            }

            if (sign) {
                if (i64 < 0) {
                    *buf++ = '-';
                    ui64 = -i64;
                }
                else {
                    ui64 = i64;
                }
            }
            buf = _sprintf_num(buf, end, ui64, zero, hex, width);
            fmt++;
        }
        else {
            *buf++ = *fmt++;
        }
    }
    return buf;
}

static char *_sprintf_num(char *buf, char *end, uint64_t ui64, char zero,
    int hexadecimal, size_t width)
{
    char *p, temp[UINT64_MAXLEN];
    size_t len;
    uint32_t ui32;

    p = temp + UINT64_MAXLEN - 1;

    if (hexadecimal == 0) {
        //
        // To divide 64-bit numbers and to find remainders on the x86 platform
        // may call complex functions. For 32-bit numbers may use an inlined
        // multiplication and shifts.
        //
        if (ui64 <= UINT32_MAX) {
            ui32 = ui64;
            do {
                *--p = ui32 % 10 + '0';
            } while (ui32 /= 10);
        }
        else {
            do {
                *--p = ui64 % 10 + '0';
            } while (ui64 /= 10);
        }
    }
    else if (hexadecimal == 1) {
        do {
            *--p = _hex[ui64 & 0xf];
        } while (ui64 >>= 4);
    }
    else { // hexadecimal == 2
        do {
            *--p = _HEX[ui64 & 0xf];
        } while (ui64 >>= 4);
    }

    // zero or space padding
    len = (temp + UINT64_MAXLEN - 1) - p;
    while (len++ < width && buf < end) {
        *buf++ = zero;
    }

    // number safe copy
    len = (temp + UINT64_MAXLEN - 1) - p;
    if (buf + len > end) {
        len = end - buf;
    }
    memcpy(buf, p, len);

    return buf + len;
}

size_t vslprintf_len(const char *fmt, va_list args)
{
    char *p;
    double f;
    size_t slen, n;
    int64_t i64;
    uint64_t ui64, frac, scale;
    int width, sign, hex, frac_width, k;
    Str *s;

    n = 0;

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;

            i64 = 0;
            ui64 = 0;

            width = 0;
            sign = 1;
            hex = 0;
            frac_width = 0;
            slen = SIZE_MAX;

            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt++ - '0');
            }
            for ( ;; ) {
                switch (*fmt) {
                case 'u':
                    sign = 0;
                    fmt++;
                    continue;
                case 'X':
                case 'x':
                    hex = 1;
                    sign = 0;
                    fmt++;
                    continue;
                case '.':
                    fmt++;
                    while (*fmt >= '0' && *fmt <= '9') {
                        frac_width = frac_width * 10 + (*fmt++ - '0');
                    }
                    break;
                case '*':
                    slen = va_arg(args, size_t);
                    fmt++;
                    continue;
                default:
                    break;
                }
                break;
            }

            switch (*fmt) {
            case 'S':
                s = va_arg(args, Str *);
                n += s->len();
                fmt++;
                continue;
            case 's':
                p = va_arg(args, char *);
                if (slen == SIZE_MAX) {
                    n += strlen(p);
                }
                else {
                    n += slen;
                }
                fmt++;
                continue;
            case 'T':
                i64 = va_arg(args, time_t);
                sign = 1;
                break;
            case 'z':
                if (sign) {
                    i64 = va_arg(args, ssize_t);
                }
                else {
                    ui64 = va_arg(args, size_t);
                }
                break;
            case 'd':
                if (sign) {
                    i64 = va_arg(args, int);
                }
                else {
                    ui64 = va_arg(args, unsigned int);
                }
                break;
            case 'l':
                if (sign) {
                    i64 = va_arg(args, long);
                }
                else {
                    ui64 = va_arg(args, unsigned long);
                }
                break;
            case 'D':
                if (sign) {
                    i64 = va_arg(args, int32_t);
                }
                else {
                    ui64 = va_arg(args, uint32_t);
                }
                break;
            case 'L':
                if (sign) {
                    i64 = va_arg(args, int64_t);
                }
                else {
                    ui64 = va_arg(args, uint64_t);
                }
                break;
            case 'f':
                f = va_arg(args, double);
                if (f < 0) {
                    n++;
                    f = -f;
                }
                ui64 = f;
                frac = 0;
                if (frac_width) {
                    scale = 1;
                    for (k = frac_width; k; k--) {
                        scale *= 10;
                    }
                    frac = (f - ui64) * scale + 0.5;
                    if (frac == scale) {
                        ui64++;
                        frac = 0;
                    }
                }
                n += _sprintf_num_len(ui64, 0, width);
                if (frac_width) {
                    n += _sprintf_num_len(frac, 0, frac_width) + 1;
                }
                fmt++;
                continue;
            case 'p':
                ui64 = reinterpret_cast<uintptr_t>(va_arg(args, void *));
                hex = 1;
                sign = 0;
                width = PTR_SIZE * 2;
                break;
            case 'c':
                va_arg(args, int);
            default:
                n++;
                fmt++;
                continue;
            }

            if (sign) {
                if (i64 < 0) {
                    n++;
                    ui64 = -i64;
                }
                else {
                    ui64 = i64;
                }
            }
            n += _sprintf_num_len(ui64, hex, width);
            fmt++;
        }
        else {
            n++;
            fmt++;
        }
    }
    return n;
}

static size_t _sprintf_num_len(uint64_t ui64, int hexadecimal, size_t width)
{
    size_t n;
    uint32_t ui32;

    n = 0;

    if (hexadecimal == 0) {
        if (ui64 <= UINT32_MAX) {
            ui32 = ui64;
            do {
                n++;
            } while (ui32 /= 10);
        }
        else {
            do {
                n++;
            } while (ui64 /= 10);
        }
    }
    else {
        do {
            n++;
        } while (ui64 >>= 4);
    }
    return max(n, width);
}

Str::Str(int i, int base)
{
    char tmp[12];       // max: 37777777777
    char *tp, *pos;
    bool negative;

    ASSERT(base >= 8 && base <= 16);

    tp = tmp;
    negative = false;

    if (i < 0) {
        i = -i;
        negative = true;
    }
    do {
        *tp++ = _hex[i % base];
        i /= base;

    } while (i > 0);

    if (negative) {
        *tp++ = '-';
    }

    len_ = tp - tmp;
    buffer_ = alloc(len_);
    data_ = buffer_->data;

    pos = buffer_->data;
    do {
        *pos++ = *--tp;
    } while (tp != tmp);
}

int Str::toi() const
{
    int i, n;
    const char *pos;

    if (len_ == 0)
        return -1;

    pos = data_;
    n = len_;

    for (i = 0; n--; pos++) {
        if (*pos < '0' || *pos > '9')
            return -1;

        i = i * 10 + (*pos - '0');
    }
    if (i < 0)
        return -1;

    return i;
}

bool Str::all(int isfunc(int)) const
{
    for (size_t i = 0; i < len_; i++) {
        if (isfunc(*(data_ + i)) == 0)
            return false;
    }
    return true;
}

int Str::count(char c, int start, int end) const
{
    const char *pos;
    int n;

    if (data_ == nullptr)
        return 0;

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    n = -1;
    pos = data_ + start - 1;

    do {
        n++;
        pos++;
        pos = strnfind(pos, end - (pos - data_), c);

    } while (pos != nullptr);

    return n;
}

int Str::count(const Str& str, int start, int end) const
{
    const char *pos;
    int n;

    if (data_ == nullptr || str.data_ == nullptr)
        return 0;

    start = STR_VALID_POS(start, len_);
    end = STR_VALID_POS(end, len_);

    n = -1;
    pos = data_ + start - str.len_;

    do {
        n++;
        pos += str.len_;
        pos = strnfind(pos, end - (pos - data_), str.data_, str.len_);

    } while (pos != nullptr);

    return n;
}

Str Str::lstrip(const char *chars) const
{
    const char *start, *end;

    ASSERT(data_ != nullptr);

    start = data_;
    end = data_ + len_;

    if (chars == nullptr) {
        while (start != end && isspace(*start))
            start++;
    }
    else if (chars[0] != '\0') {
        while (start != end && strchr(chars, *start) != nullptr)
            start++;
    }
    return share(start, end - start);
}

Str Str::rstrip(const char *chars) const
{
    const char *end;

    ASSERT(data_ != nullptr);

    end = data_ + len_;

    if (chars == nullptr) {
        while (data_ != end && isspace(*(end - 1)))
            end--;
    }
    else if (chars[0] != '\0') {
        while (data_ != end && strchr(chars, *(end - 1)) != nullptr)
            end--;
    }
    return share(data_, end - data_);
}

Str Str::strip(const char *chars) const
{
    const char *start, *end;

    ASSERT(data_ != nullptr);

    start = data_;
    end = data_ + len_;

    if (chars == nullptr) {
        while (start != end && isspace(*start))
            start++;
        while (start != end && isspace(*(end - 1)))
            end--;
    }
    else if (chars[0] != '\0') {
        while (start != end && strchr(chars, *start) != nullptr)
            start++;
        while (start != end && strchr(chars, *(end - 1)) != nullptr)
            end--;
    }
    return share(start, end - start);
}

Str Str::upper() const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;

    ASSERT(data_ != nullptr);

    buffer = alloc(len_);
    pos = data_;
    end = data_ + len_;
    ptr = buffer->data;

    while (pos != end) {
        *ptr++ = toupper(*pos++);
    }
    return Str(buffer, len_);
}

Str Str::lower() const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;

    ASSERT(data_ != nullptr);

    buffer = alloc(len_);
    pos = data_;
    end = data_ + len_;
    ptr = buffer->data;

    while (pos != end) {
        *ptr++ = tolower(*pos++);
    }
    return Str(buffer, len_);
}

Str Str::capitalize() const
{
    char *pos;

    ASSERT(data_ != nullptr);

    Str dst = lower();
    pos = const_cast<char *>(dst.data_);

    if (dst.len_ > 0 && isalpha(*pos)) {
        *pos = toupper(*pos);
    }
    return dst;
}

Str Str::capitalize_each(char sep) const
{
    char *pos;
    const char *end;
    size_t n;

    ASSERT(data_ != nullptr);

    Str dst = lower();
    pos = const_cast<char *>(dst.data_);
    end = dst.data_ + dst.len_;
    n = dst.len_;

    do {
        if (n > 0 && isalpha(*pos)) {
            *pos = toupper(*pos);
        }
        pos = const_cast<char *>(strnfind(pos, n, sep));
        if (pos == nullptr)
            break;

        n = end - pos;

        while (*pos == sep) {
            pos++;
            n--;
        }
    } while (n != 0);

    return dst;
}

Str Str::translate(uint8_t table[256]) const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;

    ASSERT(data_ != nullptr);

    buffer = alloc(len_);
    pos = data_;
    end = data_ + len_;
    ptr = buffer->data;

    while (pos != end) {
        *ptr++ = table[static_cast<uint8_t>(*pos++)];
    }
    return Str(buffer, len_);
}

Str Str::translate(int isfunc(int), char c) const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;

    ASSERT(data_ != nullptr);

    buffer = alloc(len_);
    pos = data_;
    end = data_ + len_;
    ptr = buffer->data;

    while (pos != end) {
        *ptr++ = isfunc(*pos) ? c : *pos;
        pos++;
    }
    return Str(buffer, len_);
}

Str Str::remove(int isfunc(int)) const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;
    size_t n;

    ASSERT(data_ != nullptr);

    pos = data_;
    end = data_ + len_;
    n = len_;

    while (pos != end) {
        if (isfunc(*pos))
            n--;
        pos++;
    }

    pos = data_;

    buffer = alloc(n);
    ptr = buffer->data;

    while (pos != end) {
        if (isfunc(*pos) == 0)
            *ptr++ = *pos;
        pos++;
    }
    return Str(buffer, n);
}

Str Str::retain(int isfunc(int)) const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;
    size_t n;

    ASSERT(data_ != nullptr);

    pos = data_;
    end = data_ + len_;
    n = len_;

    while (pos != end) {
        if (isfunc(*pos) == 0)
            n--;
        pos++;
    }

    pos = data_;

    buffer = alloc(n);
    ptr = buffer->data;

    while (pos != end) {
        if (isfunc(*pos))
            *ptr++ = *pos;
        pos++;
    }
    return Str(buffer, n);
}

Str Str::escape() const
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;
    size_t n;

    ASSERT(data_ != nullptr);

    pos = data_;
    end = data_ + len_;
    n = len_;

    while (pos < end) {
        if (isprint(*pos) == 0) {
            if (isspace(*pos))
                n++;
            else
                n += 3;
        }
        pos++;
    }

    pos = data_;

    buffer = alloc(n);
    ptr = buffer->data;

    while (pos < end) {
        if (isprint(*pos)) {
            *ptr++ = *pos;
        }
        else if (isspace(*pos)) {
            *ptr++ = '\\';
            switch (*pos) {
            case '\t':
                *ptr++ = 't';
                break;
            case '\n':
                *ptr++ = 'n';
                break;
            case '\v':
                *ptr++ = 'v';
                break;
            case '\f':
                *ptr++ = 'f';
                break;
            case '\r':
                *ptr++ = 'r';
                break;
            }
        }
        else {
            *ptr++ = '\\';
            *ptr++ = 'x';
            *ptr++ = _hex[(*pos >> 4) & 0xf];
            *ptr++ = _hex[ *pos       & 0xf];
        }
        pos++;
    }
    return Str(buffer, n);
}

StrList Str::split(char sep) const
{
    StrList substrs;
    const char *pos, *ptr;
    size_t n;

    ASSERT(data_ != nullptr && sep != '\0');

    pos = data_;
    n = len_;

    while ((ptr = strnfind(pos, n, sep)) != nullptr) {
        substrs.push_back(share(pos, ptr - pos));
        pos = ptr + 1;
        n = len_ - (pos - data_);
    }
    substrs.push_back(share(pos, n));

    return substrs;
}

StrList Str::split(const Str& sep) const
{
    StrList substrs;
    const char *pos, *ptr;
    size_t n;

    ASSERT(data_ != nullptr && sep.len_ != 0);

    pos = data_;
    n = len_;

    while ((ptr = strnfind(pos, n, sep.data_, sep.len_)) != nullptr) {
        substrs.push_back(share(pos, ptr - pos));
        pos = ptr + sep.len_;
        n = len_ - (pos - data_);
    }
    substrs.push_back(share(pos, n));

    return substrs;
}

StrStrPair Str::split_pair(char sep) const
{
    const char *pos, *ptr;

    ASSERT(data_ != nullptr && sep != '\0');

    pos = strnfind(data_, len_, sep);

    if (pos != nullptr) {
        ptr = pos + 1;
        return make_pair(
                share(data_, pos - data_),
                share(ptr, data_ + len_ - ptr));
    }
    return make_pair(*this, nullstr);
}

StrStrPair Str::split_pair(const Str& sep) const
{
    const char *pos, *ptr;

    ASSERT(data_ != nullptr && sep.len_ != 0);

    pos = strnfind(data_, len_, sep.data_, sep.len_);

    if (pos != nullptr) {
        ptr = pos + sep.len_;
        return make_pair(
                share(data_, pos - data_),
                share(ptr, data_ + len_ - ptr));
    }
    return make_pair(*this, nullstr);
}

StrList Str::split_lines() const
{
    StrList substrs;
    const char *pos, *ptr, *end;

    ASSERT(data_ != nullptr);

    pos = data_;
    ptr = data_;
    end = data_ + len_;

    while (pos != end) {
        while (ptr != end && *ptr != '\r' && *ptr != '\n') {
            ptr++;
        }
        substrs.push_back(share(pos, ptr - pos));

        if (ptr == end)
            break;

        if (*ptr++ == '\r' && ptr != end && *ptr == '\n') {
            ptr++;
        }
        pos = ptr;
    }
    return substrs;
}

Str Str::concat(const Str& str) const
{
    str_buffer_t *buffer;
    size_t n;

    ASSERT(data_ != nullptr && str.data_ != nullptr);

    n = len_ + str.len_;

    buffer = alloc(n);
    memcpy(buffer->data, data_, len_);
    memcpy(buffer->data + len_, str.data_, str.len_);

    return Str(buffer, n);
}

Str Str::sprintf(const char *fmt, ...)
{
    str_buffer_t *buffer;
    size_t n;
    va_list args;

    va_start(args, fmt);
    n = vslprintf_len(fmt, args);
    va_end(args);

    buffer = alloc(n);

    va_start(args, fmt);
    vslprintf((char *) buffer->data, (char *) buffer->data + n, fmt, args);
    va_end(args);

    return Str(buffer, n);
}

Str Str::join(const StrList& strs) const
{
    ASSERT(data_ != nullptr);

    if (strs.size() == 0)
        return "";

    str_buffer_t *buffer;
    char *ptr;
    size_t n, i;

    n = len_ * (strs.size() - 1);

    for (auto& str : strs) {
        n += str.len_;
    }

    buffer = alloc(n);
    ptr = buffer->data;

    i = 0;
    for (auto& str : strs) {
        memcpy(ptr, str.data_, str.len_);
        ptr += str.len_;
        i++;

        if (i != strs.size()) {
            memcpy(ptr, data_, len_);
            ptr += len_;
        }
    }
    return Str(buffer, n);
}

Str Str::join(char c, const StrList& strs)
{
    if (strs.size() == 0)
        return "";

    str_buffer_t *buffer;
    char *ptr;
    size_t n, i;

    n = strs.size() - 1;

    for (auto& str : strs) {
        n += str.len_;
    }

    buffer = alloc(n);
    ptr = buffer->data;

    i = 0;
    for (auto& str : strs) {
        memcpy(ptr, str.data_, str.len_);
        ptr += str.len_;
        i++;

        if (i != strs.size()) {
            *ptr++ = c;
        }
    }
    return Str(buffer, n);
}

void Str::print() const
{
    size_t i;

    i = 0;
    do {
        putchar(*(data_ + i));
    }
    while (++i < len_);
}

} // namespace
