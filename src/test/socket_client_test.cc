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

#define BUFLEN      128

void print(Socket *sock)
{
    int n;
    char buf[BUFLEN];

    try {
        n = sock->recv(buf, BUFLEN);
        write(STDOUT_FILENO, buf, n);
    }
    catch (Error& e) {
        log_error("recv error: %s", e.what());
    }
}

int main(int argc, char *argv[])
{
    Socket *sock;

    Logger::initialize(Logger::INFO);

    if (argc != 2) {
        log_stderr("Usage: ./socket_client_test <hostname>");
        exit(1);
    }

    sock = Socket::create(AF_INET, SOCK_STREAM, 0);
    if (sock == nullptr)
        exit(1);

    try {
        sock->connect(argv[1], 8888);
    }
    catch (Error& e) {
        log_error("connect error: %s", e.what());
        exit(1);
    }
    print(sock);
    exit(0);
}
