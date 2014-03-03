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

using namespace ctornado;

int main()
{
    const char *s, *p, *qs;
    Query *q;

    Logger::initialize(Logger::INFO);

    s = "http://www.yeolar.com/note/?page=3";
    p = "/?page=2";
    qs = "a=1&b=2&c=&a=3";

    log_stderr("url:            %s", s);
    URL u = URL::split(s);

    log_stderr("");
    log_stderr("    scheme:     %s", u.scheme_.tos().c_str());
    log_stderr("    netloc:     %s", u.netloc_.tos().c_str());
    log_stderr("    path:       %s", u.path_.tos().c_str());
    log_stderr("    query:      %s", u.query_.tos().c_str());
    log_stderr("    fragment:   %s", u.fragment_.tos().c_str());

    log_stderr("");
    log_stderr("Unsplit =>      %s", u.unsplit().tos().c_str());

    log_stderr("");
    log_stderr("path:           %s", p);
    log_stderr("Join url =>     %s", url_join(s, p).tos().c_str());

    log_stderr("");
    log_stderr("query:          %s", qs);
    q = Query::parse(qs);

    log_stderr("Concat url =>   %s", url_concat(s, q).tos().c_str());

    s = "http://user:pass@www.example.com:8888/path/";

    log_stderr("");
    log_stderr("url:            %s", s);
    URL v = URL::split(s);

    log_stderr("");
    log_stderr("    netloc:     %s", v.netloc_.tos().c_str());
    log_stderr("    username:   %s", v.username().tos().c_str());
    log_stderr("    password:   %s", v.password().tos().c_str());
    log_stderr("    host:       %s", v.host().tos().c_str());
    log_stderr("    port:       %d", v.port());

    return 0;
}


