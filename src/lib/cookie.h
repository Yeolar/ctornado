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

//
// Abstract a key/value pair, which has some RFC 2109 attributes.
//
// The valid RFC 2109 attributes, which are:
//
//  - expires
//  - path
//  - comment
//  - domain
//  - max-age
//  - secure
//  - version
//  - httponly
//
// The attribute httponly specifies that the cookie is only transfered
// in HTTP requests, and is not accessible through JavaScript.
// This is intended to mitigate some forms of cross-site scripting.
//
// The keys are case-insensitive.
//
class CookieMorsel
{
public:
    CookieMorsel() {}
    ~CookieMorsel() {}

    //
    // Set the key, value and coded_value.
    //
    void set(const Str& key, const Str& value, const Str& coded_value);

    //
    // Set the attribute.
    //
    void set_attribute(const Str& key, const Str& value);

    //
    // Get the value of attribute.
    //
    Str get_attribute(const Str& key);

    //
    // Return a string representation of the Morsel, suitable
    // to be sent as an HTTP header. Header is by default "Set-Cookie:".
    //
    Str output();

    Str key_;
    Str value_;
    Str coded_value_;

private:
    StrStrMap attributes_;
};

class Cookie
{
public:
    Cookie() {}
    ~Cookie();

    //
    // Return an encoded value.
    //
    static Str value_encode(const Str& value);

    //
    // Return a decoded value.
    //
    static Str value_decode(const Str& value);

    //
    // Return true if has key.
    //
    bool has(const Str& key);

    //
    // Get morsel of the key.
    //
    CookieMorsel *get(const Str& key);

    //
    // Set key and value.
    //
    void set(const Str& key, const Str& value);

    //
    // Parse the data as an HTTP_COOKIE and add the values found there
    // as Morsels.
    //
    void load(const Str& data);

    //
    // Return a string representation suitable to be sent as HTTP headers.
    //
    Str output();

private:
    map<Str, CookieMorsel *, StrLess> map_;

    void __set(const Str& key, const Str& value, const Str& coded_value);
};

} // namespace

#endif // __COOKIE_H

