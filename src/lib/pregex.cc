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

Regex::~Regex()
{
    pcre_free(re_);
    if (extra_ != nullptr)
        pcre_free(extra_);
}

Regex *Regex::compile(const char *pattern, int flags)
{
    pcre *re;
    pcre_extra *extra;
    const char *err_str;
    int err_offset;

    //
    // flags (||'ed together) can be:
    //  PCRE_ANCHORED       -- Like adding ^ at start of pattern.
    //  PCRE_CASELESS       -- Like m//i
    //  PCRE_DOLLAR_ENDONLY -- Make $ match end of string regardless of \n's
    //                         No Perl equivalent.
    //  PCRE_DOTALL         -- Makes . match newlins too.  Like m//s
    //  PCRE_EXTENDED       -- Like m//x
    //  PCRE_EXTRA          --
    //  PCRE_MULTILINE      -- Like m//m
    //  PCRE_UNGREEDY       -- Set quantifiers to be ungreedy.  Individual quantifiers
    //                         may be set to be greedy if they are followed by "?".
    //  PCRE_UTF8           -- Work with UTF8 strings.
    //
    re = pcre_compile(pattern, flags, &err_str, &err_offset, nullptr);
    if (re == nullptr) {
        log_vverb("compile '%s' failed: %s", pattern, err_str);
        throw RegexError("Regex compile failed: %s", err_str);
    }
    //
    // pcre_study() returns nullptr for both errors and when it can not optimize
    // the regex.  The last argument is how one checks for errors (it is nullptr
    // if everything works, and points to an error string otherwise.
    //
    extra = pcre_study(re, 0, &err_str);
    if (err_str != nullptr) {
        log_vverb("study '%s' failed: %s", pattern, err_str);
        throw RegexError("Regex study failed: %s", err_str);
    }
    return new Regex(re, extra, pattern);
}

RegexMatch Regex::exec(const Str& str, int count, int flags)
{
    int n = (count == -1) ? 10 : count;
    int captures[n * 3];
    int m;

    //
    // flags (||'ed together) can be:
    //  PCRE_ANCHORED -- can be turned on at this time.
    //  PCRE_NOTBOL
    //  PCRE_NOTEOL
    //  PCRE_NOTEMPTY
    //
    m = pcre_exec(re_, extra_, str.data_, str.len_, 0, flags, captures, n * 3);
    if (m < 0) {
        switch (m) {
        case PCRE_ERROR_NOMATCH:
            return RegexMatch(str, nullptr, 0);
        case PCRE_ERROR_NULL:
            throw RegexError("Something is full");
        case PCRE_ERROR_BADOPTION:
            throw RegexError("A bad option is passed");
        case PCRE_ERROR_BADMAGIC:
            throw RegexError("Magic number bad (compiled re corrupt?)");
        case PCRE_ERROR_UNKNOWN_NODE:
            throw RegexError("Something kooky in the compiled re");
        case PCRE_ERROR_NOMEMORY:
            throw RegexError("Run out of memory");
        default:
            throw RegexError("Unknown error");
        }
    }
    if (m == 0) {
        if (count == -1)
            log_warn("too many substrings to fit in RegexMatch object");
        m = n;
    }
    return RegexMatch(str, captures, m);
}

RegexMatch::RegexMatch(const Str& str, int *captures, size_t size)
{
    size_t n;

    str_ = str;
    captures_ = nullptr;
    size_ = size;

    if (captures != nullptr) {
        n = sizeof(int) * size * 2;
        captures_ = reinterpret_cast<int *>(alloc_w(n));
        memcpy(captures_, captures, n);
    }
}

RegexMatch::~RegexMatch()
{
    if (captures_ != nullptr)
        free_w(captures_);
}

range_t RegexMatch::get(int i)
{
    ASSERT(i >= 0 && i < (int)size_);

    return { captures_[i*2], captures_[i*2+1] };
}

Str RegexMatch::substr(int i)
{
    ASSERT(i >= 0 && i < (int)size_);

    return str_.substr(captures_[i*2], captures_[i*2+1]);
}

size_t RegexMatch::size()
{
    return size_;
}

bool RegexMatch::empty()
{
    return size_ == 0;
}

} // namespace
