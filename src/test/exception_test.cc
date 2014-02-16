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
    Logger::initialize(Logger::ERROR);

    try {
        throw Error("No errno error");
    }
    catch (Error& e) {
        log_error(e.what());
    }

    try {
        throw NotImplError();
    }
    catch (Error& e) {
        log_error(e.what());
    }

    try {
        throw IOError(EAGAIN);
    }
    catch (Error& e) {
        log_error(e.what());
    }

    try {
        throw SocketError(EBADF);
    }
    catch (Error& e) {
        log_error(e.what());
    }

    try {
        throw SocketGaiError(EAI_NONAME);
    }
    catch (Error& e) {
        log_error(e.what());
    }

    try {
        throw ValueError("Invalid value: %d", -1);
    }
    catch (Error& e) {
        log_error(e.what());
    }

    try {
        throw HTTPError(500, "please contact to <%s@site.com>", "admin");
    }
    catch (Error& e) {
        log_error(e.what());
    }

    return 0;
}
