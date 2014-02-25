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

void test_encode_decode(const char *data, const char *encoded_ref)
{
    log_stderr("");
    log_stderr("Original:  '%s'", data);
    log_stderr("Reference: '%s'", encoded_ref);

    Str encoded = base64_encode(data);
    log_stderr("Encoded:   '%s'", encoded.tos().c_str());

    if (!encoded.eq(encoded_ref)) {
        log_stderr("Failed: encoded string differs from reference");
        return;
    }

    Str decoded = base64_decode(encoded);
    log_stderr("Decoded:   '%s'", decoded.tos().c_str());

    if (!decoded.eq(data)) {
        log_stderr("Failed: encoded/decoded data differs from original data");
        return;
    }

    log_stderr("Passed!");
}

int main()
{
    struct test {
        const char *data;
        const char *encoded_ref;
    } tests[] = {
        { "",        ""             },
        { "1",       "MQ=="         },
        { "22",      "MjI="         },
        { "333",     "MzMz"         },
        { "4444",    "NDQ0NA=="     },
        { "55555",   "NTU1NTU="     },
        { "666666",  "NjY2NjY2"     },
        { "abc:def", "YWJjOmRlZg==" },
    };

    Logger::initialize(Logger::INFO);

    log_stderr("Base64 encoding/decoding tests:");

    for (size_t i = 0; i < NELEMS(tests); i++) {
        test_encode_decode(tests[i].data, tests[i].encoded_ref);
    }

    return 0;
}

