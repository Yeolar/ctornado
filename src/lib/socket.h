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

#ifndef __SOCKET_H
#define __SOCKET_H

#include "ctornado.h"

namespace ctornado {

typedef struct {
    int family;                     // socket address family
    socklen_t addrlen;              // socket address length
    union {
        struct sockaddr_in in;      // ipv4 socket address
        struct sockaddr_in6 in6;    // ipv6 socket address
        struct sockaddr_un un;      // unix domain address
    } addr;
} sockinfo_t;

typedef struct {
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
} addr_t;

bool valid_ip(const char *ip);
bool valid_port(int port);

//
// Resolve a hostname and service by translating it to socket address and
// return it in si
//
int socket_resolve(sockinfo_t *si, int family, const char *name, int port);

//
// Unresolve the socket address by translating it to host and service string
// and return in unresolve
//
int socket_unresolve_addr(addr_t *unresolve,
        struct sockaddr *addr, socklen_t addrlen);

//
// Unresolve the socket descriptor address by translating it to host and
// service string and return in unresolve
//
int socket_unresolve_descriptor(addr_t *unresolve, int sd);

//
// Unresolve the socket descriptor peer address by translating it to host
// and service string and return in unresolve
//
int socket_unresolve_peer_descriptor(addr_t *unresolve, int sd);

//
// Creates listening sockets bound to the given port and name (address).
//
// Returns a list of socket objects (multiple sockets are returned if
// the given address maps to multiple IP addresses, which is most common
// for mixed IPv4 and IPv6 use).
//
// Name may be either an IP address or hostname.  If it's a hostname,
// the server will listen on all IP addresses associated with the
// name.  Name may be an empty string or NULL to listen on all
// available interfaces.  family may be set to either AF_INET
// or AF_INET6 to restrict to ipv4 or ipv6 addresses, otherwise
// both will be used if available.
//
// The backlog argument has the same meaning as for socket.listen().
//
SocketList *bind_sockets(int port, const char *name=nullptr,
        int family=AF_UNSPEC, int backlog=128);

class Socket
{
public:
    Socket(int sd, int family, int socktype, int protocol)
        : fd_(sd), family_(family), socktype_(socktype), protocol_(protocol) {}
    ~Socket() {}

    static Socket *create(int family, int socktype, int protocol);

    void bind(const struct sockaddr *addr, socklen_t addrlen);
    void bind(const char *name, int port);
    void connect(const char *name, int port);
    void listen(int backlog);
    Socket *accept();
    void close();

    void set_close_exec();
    void set_blocking();
    void set_nonblocking();
    void set_reuseaddr();
    void set_tcpnodelay();
    void set_linger(int timeout);
    void set_ipv6only();
    void set_sndbuf(int size);
    void set_rcvbuf(int size);
    int get_sndbuf();
    int get_rcvbuf();
    int get_soerror();

    ssize_t send(const void *buf, size_t len);
    ssize_t recv(void *buf, size_t len);

    int fd_;
    int family_;
    int socktype_;
    int protocol_;
};

} // namespace

#endif // __SOCKET_H
