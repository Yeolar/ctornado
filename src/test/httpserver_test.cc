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

void handle_request(HTTPRequest *request)
{
    Str message;

    message = Str::sprintf("You requested %S\n", &request->uri_);
    request->write(Str::sprintf(
                "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%S",
                message.len(), &message));
    request->finish();
}

int main()
{
    HTTPServer *server;

    Logger::initialize(Logger::VERB);

    server = new HTTPServer(handle_request);
    server->listen(8888);

    IOLoop::instance()->start();

    return 0;
}
