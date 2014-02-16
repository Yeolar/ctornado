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

URL URL::split(const Str& url, bool allow_fragments)
{
    Str scheme, netloc, query, fragment, url0, url1, rest;
    int i, j;

    url0 = url;
    netloc = "";
    query = "";
    fragment = "";

    i = url0.find(':');

    if (i > 0) {
        url1 = url0.substr(0, i);

        if (url1.eq("http")) {
            scheme = url1.lower();
            url0 = url0.substr(i + 1, -1);

            if (url0.len_ >= 2 && url0.substr(0, 2).eq("//")) {
                j = split_netloc(url0, 2);
                netloc = url0.substr(2, j);
                url0 = url0.substr(j, -1);
            }
            if (allow_fragments) {
                j = url0.find('#');
                if (j != -1) {
                    fragment = url0.substr(j + 1, -1);
                    url0 = url0.substr(0, j);
                }
            }
            j = url0.find('?');
            if (j != -1) {
                query = url0.substr(j + 1, -1);
                url0 = url0.substr(0, j);
            }
            return URL(scheme, netloc, url0, query, fragment);
        }
        if (is_scheme_chars(url1)) {
            rest = url0.substr(i + 1, -1);
            if (rest.len_ == 0 || !rest.all(isdigit)) {
                scheme = url1.lower();
                url0 = rest;
            }
        }
    }
    if (url0.len_ >= 2 && url0.substr(0, 2).eq("//")) {
        j = split_netloc(url0, 2);
        netloc = url0.substr(2, j);
        url0 = url0.substr(j, -1);
    }
    if (allow_fragments) {
        j = url0.find('#');
        if (j != -1) {
            fragment = url0.substr(j + 1, -1);
            url0 = url0.substr(0, j);
        }
    }
    j = url0.find('?');
    if (j != -1) {
        query = url0.substr(j + 1, -1);
        url0 = url0.substr(0, j);
    }
    return URL(scheme, netloc, url0, query, fragment);
}

Str URL::unsplit()
{
    Str url = path_;

    if (netloc_.len_ != 0 || (
                scheme_.len_ != 0 &&
                is_uses_netloc(scheme_) &&
                !(url.len_ >= 2 && url.substr(0, 2).eq("//")))) {
        if (url.len_ != 0 && url[0] != '/') {
            url = Str::sprintf("/%S", &url);
        }
        url = Str::sprintf("//%S%S", &netloc_, &url);
    }
    if (scheme_.len_ != 0) {
        url = Str::sprintf("%S:%S", &scheme_, &url);
    }
    if (query_.len_ != 0) {
        url = Str::sprintf("%S?%S", &url, &query_);
    }
    if (fragment_.len_ != 0) {
        url = Str::sprintf("%S#%S", &url, &fragment_);
    }
    return url;
}

int URL::split_netloc(const Str& url, int start)
{
    static const char *delimiters = "/?#";
    int pos, found;

    pos = url.len_;

    for (int i = 0; i < 3; i++) {
        found = url.find(delimiters[i], start);
        if (found >= 0)
            pos = min(pos, found);
    }
    return pos;
}

bool URL::is_scheme_chars(const Str& str)
{
    static const Str scheme_chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789"
            "+-.");

    for (size_t i = 0; i < str.len_; i++) {
        if (scheme_chars.find(str[i]) == -1)
            return false;
    }
    return true;
}

bool URL::is_uses_relative(const Str& str)
{
    static const Str uses_relative[] = {
        "ftp", "http", "gopher", "nntp", "imap", "wais",
        "file", "mms", "https", "shttp", "prospero",
        "rtsp", "rtspu", "svn", "svn+ssh", "sftp"};

    for (size_t i = 0; i < NELEMS(uses_relative); i++) {
        if (str.eq(uses_relative[i]))
            return true;
    }
    return false;
}

bool URL::is_uses_netloc(const Str& str)
{
    static const Str uses_netloc[] = {
        "ftp", "http", "gopher", "nntp", "telnet", "imap", "wais",
        "file", "mms", "https", "shttp", "snews", "prospero",
        "rtsp", "rtspu", "rsync", "svn", "svn+ssh", "sftp", "nfs",
        "git", "git+ssh"};

    for (size_t i = 0; i < NELEMS(uses_netloc); i++) {
        if (str.eq(uses_netloc[i]))
            return true;
    }
    return false;
}

Query *Query::parse(const Str& str)
{
    Query *query;

    query = new Query();
    query->parse_extend(str);

    return query;
}

void Query::parse_extend(const Str& str)
{
    if (!str.null()) {
        for (auto& qp : str.split('&')) {
            auto kv = qp.split_pair('=');
            if (kv.second.len_ == 0)
                continue;
            map_.insert(kv);
        }
    }
}

Str Query::encode()
{
    StrList qps;

    for (auto& qp : map_) {
        qps.push_back(Str::sprintf("%S=%S", &qp.first, &qp.second));
    }

    if (qps.size() > 0)
        return Str::join('&', qps);
    else
        return nullstr;
}

void Query::add(const Str& name, const Str& value)
{
    map_.insert({ name, value });
}

Str Query::get(const Str& name, const Str& deft)
{
    auto it = map_.find(name);

    if (it != map_.end())
        return it->second;
    else
        return deft;
}

Str url_join(const Str& base, const Str& url, bool allow_fragments)
{
    if (base.len_ == 0)
        return url;

    if (url.len_ == 0)
        return base;

    URL b = URL::split(base, allow_fragments);
    URL u = URL::split(url, allow_fragments);

    if (u.scheme_.len_ == 0) {
        u.scheme_ = b.scheme_;
    }
    if (!u.scheme_.eq(b.scheme_) || !URL::is_uses_relative(u.scheme_))
        return url;

    if (URL::is_uses_netloc(u.scheme_)) {
        if (u.netloc_.len_ != 0)
            return u.unsplit();
        u.netloc_ = b.netloc_;
    }
    if (u.path_[0] == '/')
        return u.unsplit();

    if (u.path_.len_ == 0) {
        u.path_ = b.path_;
        if (u.query_.len_ == 0)
            u.query_ = b.query_;
        return u.unsplit();
    }

    auto segs1 = b.path_.split('/');
    auto segs2 = u.path_.split('/');
    StrVector segments;

    segs1.pop_back();
    segs1.insert(segs1.end(), segs2.begin(), segs2.end());

    if (segs1.size() > 0 && segs1.back().eq(".")) {
        segs1.back() = "";
    }
    for (auto& str : segs1) {
        if (!str.eq("."))
            segments.push_back(str);
    }
    while (true) {
        int i = 1;
        int n = segments.size() - 1;

        while (i < n) {
            if (segments[i].eq("..") &&
                    (segments[i-1].len_ != 0 && !segments[i-1].eq(".."))) {
                segments.erase(
                        segments.begin() + i - 1,
                        segments.begin() + i + 1);
                break;
            }
            i += 1;
        }
        if (i >= n)
            break;
    }
    if (segments.size() == 2 && segments[0].len_ == 0 && segments[1].eq("..")) {
        segments.back() = "";
    }
    else if (segments.size() >= 2 && segments.back().eq("..")) {
        segments.pop_back();
        segments.back() = "";
    }
    u.path_ = Str::join('/', StrList(segments.begin(), segments.end()));

    return u.unsplit();
}

Str url_concat(const Str& url, Query *query)
{
    Str qs;
    char c;

    if (query == nullptr)
        return url;

    qs = query->encode();
    c = url[-1];

    if (c == '?' || c == '&') {
        return url.concat(qs);
    }
    c = url.find('?') != -1 ? '&' : '?';

    return Str::sprintf("%S%c%S", &url, c, &qs);
}

} // namespace
