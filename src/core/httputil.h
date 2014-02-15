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

#ifndef __HTTPUTIL_H
#define __HTTPUTIL_H

#include "ctornado.h"

namespace ctornado {

//
// Maintains Http-Header-Case for all keys.
//
// Supports multiple values per key via a pair of new methods,
// add() and get_list().  get() returns a single value per key,
// with multiple values joined by a comma.
//
class HTTPHeaders
{
public:
    HTTPHeaders() {}
    ~HTTPHeaders() {}

    //
    // Returns an instance from HTTP header text.
    //
    static HTTPHeaders *parse(const Str& str);

    //
    // Returns ture if has the given key.
    //
    bool has(const Str& name);

    //
    // Adds a new value for the given key.
    //
    void add(const Str& name, const Str& value);

    //
    // Returns the value of the given key.
    //
    Str get(const Str& name, const Str& deft=nullstr);

    //
    // Returns map of all (name, value) pairs.
    //
    ss_map_t *get_all();

    //
    // Converts a name to Http-Header-Case.
    //
    Str normalize_name(const Str& name);

private:
    ss_map_t map_;

    static Regex *normalized_header_re_;
    static ss_map_t normalized_headers_;
};

//
// Represents an HTTP file.
//
// The content_type comes from the provided HTTP header and should not be
// trusted outright given that it can be easily forged.
//
class HTTPFile
{
public:
    HTTPFile(const Str& filename, const Str& body, const Str& content_type)
        : filename_(filename), body_(body), content_type_(content_type) {}
    ~HTTPFile() {}

    Str filename_;
    Str body_;
    Str content_type_;
};

//
// Parses a form request body.
//
// Supports "application/x-www-form-urlencoded" and "multipart/form-data".
// The arguments and files parameters will be updated with the parsed contents.
//
void parse_body_arguments(const Str& content_type, const Str& body,
        Query *arguments, file_mmap_t *files);

//
// Parses a multipart/form-data body.
//
// The arguments and files parameters will be updated with the contents
// of the body.
//
void parse_multipart_form_data(const Str& boundary, const Str& data,
        Query *arguments, file_mmap_t *files);

} // namespace

#endif // __HTTPUTIL_H

