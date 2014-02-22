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

void connection_ready(Socket *sock, int fd, uint32_t events)
{
    Socket *client;

    while (true) {
        try {
            client = sock->accept();
        }
        catch (SocketError& e) {
            if (e.no() != EWOULDBLOCK && e.no() != EAGAIN)
                throw;
            return;
        }
        client->set_nonblocking();

        log_info("accept client: %s", get_peer_ip(client->fd_).tos().c_str());
        // ...

        client->close();
        delete client;
    }
}

int main()
{
    Socket *sock;
    IOLoop *ioloop;

    Logger::initialize(Logger::VERB);

    sock = Socket::create(AF_INET, SOCK_STREAM, 0);
    sock->set_reuseaddr();
    sock->set_nonblocking();
    sock->bind("", 8888);
    sock->listen(128);

    log_debug("server listening on port: 8888");

    cb_handler_t handler = std::bind(&connection_ready, sock, _1, _2);
    ioloop = IOLoop::instance();
    ioloop->add_handler(sock->fd_, handler, IOLoop::READ);
    ioloop->start();

    return 0;
}

