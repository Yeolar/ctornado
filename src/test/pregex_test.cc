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
    Regex *re;
    RegexMatch m;
    const char *pattern;
    const Str test_strs[] = {
        "This should match... ctornado",
        "This could match... ctornado!",
        "More than one ctornado... ctornado",
        "No chance of a match..." };

    Logger::initialize(Logger::INFO);

    pattern = "(.*)(ctornado)+";
    log_stderr("Regex pattern to use: %s", pattern);

    // Compile the regex string.
    try {
        re = Regex::compile(pattern);
    }
    catch (Error& e) {
        log_error("regex compile error: %s", e.what());
        exit(1);
    }

    for (size_t i = 0; i < NELEMS(test_strs); i++) {
        log_stderr("String: %s", test_strs[i].tos().c_str());
        log_stderr("        %s", "0123456789012345678901234567890123456789");
        log_stderr("        %s", "0         1         2         3");

        // Try to find the regex in the str, and report results.
        try {
            m = re->exec(test_strs[i]);
        }
        catch (Error& e) {
            log_error("regex exec error: %s", e.what());
            continue;
        }

        for (size_t j = 0; j < m.size(); j++) {
            range_t pos = m.get(j);
            log_stderr("Match(%d/%d): [%2d:%2d]: '%s'",
                    j, m.size() - 1, pos.begin, pos.end,
                    m.substr(j).tos().c_str());
        }
    }
    delete re;

    return 0;
}
