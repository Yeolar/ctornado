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

//
//  "([\w\d!#%&'~_`><@,:/\$\*\+\-\.\^\|\)\(\?\}\{\=]+?)"
//  "\s*=\s*"
//  "(\"(?:[^\\\"]|\\.)*\""
//  "|"
//  "\w{3},\s[\s\w\d-]{9,11}\s[\d:]{8}\sGMT"
//  "|"
//  "[\w\d!#%&'~_`><@,:/\$\*\+\-\.\^\|\)\(\?\}\{\=]*)"
//  "\s*;?"
//
static Regex *_cookie_pattern = Regex::compile(
    "([\\w\\d!#%&'~_`><@,:/\\$\\*\\+\\-\\.\\^\\|\\)\\(\\?\\}\\{\\=]+?)"
    "\\s*=\\s*"
    "(\"(?:[^\\\"]|\\.)*\""
    "|"
    "\\w{3},\\s[\\s\\w\\d-]{9,11}\\s[\\d:]{8}\\sGMT"
    "|"
    "[\\w\\d!#%&'~_`><@,:/\\$\\*\\+\\-\\.\\^\\|\\)\\(\\?\\}\\{\\=]*)"
    "\\s*;?");

static int _translator[] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,    // ,
    0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,    // ;
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

static int _is_legal_char(int c)
{
    return c >= 0 ? _translator[c] == 0 : 0;
}

static Str _quote(const Str& str)
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;
    int i = 0, j = 0, n;
    uint8_t c;

    if (str.all(_is_legal_char))
        return str;

    pos = str.begin();
    end = str.end();

    while (pos != end) {
        c = *pos++;

        if (c == '"' || c == '\\')
            i += 1;
        else
            j += _translator[c];
    }
    n = str.len_ + i + j*3 + 2;
    buffer = Str::alloc(n);

    ptr = buffer->data;
    *ptr++ = '"';

    pos = str.begin();

    while (pos != end) {
        c = *pos++;

        if (c == '"' || c == '\\') {
            *ptr++ = '\\';
            *ptr++ = c;
        }
        else if (_translator[c]) {
            *ptr++ = '\\';
            *ptr++ = '0' + ((c >> 6) & 07);
            *ptr++ = '0' + ((c >> 3) & 07);
            *ptr++ = '0' + (c & 07);
        }
        else
            *ptr++ = c;
    }
    *ptr++ = '"';

    return Str(buffer, n);
}

static Str _unquote(const Str& str)
{
    str_buffer_t *buffer;
    const char *pos, *end, *tmp;
    char *ptr;
    uint8_t c1, c2, c3;
    size_t n;
    Str sub;

    if (str.len_ < 2)
        return str;

    if (str[0] != '"' || str[-1] != '"')
        return str;

    sub = str.substr(1, str.len_ - 1);
    pos = sub.begin();
    end = sub.end();

    buffer = Str::alloc(sub.len_);
    ptr = buffer->data;

    while (pos < end) {
        tmp = pos;

        while (pos != end && *pos != '\\')
            pos++;
        if (pos != tmp) {
            memcpy(ptr, tmp, pos - tmp);
            ptr += pos - tmp;
        }
        if (pos != end)     // jump '\\'
            pos++;
        if (pos == end)
            break;

        if (pos + 2 < end) {
            c1 = *pos;
            c2 = *(pos + 1);
            c3 = *(pos + 2);
            if ((c1 >= '0' && c1 <= '3') &&
                (c2 >= '0' && c2 <= '7') &&
                (c3 >= '0' && c3 <= '7')) {
                *ptr++ = ((c1 - '0') << 6) + ((c2 - '0') << 3) + (c3 - '0');
                pos += 3;
            }
        }
    }
    n = ptr - buffer->data;

    return Str(buffer, n).copy();   // copy to release redundant bytes
}

ss_map_t _reserved_init()
{
    ss_map_t map;

    map.insert({ "expires", "expires" });
    map.insert({ "path", "Path" });
    map.insert({ "comment", "Comment" });
    map.insert({ "domain", "Domain" });
    map.insert({ "max-age", "Max-Age" });
    map.insert({ "secure", "secure" });
    map.insert({ "httponly", "httponly" });
    map.insert({ "version", "Version" });

    return map;
}

static ss_map_t _reserved(_reserved_init());

void CookieMorsel::set(const Str& key, const Str& value, const Str& coded_value)
{
    if (_reserved.find(key.lower()) == _reserved.end()) {
        key_ = key;
        value_ = value;
        coded_value_ = coded_value;
    }
}

void CookieMorsel::set(const Str& key, const Str& value)
{
    Str k = key.lower();

    if (_reserved.find(k) != _reserved.end()) {
        map_[k] = value;
    }
}

Str CookieMorsel::get(const Str& key)
{
    try {
        return map_.at(key);
    }
    catch (std::out_of_range) {
        return nullstr;
    }
}

Str CookieMorsel::output()
{
    s_list_t result;

    result.push_back(Str::sprintf("%S=%S", &key_, &coded_value_));

    for (auto& pair : map_) {
        if (pair.second.eq(""))
            continue;

        if (pair.first.eq("secure") || pair.first.eq("httponly")) {
            result.push_back(_reserved[pair.first]);
        }
        else {
            result.push_back(Str::sprintf("%S=%S",
                        &_reserved[pair.first], &pair.second));
        }
    }
    return Str::join("; ", result);
}

Cookie::~Cookie()
{
    for (auto& pair : map_)
        delete pair.second;
}

Str Cookie::value_encode(const Str& value)
{
    return _quote(value);
}

Str Cookie::value_decode(const Str& value)
{
    return _unquote(value);
}

bool Cookie::has(const Str& key)
{
    return map_.find(key) != map_.end();
}

CookieMorsel *Cookie::get(const Str& key)
{
    try {
        return map_.at(key);
    }
    catch (std::out_of_range) {
        return nullptr;
    }
}

void Cookie::__set(const Str& key, const Str& value, const Str& coded_value)
{
    if (map_.find(key) == map_.end()) {
        map_[key] = new CookieMorsel();
    }
    map_[key]->set(key, value, coded_value);
}

void Cookie::set(const Str& key, const Str& value)
{
    __set(key, value, _quote(value));
}

void Cookie::load(const Str& data)
{
    CookieMorsel *cm = nullptr;
    Str key, value, str = data;

    while (str.len_ > 0) {
        RegexMatch m = _cookie_pattern->exec(str);
        if (m.size() == 0)
            break;

        key = m.substr(1);
        value = m.substr(2);
        str = str.substr(m.get(0).end, -1);

        if (key[0] == '$') {
            if (cm)
                cm->set(key.substr(1, -1), value);
        }
        else if (_reserved.find(key.lower()) != _reserved.end()) {
            if (cm)
                cm->set(key, _unquote(value));
        }
        else {
            __set(key, _unquote(value), value);
            cm = map_[key];
        }
    }
}

Str Cookie::output()
{
    s_list_t lines;

    for (auto& pair : map_) {
        lines.push_back(Str("Set-Cookie: ").concat(pair.second->output()));
    }
    return Str::join("\r\n", lines);
}

} // namespace
