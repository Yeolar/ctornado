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
    Logger::initialize(Logger::VVERB);

    Buffer buffer;

    buffer.push(Str("first"));
    buffer.push(Str("second"));
    buffer.push(Str("third"));
    log_stderr("buffer size: %zu", buffer.size());
    log_stderr("buffer top: %s", buffer.top().tos().c_str());

    buffer.remove_prefix(3);
    log_stderr("buffer size: %zu", buffer.size());
    log_stderr("buffer top: %s", buffer.top().tos().c_str());

    buffer.merge_prefix(4);
    log_stderr("buffer size: %zu", buffer.size());
    log_stderr("buffer top: %s", buffer.top().tos().c_str());

    buffer.pop();
    log_stderr("buffer size: %zu", buffer.size());
    log_stderr("buffer top: %s", buffer.top().tos().c_str());

    buffer.double_prefix();
    log_stderr("buffer size: %zu", buffer.size());
    log_stderr("buffer top: %s", buffer.top().tos().c_str());

    return 0;
}
