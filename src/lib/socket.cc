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

#define SOCK_UNIX_ADDR_STRLEN           \
    (sizeof(struct sockaddr_un) - offsetof(struct sockaddr_un, sun_path))
#define SOCK_INET4_ADDR_STRLEN          \
    (sizeof("255.255.255.255") - 1)
#define SOCK_INET6_ADDR_STRLEN          \
    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") - 1)
#define SOCK_INET_ADDR_STRLEN           \
    MAX(SOCK_INET4_ADDR_STRLEN, SOCK_INET6_ADDR_STRLEN)

Str get_peer_ip(int fd)
{
    addr_t unresolve;
    str_buffer_t *buffer;
    size_t n;

    socket_unresolve_peer_descriptor(&unresolve, fd);

    n = strlen(unresolve.host);
    buffer = Str::alloc(n);
    memcpy(buffer->data, unresolve.host, n);

    return Str(buffer, n);
}

int get_peer_port(int fd)
{
    addr_t unresolve;

    socket_unresolve_peer_descriptor(&unresolve, fd);

    return atoi(unresolve.service);
}

bool valid_ip(const char *ip)
{
    struct addrinfo *ai, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ip, 0, &hints, &ai) != 0)
        return false;

    return ai != nullptr;
}

bool valid_port(int port)
{
    return port > 0 && port <= UINT16_MAX;
}

static int _resolve_unix(sockinfo_t *si, const char *name)
{
    struct sockaddr_un *un;

    if (strlen(name) >= SOCK_UNIX_ADDR_STRLEN)
        return -1;

    un = &si->addr.un;
    un->sun_family = AF_UNIX;
    strcpy(un->sun_path, name);

    si->family = AF_UNIX;
    si->addrlen = sizeof(*un);

    return 0;
}

static int _resolve_inet(sockinfo_t *si, int family, const char *name, int port)
{
    struct addrinfo *ai, hints;
    char service[UINTMAX_MAXLEN];
    int status;

    ASSERT(valid_port(port));

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_family = family;           // AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM;

    if (name == nullptr || *name == '\0') {
        //
        // If AI_PASSIVE flag is specified, and name is nullptr or "",
        // the returned socket address will contain the wildcard IP address.
        //
        name = nullptr;
        hints.ai_flags |= AI_PASSIVE;
    }

    snprintf(service, UINTMAX_MAXLEN, "%d", port);

    status = getaddrinfo(name, service, &hints, &ai);
    if (status != 0) {
        log_vverb("address resolution of %s:%s failed: %s",
                name, service, gai_strerror(status));
        return -1;
    }

    //
    // getaddrinfo can return a linked list of more than one addrinfo, just
    // use the first address from this collection.
    //
    // The sorting function used within getaddrinfo is defined in RFC 3484,
    // the order can be tweaked for a particular system by editing /etc/gai.conf
    //
    if (ai != nullptr) {
        si->family = ai->ai_family;
        si->addrlen = ai->ai_addrlen;
        memcpy(&si->addr, ai->ai_addr, si->addrlen);

        freeaddrinfo(ai);
        return 0;
    }
    return -1;
}

int socket_resolve(sockinfo_t *si, int family, const char *name, int port)
{
    if (family == AF_UNIX && name != nullptr)
        return _resolve_unix(si, name);

    return _resolve_inet(si, family, name, port);
}

int socket_unresolve_addr(addr_t *unresolve,
        struct sockaddr *addr, socklen_t addrlen)
{
    if (getnameinfo(addr, addrlen,
                unresolve->host, sizeof(unresolve->host),
                unresolve->service, sizeof(unresolve->service),
                NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
        return -1;
    }
    return 0;
}

int socket_unresolve_descriptor(addr_t *unresolve, int sd)
{
    struct sockaddr addr;
    socklen_t addrlen;

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if (getsockname(sd, &addr, &addrlen) < 0)
        return -1;

    return socket_unresolve_addr(unresolve, &addr, addrlen);
}

int socket_unresolve_peer_descriptor(addr_t *unresolve, int sd)
{
    struct sockaddr addr;
    socklen_t addrlen;

    memset(&addr, 0, sizeof(addr));
    addrlen = sizeof(addr);

    if (getpeername(sd, &addr, &addrlen) < 0)
        return -1;

    return socket_unresolve_addr(unresolve, &addr, addrlen);
}

SocketList *bind_sockets(int port, const char *name, int family, int backlog)
{
    SocketList *sockets;
    struct addrinfo *cai, *ai, hints;
    char service[UINTMAX_MAXLEN];
    int status;
    Socket *sock;

    ASSERT(valid_port(port));

    memset(&hints, 0, sizeof(hints));
    // AI_ADDRCONFIG ensures that we only try to bind on ipv6
    // if the system is configured for it
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(service, UINTMAX_MAXLEN, "%d", port);

    status = getaddrinfo(name, service, &hints, &ai);
    if (status != 0) {
        log_vverb("address resolution of %s:%s failed: %s",
                name, service, gai_strerror(status));
        return nullptr;
    }

    sockets = new SocketList;

    for (cai = ai; cai != nullptr; cai = cai->ai_next) {
        sock = Socket::create(cai->ai_family, cai->ai_socktype,
                cai->ai_protocol);
        if (sock == nullptr)
            continue;

        try {
            sock->set_close_exec();
            sock->set_reuseaddr();
            //
            // On linux, ipv6 sockets accept ipv4 too by default,
            // but this makes it impossible to bind to both
            // 0.0.0.0 in ipv4 and :: in ipv6.  Disable ipv4 on our
            // ipv6 sockets and use a separate ipv4 socket when needed.
            //
            if (cai->ai_family == AF_INET6) {
                sock->set_ipv6only();
            }
            sock->set_nonblocking();
            sock->bind(cai->ai_addr, cai->ai_addrlen);
            sock->listen(backlog);
        }
        catch (SocketError& e) {
            delete sock;
            continue;
        }
        sockets->push_back(sock);
    }
    return sockets;
}

Socket *Socket::create(int family, int socktype, int protocol)
{
    int sd;

    sd = ::socket(family, socktype, protocol);
    if (sd == -1) {
        log_error("create socket failed: %s", strerror(errno));
        return nullptr;
    }
    return new Socket(sd, family, socktype, protocol);
}

void Socket::bind(const struct sockaddr *addr, socklen_t addrlen)
{
    if (::bind(fd_, addr, addrlen) == -1) {
        log_vverb("bind on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::bind(const char *name, int port)
{
    sockinfo_t si;
    struct sockaddr *addr;

    socket_resolve(&si, family_, name, port);
    addr = reinterpret_cast<struct sockaddr *>(&si.addr);

    if (::bind(fd_, addr, si.addrlen) == -1) {
        log_vverb("bind %s:%d failed: %s", name, port, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::connect(const char *name, int port)
{
    sockinfo_t si;
    struct sockaddr *addr;

    socket_resolve(&si, family_, name, port);
    addr = reinterpret_cast<struct sockaddr *>(&si.addr);

    if (::connect(fd_, addr, si.addrlen) == -1) {
        log_vverb("connect %s:%d failed: %s", name, port, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::listen(int backlog)
{
    if (::listen(fd_, backlog) == -1) {
        log_vverb("listen fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
}

Socket *Socket::accept()
{
    struct sockaddr addr;
    socklen_t addrlen;
    int sd;

    addrlen = sizeof(addr);

    sd = ::accept(fd_, &addr, &addrlen);
    if (sd == -1) {
        log_vverb("accept on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
    return new Socket(sd, family_, socktype_, protocol_);
}

void Socket::close()
{
    ::close(fd_);
}

void Socket::set_close_exec()
{
    if (ctornado::set_close_exec(fd_) == -1) {
        log_vverb("set FD_CLOEXEC on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_blocking()
{
    if (ctornado::set_blocking(fd_) == -1) {
        log_vverb("set O_NONBLOCK on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_nonblocking()
{
    if (ctornado::set_nonblocking(fd_) == -1) {
        log_vverb("unset O_NONBLOCK on fd(%d) failed: %s",
                fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_reuseaddr()
{
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);

    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, len) < 0) {
        log_vverb("set SO_REUSEADDR on fd(%d) failed: %s",
                fd_, strerror(errno));
        throw SocketError(errno);
    }
}

//
// Disable Nagle algorithm on TCP socket.
//
// This option helps to minimize transmit latency by disabling coalescing
// of data to fill up a TCP segment inside the kernel. Sockets with this
// option must use readv or writev to do data transfer in bulk and hence
// avoid the overhead of small packets.
//
void Socket::set_tcpnodelay()
{
    int nodelay;
    socklen_t len;

    nodelay = 1;
    len = sizeof(nodelay);

    if (setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &nodelay, len) < 0) {
        log_vverb("set TCP_NODELAY on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_linger(int timeout)
{
    struct linger linger;
    socklen_t len;

    linger.l_onoff = 1;
    linger.l_linger = timeout;
    len = sizeof(linger);

    if (setsockopt(fd_, SOL_SOCKET, SO_LINGER, &linger, len) < 0) {
        log_vverb("set SO_LINGER(%d) on fd(%d) failed: %s",
                timeout, fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_ipv6only()
{
    int v6only;
    socklen_t len;

    v6only = 1;
    len = sizeof(v6only);

    if (setsockopt(fd_, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, len) < 0) {
        log_vverb("set IPV6_V6ONLY on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_sndbuf(int size)
{
    socklen_t len;

    len = sizeof(size);

    if (setsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &size, len) < 0) {
        log_vverb("set SO_SNDBUF(%d) on fd(%d) failed: %s",
                size, fd_, strerror(errno));
        throw SocketError(errno);
    }
}

void Socket::set_rcvbuf(int size)
{
    socklen_t len;

    len = sizeof(size);

    if (setsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &size, len) < 0) {
        log_vverb("set SO_RCVBUF(%d) on fd(%d) failed: %s",
                size, fd_, strerror(errno));
        throw SocketError(errno);
    }
}

int Socket::get_sndbuf()
{
    int size;
    socklen_t len;

    len = sizeof(size);

    if (getsockopt(fd_, SOL_SOCKET, SO_SNDBUF, &size, &len) < 0) {
        log_vverb("get SO_SNDBUF on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
    return size;
}

int Socket::get_rcvbuf()
{
    int size;
    socklen_t len;

    len = sizeof(size);

    if (getsockopt(fd_, SOL_SOCKET, SO_RCVBUF, &size, &len) < 0) {
        log_vverb("get SO_RCVBUF on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
    return size;
}

int Socket::get_soerror()
{
    int err;
    socklen_t len;

    len = sizeof(err);

    if (getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        log_vverb("get SO_ERROR on fd(%d) failed: %s", fd_, strerror(errno));
        throw SocketError(errno);
    }
    return err;
}

ssize_t Socket::send(const void *buf, size_t len)
{
    ssize_t n;

    while (true) {
        n = ::send(fd_, buf, len, 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            throw SocketError(errno);
        }
        return n;
    }
}

ssize_t Socket::recv(void *buf, size_t len)
{
    ssize_t n;

    while (true) {
        n = ::recv(fd_, buf, len, 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            throw SocketError(errno);
        }
        return n;
    }
}

} // namespace
