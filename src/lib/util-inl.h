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

#ifndef __UTIL_INL_H
#define __UTIL_INL_H

#include "ctornado.h"

inline void *operator new(size_t size)
{
    void *p;

    // malloc(0) is unpredictable; avoid it.
    if (size == 0)
        size = 1;

#ifdef USE_JEMALLOC
    p = je_malloc(size);
#else
    p = malloc(size);
#endif

    while (p == nullptr) {
        std::new_handler handler = std::set_new_handler(nullptr);
        std::set_new_handler(handler);

        if (!handler)
            throw std::bad_alloc();

        handler();

#ifdef USE_JEMALLOC
        p = je_malloc(size);
#else
        p = malloc(size);
#endif
    }
    return p;
}

inline void operator delete(void *ptr) noexcept
{
    if (ptr == nullptr)
        return;

#ifdef USE_JEMALLOC
    je_free(ptr);
#else
    free(ptr);
#endif
}

namespace ctornado {

inline void *_alloc(size_t size, const char *name, int line)
{
    void *p;

    // malloc(0) is unpredictable; avoid it.
    if (size == 0)
        size = 1;

#ifdef USE_JEMALLOC
    p = je_malloc(size);
#else
    p = malloc(size);
#endif

    if (p == nullptr) {
        log(Logger::ERROR, name, line, "malloc(%zu) failed", size);
    }
    else {
        log(Logger::VVERB, name, line, "malloc(%zu) at %p", size, p);
    }
    return p;
}

inline void *_zalloc(size_t size, const char *name, int line)
{
    void *p;

    p = _alloc(size, name, line);
    if (p != nullptr) {
        memset(p, 0, size);
    }
    return p;
}

inline void _free(void *ptr, const char *name, int line)
{
    ASSERT(ptr != nullptr);

    log(Logger::VVERB, name, line, "free(%p)", ptr);
#ifdef USE_JEMALLOC
    je_free(ptr);
#else
    free(ptr);
#endif
}

} // namespace

#endif // __UTIL_INL_H

