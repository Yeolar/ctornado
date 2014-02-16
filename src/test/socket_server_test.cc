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

#define BUFLEN  128

void serve(Socket *sock)
{
    Socket *client;
    addr_t unresolve;
    FILE *fp;
    char buf[BUFLEN];

    for (;;) {
        try {
            client = sock->accept();

            socket_unresolve_descriptor(&unresolve, client->fd_);
            log_info("accept client: %s", unresolve.host);
        }
        catch (Error& e) {
            log_error("accept error: %s", e.what());
            break;
        }

        if ((fp = popen("/usr/bin/uptime", "r")) == NULL) {
            sprintf(buf, "error: %s\n", strerror(errno));
            client->send(buf, strlen(buf));
        }
        else {
            while (fgets(buf, BUFLEN, fp) != NULL) {
                client->send(buf, strlen(buf));
            }
            pclose(fp);
        }
        client->close();
    }
}

int main(int argc, char *argv[])
{
    Socket *sock;

    Logger::initialize(Logger::VVERB);

    sock = Socket::create(AF_INET, SOCK_STREAM, 0);
    if (sock == nullptr)
        exit(1);

    try {
        sock->bind("localhost", 8888);
        sock->listen(10);
    }
    catch (Error& e) {
        log_error("init error: %s", e.what());
        exit(1);
    }
    serve(sock);
    exit(1);
}

