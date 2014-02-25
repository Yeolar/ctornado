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

#define MAX_DATA_SIZE    1024
#define MAX_ENCODED_SIZE 2048

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
    //char in[1024], out[2048];

    log_stderr("Base64 encoding/decoding tests:");

    for (size_t i = 0; i < NELEMS(tests); i++) {
        test_encode_decode(tests[i].data, tests[i].encoded_ref);
    }

    /*
    if (argc>1 && !strcmp(argv[1], "-t")) {
        memset(in, 123, sizeof(in));
        for(i=0; i<10000; i++){
            START_TIMER
            av_base64_encode(out, sizeof(out), in, sizeof(in));
            STOP_TIMER("encode")
        }
        for(i=0; i<10000; i++){
            START_TIMER
            av_base64_decode(in, out, sizeof(in));
            STOP_TIMER("decode")
        }

        for(i=0; i<10000; i++){
            START_TIMER
            av_base64_decode(NULL, out, 0);
            STOP_TIMER("syntax check")
        }
    }
    */

    return 0;
}

