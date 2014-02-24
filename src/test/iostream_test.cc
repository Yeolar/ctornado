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
using namespace std::placeholders;

void send_request(IOStream *stream);
void on_headers(IOStream *stream, const Str& data);
void on_body(IOStream *stream, const Str& data);

IOLoop *ioloop = IOLoop::instance();

void send_request(IOStream *stream)
{
    log_debug("Send request to 'http://www.yeolar.com'");

    stream->write("GET / HTTP/1.0\r\nHost: www.yeolar.com\r\n\r\n");
    stream->read_until("\r\n\r\n", bind(&on_headers, stream, _1));
}

void on_headers(IOStream *stream, const Str& data)
{
    int n;

    for (auto& line : data.split("\r\n")) {
        auto kv = line.split_pair(':');

        if (kv.first.strip().eq("Content-Length")) {
            n = kv.second.strip().toi();
            log_debug("Content-Length: %d", n);

            stream->read_bytes(n, bind(&on_body, stream, _1));
            break;
        }
    }
}

void on_body(IOStream *stream, const Str& data)
{
    int n = data.len();

    log_debug("Body length: %zu", n);
    printf("%.*s\n", n, data.data());

    stream->close();
    ioloop->stop();
}

int main()
{
    Socket *sock;
    IOStream *stream;

    Logger::initialize(Logger::VERB);

    sock = Socket::create(AF_INET, SOCK_STREAM, 0);
    stream = new IOStream(sock);

    stream->connect("yeolar.com", 80, bind(&send_request, stream));
    ioloop->start();

    return 0;
}
