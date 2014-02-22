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

//
// These quoting routines conform to the RFC2109 specification, which in
// turn references the character definitions from RFC2068.  They provide
// a two-way quoting algorithm.  Any non-text character is translated
// into a 4 character sequence: a forward-slash followed by the
// three-digit octal equivalent of the character.  Any '\' or '"' is
// quoted with a preceding '\' slash.
//
// Because of the way browsers really handle cookies (as opposed
// to what the RFC says) we also encode , and ;
//
static int _legal_chars[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,0,1,1,1,1,1,0,0,1,1,0,1,1,0,    // !#$%&'*+-.
    1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,    // 0-9
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,    // A-O
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,    // P-Z^_
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,    // `a-o
    1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,    // p-z|~
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

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
    return c >= 0 ? _legal_chars[c] : 0;
}

static Str _quote(const Str& str)
{
    str_buffer_t *buffer;
    const char *pos, *end;
    char *ptr;
    int i = 0, j = 0, n;
    uint8_t c;

    //
    // If the string does not need to be double-quoted,
    // then just return the string.  Otherwise, surround
    // the string in doublequotes and precede quote (with a \)
    // special characters.
    //
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

    //
    // If there aren't any doublequotes,
    // then there can't be any special characters.  See RFC 2109.
    //
    if (str.len_ < 2)
        return str;

    if (str[0] != '"' || str[-1] != '"')
        return str;

    sub = str.substr(1, str.len_ - 1);      // remove the "s
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

StrStrMap _reserved_init()
{
    StrStrMap map;

    map.insert({ "expires"  , "expires"  });
    map.insert({ "path"     , "Path"     });
    map.insert({ "comment"  , "Comment"  });
    map.insert({ "domain"   , "Domain"   });
    map.insert({ "max-age"  , "Max-Age"  });
    map.insert({ "secure"   , "secure"   });
    map.insert({ "httponly" , "httponly" });
    map.insert({ "version"  , "Version"  });

    return map;
}

static StrStrMap _reserved(_reserved_init());

void CookieMorsel::set(const Str& key, const Str& value, const Str& coded_value)
{
    if (_reserved.find(key.lower()) == _reserved.end() &&
            key.all(_is_legal_char)) {
        key_ = key;
        value_ = value;
        coded_value_ = coded_value;
    }
}

void CookieMorsel::set_attribute(const Str& key, const Str& value)
{
    Str k = key.lower();

    if (_reserved.find(k) != _reserved.end()) {
        attributes_[k] = value;
    }
}

Str CookieMorsel::get_attribute(const Str& key)
{
    try {
        return attributes_.at(key);
    }
    catch (out_of_range) {
        return nullstr;
    }
}

Str CookieMorsel::output()
{
    StrList result;

    result.push_back(Str::sprintf("%S=%S", &key_, &coded_value_));

    for (auto& kv : attributes_) {
        if (kv.second.eq(""))
            continue;

        if (kv.first.eq("secure") || kv.first.eq("httponly")) {
            result.push_back(_reserved[kv.first]);
        }
        else {
            result.push_back(Str::sprintf("%S=%S",
                        &_reserved[kv.first], &kv.second));
        }
    }
    return Str::join("; ", result);
}

Cookie::~Cookie()
{
    for (auto& kv : map_)
        delete kv.second;
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
    catch (out_of_range) {
        return nullptr;
    }
}

void Cookie::set(const Str& key, const Str& value)
{
    __set(key, value, _quote(value));
}

void Cookie::__set(const Str& key, const Str& value, const Str& coded_value)
{
    if (map_.find(key) == map_.end()) {
        map_[key] = new CookieMorsel();
    }
    map_[key]->set(key, value, coded_value);
}

void Cookie::load(const Str& data)
{
    CookieMorsel *cm = nullptr;
    RegexMatch *m = nullptr;
    Str key, value;
    size_t pos = 0;

    while (pos < data.len_) {
        if (m != nullptr)
            delete m;

        try {
            m = _cookie_pattern->exec(data.substr(pos, -1));
        }
        catch (Error& e) {
            log_vverb("Cookie regex exec error: %s", e.what());
            break;
        }
        if (m->size() == 0)      // no more cookies
            break;

        key = m->substr(1);
        value = m->substr(2);
        pos += m->get(0).end;

        // Parse the key and value in case it's metainfo
        if (key[0] == '$') {
            //
            // Ignore attributes which pertain to the cookie mechanism
            // as a whole. See RFC 2109.
            //
            if (cm)
                cm->set_attribute(key.substr(1, -1), value);
        }
        else if (_reserved.find(key.lower()) != _reserved.end()) {
            if (cm)
                cm->set_attribute(key, _unquote(value));
        }
        else {
            __set(key, _unquote(value), value);
            cm = map_[key];
        }
    }
    if (m != nullptr)
        delete m;
}

Str Cookie::output()
{
    StrList lines;

    for (auto& kv : map_) {
        lines.push_back(Str("Set-Cookie: ").concat(kv.second->output()));
    }
    return Str::join("\r\n", lines);
}

} // namespace
