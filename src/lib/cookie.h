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

#ifndef __COOKIE_H
#define __COOKIE_H

#include "ctornado.h"

namespace ctornado {

class CookieMorsel
{
public:
    CookieMorsel() {}

    void set(const Str& key, const Str& value, const Str& coded_value);
    void set(const Str& key, const Str& value);
    Str get(const Str& key);

    Str output();

    Str key_;
    Str value_;
    Str coded_value_;

private:
    ss_map_t map_;
};

class Cookie
{
public:
    Cookie() {}
    ~Cookie();

    static Str value_encode(const Str& value);
    static Str value_decode(const Str& value);

    bool has(const Str& key);
    CookieMorsel *get(const Str& key);
    void set(const Str& key, const Str& value);

    void load(const Str& data);

    Str output();

private:
    map<Str, CookieMorsel *, StrLess> map_;

    void __set(const Str& key, const Str& value, const Str& coded_value);
};

} // namespace

#endif // __COOKIE_H

