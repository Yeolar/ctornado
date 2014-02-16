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

#ifndef __URLLIB_H
#define __URLLIB_H

#include "ctornado.h"

namespace ctornado {

class URL
{
public:
    URL(const Str& scheme, const Str& netloc, const Str& path,
            const Str& query, const Str& fragment)
        : scheme_(scheme)
        , netloc_(netloc)
        , path_(path)
        , query_(query)
        , fragment_(fragment) {}
    ~URL() {}

    static URL split(const Str& url, bool allow_fragments=true);
    Str unsplit();

    static bool is_uses_relative(const Str& str);
    static bool is_uses_netloc(const Str& str);

    Str scheme_;
    Str netloc_;
    Str path_;
    Str query_;
    Str fragment_;

private:
    static int split_netloc(const Str& url, int start=0);
    static bool is_scheme_chars(const Str& str);
};

class Query
{
public:
    Query() {}
    ~Query() {}

    static Query *parse(const Str& str);
    void parse_extend(const Str& str);

    Str encode();

    void add(const Str& name, const Str& value);
    Str get(const Str& name, const Str& deft=nullstr);

private:
    StrStrMMap map_;
};

Str url_join(const Str& base, const Str& url, bool allow_fragments=true);

//
// Concatenate url and query regardless of whether url has existing
// query parameters.
//
Str url_concat(const Str& url, Query *query);

} // namespace

#endif // __URLLIB_H
