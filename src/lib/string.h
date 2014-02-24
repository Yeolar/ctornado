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

#ifndef __STRING_H
#define __STRING_H

#include "ctornado.h"

namespace ctornado {

//
// memset low n bytes of a uint type value
//
void *memsetu(void *ptr, uint64_t value, size_t n);

//
// Find character or string in the string of n bytes (reversely)
//
const char *strnfind(const char *s, size_t n, char c);
const char *strnrfind(const char *s, size_t n, char c);
const char *strnfind(const char *s, size_t n, const char *s2, size_t n2);
const char *strnrfind(const char *s, size_t n, const char *s2, size_t n2);

//
// Similar with (v)snprintf, but return the number of characters really written
//
int scnprintf(char *buf, size_t size, const char *fmt, ...);
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);

//
// Like vsprintf, support Str type
//
char *slprintf(char *buf, char *end, const char *fmt, ...);
char *vslprintf(char *buf, char *end, const char *fmt, va_list args);
size_t vslprintf_len(const char *fmt, va_list args);

typedef struct {
    uint16_t cnt;
    char data[1];
} str_buffer_t;

#define STR_BUF_4K      (4096 - sizeof(uint16_t))

//
// There are three types of Str:
//
//  empty string    indicate nullptr string
//  unref string    string with no reference count
//  ref string      string with reference count
//
// There is no need to free the buffer of Str used, but if you create unref
// string from a (char *)str, when the str is freed, the unref string
// corresponded will be turned into undefined status. So never use an unref
// string after its source str is freed.
//
class Str
{
public:
    Str();
    Str(str_buffer_t *buffer, size_t n);
    Str(const Str& str);
    Str(Str&& str);
    Str(int i, int base=10);
    Str(const char *str, size_t n=0);
    Str(const Str& str, const char *pos, size_t n);
    ~Str();

    static str_buffer_t *alloc(size_t n);

    // Create an unref Str object from str
    static Str create(const char *str);
    // Create a ref Str object, point to the same buffer
    Str share(const char *pos, size_t n) const;
    // Create a ref Str object, alloc and copy
    Str copy() const;

    Str& operator=(const char *str);
    Str& operator=(const Str& str);
    Str& operator=(Str&& str);

    char operator[](int i) const;

    size_t len() const;
    const char *data() const;

    string tos() const;
    int toi() const;

    const char *begin() const;
    const char *end() const;

    bool eq(const Str& str) const;
    bool all(int isfunc(int)) const;
    bool null() const;
    bool empty() const;

    int find(char c, int start=0, int end=-1) const;
    int find(const Str& str, int start=0, int end=-1) const;
    int rfind(char c, int start=0, int end=-1) const;
    int rfind(const Str& str, int start=0, int end=-1) const;

    int count(char c, int start=0, int end=-1) const;
    int count(const Str& str, int start=0, int end=-1) const;

    bool starts_with(const Str& prefix) const;
    bool ends_with(const Str& suffix) const;

    void remove_prefix(size_t n);
    Str substr(int start, int end) const;

    Str lstrip(const char *chars=nullptr) const;
    Str rstrip(const char *chars=nullptr) const;
    Str strip(const char *chars=nullptr) const;

    Str upper() const;
    Str lower() const;
    Str capitalize() const;
    Str capitalize_each(char sep=' ') const;
    Str translate(uint8_t table[256]) const;
    Str translate(int isfunc(int), char c) const;
    Str remove(int isfunc(int)) const;
    Str retain(int isfunc(int)) const;

    Str escape() const;

    StrList split(char sep) const;
    StrList split(const Str& sep) const;
    StrStrPair split_pair(char sep) const;
    StrStrPair split_pair(const Str& sep) const;
    StrList split_lines() const;

    Str concat(const Str& str) const;

    static Str sprintf(const char *fmt, ...);

    Str join(const StrList& strs) const;
    static Str join(char c, const StrList& strs);
    static Str join(const char *sep, const StrList& strs);

    Str replace(const char *oldstr, const char *newstr) const;

    void print() const;
    void println() const;

private:
    size_t len_;
    const char *data_;
    str_buffer_t *buffer_;
};

struct StrLess
{
    bool operator()(const Str& s1, const Str& s2) const
    {
        if (s1.data() == nullptr && s2.data() == nullptr)
            return true;

        if (s1.data() == nullptr || s2.data() == nullptr)
            return false;

        int n = memcmp(s1.data(), s2.data(), min(s1.len(), s2.len()));

        return (n < 0 || (n == 0 && s1.len() < s2.len()));
    }
};

} // namespace

#endif // __STRING_H

