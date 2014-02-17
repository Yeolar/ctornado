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

int main(int argc, char *argv[])
{
    EPoll poll;
    Socket *sock, *client;
    SocketMap sock_map;
    EventList event_list;
    int fd;
    uint32_t events;
    int n;

    Logger::initialize(Logger::INFO);

    sock = Socket::create(AF_INET, SOCK_STREAM, 0);
    sock->bind("localhost", 8888);
    sock->set_nonblocking();
    sock->listen(128);

    poll.add(sock->fd_, EPOLLIN | EPOLLET);

    // The event loop
    for (;;) {
        poll.poll(&event_list, -1);

        while (!event_list.empty()) {
            auto kv = event_list.back();
            event_list.pop_back();

            fd = kv.first;
            events = kv.second;

            if ((events & EPOLLERR) ||
                (events & EPOLLHUP) ||
                !(events & EPOLLIN)) {
                //
                // An error has occured on this fd, or the socket is not
                // ready for reading (why were we notified then?)
                //
                log_error("epoll error");

                auto it = sock_map.find(fd);

                if (it != sock_map.end()) {
                    client = it->second;
                    client->close();
                    delete client;

                    sock_map.erase(it);
                }
            }
            else if (fd == sock->fd_) {
                //
                // We have a notification on the listening socket, which
                // means one or more incoming connections.
                //
                for (;;) {
                    try {
                        client = sock->accept();
                    }
                    catch (Error& e) {
                        if (e.no() == EAGAIN || e.no() == EWOULDBLOCK) {
                            // We have processed all incoming connections.
                            break;
                        }
                        else {
                            log_error("accept error: %s", e.what());
                            break;
                        }
                    }
                    log_info("Accept connection on fd(%d)", client->fd_);

                    //
                    // Make the incoming socket non-blocking and add it to the
                    // map of socket to monitor.
                    //
                    client->set_nonblocking();

                    sock_map.insert(make_pair(client->fd_, client));
                    poll.add(client->fd_, EPOLLIN | EPOLLET);
                }
            }
            else {
                //
                // We have data on the fd waiting to be read. Read and
                // display it. We must read whatever data is available
                // completely, as we are running in edge-triggered mode
                // and won't get a notification again for the same data.
                //
                bool done = false;
                char buf[4096];

                for (;;) {
                    try {
                        client = sock_map.at(fd);
                    }
                    catch (out_of_range) {
                        break;
                    }

                    try {
                        n = client->recv(buf, sizeof(buf));
                    }
                    catch (Error& e) {
                        if (e.no() != EAGAIN) {
                            log_error("recv error: %s", e.what());
                            done = true;
                        }
                        break;
                    }
                    if (n == 0) {
                        done = true;
                        break;
                    }

                    if (write(STDOUT_FILENO, buf, n) < 0) {
                        log_error("write error: %s", strerror(errno));
                    }
                }

                if (done) {
                    log_info("Close connection on fd(%d)", client->fd_);

                    auto it = sock_map.find(client->fd_);
                    if (it != sock_map.end()) {
                        sock_map.erase(it);
                    }
                    client->close();
                    delete client;
                }
            }
        }
    }
    sock->close();

    return 0;
}
