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
    const char *s = "0123456789abcdefghijklmnopqrstuvwxyz";

    Logger::initialize(Logger::INFO);

    log_stderr("Original:\n  '%s'", s);

    Str hex_str = hexlify(s);
    log_stderr("hexlify:\n  '%s'", hex_str.tos().c_str());

    Str unhex_str = unhexlify(hex_str);
    log_stderr("unhexlify:\n  '%s'", unhex_str.tos().c_str());

    return 0;
}

