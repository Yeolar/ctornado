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

#ifndef __UTIL_H
#define __UTIL_H

#include "ctornado.h"

namespace ctornado {

#define NELEMS(a)           ((sizeof(a)) / sizeof((a)[0]))

#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))

#ifdef ASSERT_PANIC

#define ASSERT(_x) do {                         \
    if (!(_x)) {                                \
        _assert(#_x, __FILE__, __LINE__, 1);    \
    }                                           \
} while(0)

#define NOT_REACHED() ASSERT(0)

#elif ASSERT_LOG

#define ASSERT(_x) do {                         \
    if (!(_x)) {                                \
        _assert(#_x, __FILE__, __LINE__, 0);    \
    }                                           \
} while(0)

#define NOT_REACHED() ASSERT(0)

#else

#define ASSERT(_x)
#define NOT_REACHED()

#endif

void _assert(const char *cond, const char *name, int line, int panic);
void _stacktrace(int skip_count);

#define alloc_w(_s)                             \
    _alloc(static_cast<size_t>(_s), __FILE__, __LINE__)

#define zalloc_w(_s)                            \
    _zalloc(static_cast<size_t>(_s), __FILE__, __LINE__)

#define free_w(_p) do {                         \
    _free(_p, __FILE__, __LINE__);              \
    (_p) = nullptr;                             \
} while (0)

void *_alloc(size_t size, const char *name, int line);
void *_zalloc(size_t size, const char *name, int line);
void _free(void *ptr, const char *name, int line);

typedef struct {
    int begin;
    int end;
} range_t;

//
// Is a not-space control character
//
int isnscntrl(int c);

//
// Some file descriptor utils
//
int set_close_exec(int fd);
int set_blocking(int fd);
int set_nonblocking(int fd);

} // namespace

#endif // __UTIL_H

