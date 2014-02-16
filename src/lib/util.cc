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

void _stacktrace(int skip_count)
{
#ifdef HAVE_BACKTRACE
    void *stack[64];
    char **symbols;
    int size;

    size = backtrace(stack, 64);
    symbols = backtrace_symbols(stack, size);
    if (symbols == nullptr)
        return;

    skip_count++;

    for (int i = skip_count, j = 0; i < size; i++, j++) {
        loga("[%d] %s", j, symbols[i]);
    }
    free(symbols);
#endif
}

void _assert(const char *cond, const char *name, int line, int panic)
{
    log(Logger::ERROR, name, line, "assert '%s' failed", cond);
    if (panic) {
        _stacktrace(1);
        abort();
    }
}

void *_alloc(size_t size, const char *name, int line)
{
    void *p;

    ASSERT(size != 0);

    p = malloc(size);
    if (p == nullptr) {
        log(Logger::ERROR, name, line, "malloc(%zu) failed", size);
    }
    else {
        log(Logger::VVERB, name, line, "malloc(%zu) at %p", size, p);
    }
    return p;
}

void *_zalloc(size_t size, const char *name, int line)
{
    void *p;

    p = _alloc(size, name, line);
    if (p != nullptr) {
        memset(p, 0, size);
    }
    return p;
}

void _free(void *ptr, const char *name, int line)
{
    ASSERT(ptr != nullptr);

    log(Logger::VVERB, name, line, "free(%p)", ptr);
    free(ptr);
}

int isnscntrl(int c)
{
    return iscntrl(c) && !isspace(c);
}

int set_close_exec(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFD, 0);
    if (flags < 0)
        return flags;

    return fcntl(fd, F_SETFL, flags | FD_CLOEXEC);
}

int set_blocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return flags;

    return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

int set_nonblocking(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return flags;

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

} // namespace
