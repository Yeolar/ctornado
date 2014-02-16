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

#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#include "ctornado.h"

namespace ctornado {

//
// A non-blocking, single-threaded TCP server.
//
// To use TCPServer, define a subclass which overrides the handle_stream
// method.
//
class TCPServer
{
public:
    TCPServer(IOLoop *ioloop=nullptr);
    virtual ~TCPServer() {}

    //
    // Starts accepting connections on the given port.
    //
    // This method may be called more than once to listen on multiple ports.
    // listen takes effect immediately; it is not necessary to call
    // TCPServer.start afterwards.  It is, however, necessary to start
    // the IOLoop.
    //
    void listen(int port, const char *address=nullptr);

    //
    // Singular version of add_sockets.  Takes a single socket object.
    //
    void add_socket(Socket *sock);

    //
    // Makes this server start accepting connections on the given sockets.
    //
    // The sockets parameter is a list of socket objects such as
    // those returned by bind_sockets.
    // add_sockets is typically used in combination with that
    // method and fork_processes to provide greater control over
    // the initialization of a multi-process server.
    //
    void add_sockets(socket_list_t *sockets);

    //
    // Binds this server to the given port on the given address.
    //
    // To start the server, call start. If you want to run this server
    // in a single process, you can call listen as a shortcut to the
    // sequence of bind and start calls.
    //
    // Address may be either an IP address or hostname.  If it's a hostname,
    // the server will listen on all IP addresses associated with the
    // name.  Address may be an empty string or NULL to listen on all
    // available interfaces.  Family may be set to either AF_INET
    // or AF_INET6 to restrict to ipv4 or ipv6 addresses, otherwise
    // both will be used if available.
    //
    // The backlog argument has the same meaning as for socket.listen.
    //
    // This method may be called multiple times prior to start to listen
    // on multiple ports or interfaces.
    //
    void bind(int port, const char *address=nullptr,
            int family=AF_UNSPEC, int backlog=128);

    //
    // Starts this server in the IOLoop.
    //
    // By default, we run the server in this process and do not fork any
    // additional child process.
    //
    // If num_processes <= 0, we detect the number of cores
    // available on this machine and fork that number of child
    // processes. If num_processes is given and > 1, we fork that
    // specific number of sub-processes.
    //
    // Since we use processes and not threads, there is no shared memory
    // between any server code.
    //
    void start();

    //
    // Stops listening for new connections.
    //
    // Requests currently in progress may still continue after the
    // server is stopped.
    //
    void stop();

    //
    // Override to handle a new IOStream from an incoming connection.
    //
    virtual void handle_stream(IOStream *stream, const Str& address);

    IOLoop *ioloop_;

private:
    void handle_connection(Socket *sock);
    void accept_handler(Socket *sock, int fd, uint32_t events);

    map<int, Socket *> sockets_;
    socket_list_t pending_sockets_;
    bool started_;
};

} // namespace

#endif // __TCPSERVER_H

