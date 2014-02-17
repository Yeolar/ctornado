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

#ifndef __PREGEX_H
#define __PREGEX_H

#include "ctornado.h"

namespace ctornado {

class RegexMatch
{
public:
    RegexMatch() : size_(0) {}
    RegexMatch(const Str& str, int *captures, size_t size);
    ~RegexMatch();

    range_t get(int i);
    Str substr(int i);
    size_t size();
    bool empty();

private:
    Str str_;
    int *captures_;
    size_t size_;
};

class Regex
{
public:
    Regex(pcre *re, pcre_extra *extra, const char *pattern)
        : re_(re), extra_(extra), pattern_(pattern) {}
    ~Regex();

    static Regex *compile(const char *pattern, int flags=0);
    RegexMatch exec(const Str& str, int count=-1, int flags=0);

private:
    pcre *re_;
    pcre_extra *extra_;
    Str pattern_;
};

} // namespace

#endif // __PREGEX_H

