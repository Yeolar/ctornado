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

#ifndef __EPOLL_H
#define __EPOLL_H

#include "ctornado.h"

namespace ctornado {

const char *strevent(uint32_t event);

class EPoll
{
public:
    EPoll(int size=MAX_EVENTS);
    ~EPoll() {}

    void close();

    void add(int fd, uint32_t events);
    void modify(int fd, uint32_t events);
    void remove(int fd);

    int poll(struct epoll_event *events, int max_events, int64_t timeout);
    int poll(EventList *events, int64_t timeout);
    int poll(EventMap *events, int64_t timeout);

    static const uint32_t READ  = EPOLLIN;
    static const uint32_t WRITE = EPOLLOUT;
    static const uint32_t ERROR = EPOLLERR | EPOLLHUP;
    static const uint32_t ET    = EPOLLET;

    static const int MAX_EVENTS = 128;

    int fd_;

private:
    void ctl(int op, int fd, uint32_t events);
};

} // namespace

#endif // __EPOLL_H
