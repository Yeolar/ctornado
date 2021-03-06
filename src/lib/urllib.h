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

    static bool is_uses_relative(const Str& str);
    static bool is_uses_netloc(const Str& str);

    static URL split(const Str& url, bool allow_fragments=true);
    Str unsplit();

    Str username();
    Str password();
    Str host();
    int port();

    Str scheme_;
    Str netloc_;
    Str path_;
    Str query_;
    Str fragment_;

private:
    Str username_;
    Str password_;
    Str host_;
    int port_;

    static int split_netloc(const Str& url, int start=0);
    static bool is_scheme_chars(const Str& str);

    void get_uphp();
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

//
// Construct a full (absolute) URL by combining a base URL (base)
// with another URL (url).
//
// Informally, this uses components of the base URL, in particular
// the addressing scheme, the network location and (part of) the path,
// to provide missing components in the relative URL.
//
// If the allow_fragments argument is false, fragment identifiers
// are not allowed, even if the URL’s addressing scheme normally
// does support them. The default value for this argument is true.
//
Str url_join(const Str& base, const Str& url, bool allow_fragments=true);

//
// Concatenate url and query regardless of whether url has existing
// query parameters.
//
Str url_concat(const Str& url, Query *query);

} // namespace

#endif // __URLLIB_H

