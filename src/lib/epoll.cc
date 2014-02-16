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

const uint32_t EPoll::READ;
const uint32_t EPoll::WRITE;
const uint32_t EPoll::ERROR;

const int EPoll::MAX_EVENTS;

EPoll::EPoll()
{
    int fd;

    fd = epoll_create(MAX_EVENTS);
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

event_list_t *EPoll::poll(int64_t timeout)
{
    event_list_t *event_list;
    struct epoll_event evts[MAX_EVENTS];
    int fd;
    uint32_t events;
    int n;

    event_list = new event_list_t();

    n = epoll_wait(fd_, evts, MAX_EVENTS, timeout);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            fd = evts[i].data.fd;
            events = evts[i].events;

            log_vverb("epoll_wait on fd(%d) poll out event(%d, %#x)",
                    fd_, fd, events);

            event_list->push_back(make_pair(fd, events));
        }
    }
    if (n == 0) {
        log_vverb("epoll_wait on fd(%d) returned no events", fd_);
    }
    if (n < 0) {
        log_vverb("epoll_wait on fd(%d) failed: %s", fd_, strerror(errno));
        delete event_list;
        throw IOError(errno);
    }
    return event_list;
}

void EPoll::ctl(int op, int fd, uint32_t events)
{
    struct epoll_event evt;

    memset(&evt, 0, sizeof(evt));
    evt.events = events;
    evt.data.fd = fd;

    if (epoll_ctl(fd_, op, fd, &evt) < 0) {
        log_vverb("epoll_ctl on fd(%d) with event(%d, %#x) failed: %s",
                fd_, fd, strerror(errno));
        throw IOError(errno);
    }
}

} // namespace
