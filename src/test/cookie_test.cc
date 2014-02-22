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
    const char *s =
        "$Version=\"1\"; Customer=\"WILE_E_COYOTE\"; $Path=\"/acme\"";
    const char *id = "yeolar@gmail.com";
    Cookie *cookie;
    CookieMorsel *morsel;

    Logger::initialize(Logger::INFO);

    log_stderr("Encode 'abc':       %s",
            Cookie::value_encode("abc").tos().c_str());
    log_stderr("Encode 'a\"b\\c':     %s",
            Cookie::value_encode("a\"b\\c").tos().c_str());

    cookie = new Cookie();

    log_stderr("Load cookie:        %s", s);
    cookie->load(s);

    log_stderr("Cookie set ID:      %s", id);
    cookie->set("ID", id);

    log_stderr("Cookie set ID expires to now");
    morsel = cookie->get("ID");
    morsel->set_attribute("expires", format_email_date(sec_now()));

    log_stderr("Cookie:             %s", cookie->output().escape().tos().c_str());

    return 0;
}

