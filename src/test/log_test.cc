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
    char ascii[256];

    for (int i = 0; i < 256; i++) {
        ascii[i] = i;
    }

    Logger::initialize(Logger::INFO);

    log_critical("critical log");
    log_error("error log");
    log_warn("warning log");
    log_info("info log");
    log_debug("debug log");
    log_stderr("");

    Logger::set_level(Logger::DEBUG);

    log_critical("critical log");
    log_error("error log");
    log_warn("warning log");
    log_info("info log");
    log_debug("debug log");
    log_stderr("");

    log_hexdump(Logger::INFO, ascii, 256, "ascii hexdump");
    log_stderr("");

    Logger::set_level(Logger::ERROR);

    loga("log always");

    return 0;
}
