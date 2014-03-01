/*
 * Copyright (C) 2013 Yeolar <yeolar@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

static uint8_t _hex[] = "0123456789abcdef";

//
// Convert '0' ~ '9', 'A' ~ 'F', 'a' ~ 'f' to 0x0 ~ 0xf.
//
static uint8_t _xctoi(uint8_t c)
{
    uint8_t i;

    assert(c >= '0');

    i = c - '0';
    if (i <= 9)
        return i;

    assert(c >= 'A');

    i = c - 'A';
    if (i <= 5)
        return i + 10;

    assert(c >= 'a');

    i = c - 'a';
    if (i <= 5)
        return i + 10;

    assert(c <= 'f');

    return 0;
}

ssize_t hexlify(uint8_t *out, size_t out_len,
        const uint8_t *in, size_t in_len)
{
    uint8_t c;
    size_t len;

    len = in_len * 2;

    if (out_len < len)
        return -1;

    while (in_len-- > 0) {
        c = *in++;
        *out++ = _hex[(c >> 4) & 0xf];
        *out++ = _hex[ c       & 0xf];
    }
    return len;
}

ssize_t unhexlify(uint8_t *out, size_t out_len,
        const uint8_t *in, size_t in_len)
{
    uint8_t c1, c2;
    size_t len;

    if (in_len % 2 != 0)
        return -1;

    len = in_len / 2;

    if (out_len < len)
        return -1;

    while (in_len > 0) {
        c1 = *in++;
        c2 = *in++;
        *out++ = (_xctoi(c1) << 4) + _xctoi(c2);
        in_len -= 2;
    }
    return len;
}

