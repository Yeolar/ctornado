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

namespace ctornado {

using namespace std::placeholders;

TCPServer::TCPServer(IOLoop *ioloop)
{
    ioloop_ = ioloop;
    started_ = false;
}

void TCPServer::listen(int port, const char *address)
{
    add_sockets(bind_sockets(port, address));
}

void TCPServer::add_socket(Socket *sock)
{
    if (ioloop_ == nullptr) {
        ioloop_ = IOLoop::instance();
    }
    log_verb("add socket fd(%d) to accept", sock->fd_);

    sockets_.insert({ sock->fd_, sock });

    // Adds an IOLoop event handler to accept new connections on sock.
    ioloop_->add_handler(sock->fd_,
            std::bind(&TCPServer::accept_handler, this, sock, _1, _2),
            IOLoop::READ);
}

void TCPServer::add_sockets(socket_list_t *sockets)
{
    for (auto& sock : *sockets) {
        add_socket(sock);
    }
    delete sockets;
}

void TCPServer::bind(int port, const char *address, int family, int backlog)
{
    socket_list_t *sockets;

    sockets = bind_sockets(port, address, family, backlog);

    if (started_) {
        add_sockets(sockets);
    }
    else {
        pending_sockets_.insert(pending_sockets_.end(),
                sockets->begin(), sockets->end());
        delete sockets;
    }
}

void TCPServer::start()
{
    socket_list_t sockets;

    ASSERT(!started_);

    started_ = true;
    sockets = socket_list_t(pending_sockets_);
    pending_sockets_.clear();
    add_sockets(&sockets);
}

void TCPServer::stop()
{
    for (auto& kv : sockets_) {
        ioloop_->remove_handler(kv.first);
        kv.second->close();
        delete kv.second;
        kv.second = nullptr;
    }
}

void TCPServer::handle_stream(IOStream *stream, const Str& address)
{
    throw NotImplError();
}

void TCPServer::handle_connection(Socket *sock)
{
    addr_t unresolve;
    str_buffer_t *buffer;
    size_t n;
    Str address;

    socket_unresolve_descriptor(&unresolve, sock->fd_);

    n = strlen(unresolve.host);
    buffer = Str::alloc(n);
    memcpy(buffer->data, unresolve.host, n);
    address = Str(buffer, n);

    log_verb("handle connection on fd(%d) from %s",
            sock->fd_, address.tos().c_str());

    try {
        handle_stream(new IOStream(sock, ioloop_), address);
    }
    catch (Error& e) {
        log_error("error in connection callback: %s", e.what());
    }
}

void TCPServer::accept_handler(Socket *sock, int fd, uint32_t events)
{
    Socket *client;

    while (true) {
        try {
            client = sock->accept();
        }
        catch (SocketError& e) {
            if (e.no() == EWOULDBLOCK || e.no() == EAGAIN)
                return;
            throw;
        }
        handle_connection(client);
    }
}

} // namespace
