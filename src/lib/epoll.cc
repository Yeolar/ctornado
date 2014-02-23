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

const char *strevent(uint32_t event)
{
    if (event & (EPOLLIN | EPOLLOUT))
        return "RW";
    if (event & EPOLLIN)
        return "R";
    if (event & EPOLLOUT)
        return "W";
    return "-";
}

const uint32_t EPoll::READ;
const uint32_t EPoll::WRITE;
const uint32_t EPoll::ERROR;
const uint32_t EPoll::ET;

const int EPoll::MAX_EVENTS;

EPoll::EPoll(int size)
{
    int fd;

    ASSERT(size > 0);

    fd = epoll_create(size);
    if (fd == -1) {
        log_panic("epoll_create failed: %s", strerror(errno));
    }
    fd_ = fd;
}

void EPoll::close()
{
    ::close(fd_);
}

void EPoll::add(int fd, uint32_t events)
{
    ctl(EPOLL_CTL_ADD, fd, events);
}

void EPoll::modify(int fd, uint32_t events)
{
    ctl(EPOLL_CTL_MOD, fd, events);
}

void EPoll::remove(int fd)
{
    ctl(EPOLL_CTL_DEL, fd, 0);
}

int EPoll::poll(struct epoll_event *events, int max_events, int64_t timeout)
{
    int n;

    n = epoll_wait(fd_, events, max_events, timeout);
    if (n >= 0) {
        log_vverb("epoll_wait on fd(%d) poll out %d event(s)", fd_, n);
    }
    if (n < 0) {
        log_vverb("epoll_wait on fd(%d) failed: %s", fd_, strerror(errno));
        throw IOError(errno);
    }
    return n;
}

int EPoll::poll(EventList *events, int64_t timeout)
{
    struct epoll_event evts[MAX_EVENTS];
    int n;
    int fd;
    uint32_t evt;

    n = poll(evts, MAX_EVENTS, timeout);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            fd = evts[i].data.fd;
            evt = evts[i].events;

            log_vverb("epoll_wait on fd(%d) poll out event(%d, %s)",
                    fd_, fd, strevent(evt));

            events->push_back(make_pair(fd, evt));
        }
    }
    return n;
}

int EPoll::poll(EventMap *events, int64_t timeout)
{
    struct epoll_event evts[MAX_EVENTS];
    int n;
    int fd;
    uint32_t evt;

    n = poll(evts, MAX_EVENTS, timeout);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            fd = evts[i].data.fd;
            evt = evts[i].events;

            log_vverb("epoll_wait on fd(%d) poll out event(%d, %s)",
                    fd_, fd, strevent(evt));

            events->insert(make_pair(fd, evt));
        }
    }
    return n;
}

void EPoll::ctl(int op, int fd, uint32_t events)
{
    struct epoll_event evt;

    memset(&evt, 0, sizeof(evt));
    evt.events = events;
    evt.data.fd = fd;

    if (epoll_ctl(fd_, op, fd, &evt) < 0) {
        log_vverb("epoll_ctl on fd(%d) with event(%d, %s) failed: %s",
                fd_, fd, strevent(events), strerror(errno));
        throw IOError(errno);
    }
}

} // namespace
