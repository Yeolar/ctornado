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

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

ssize_t hexlify(uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len)
{
    uint8_t c, x;
    size_t len;

    len = in_len * 2;

    if (out_len < len)
        return -1;

    while (in_len-- > 0) {
        c = *in++;

        x = (c >> 4) & 0xf;
        *out++ = (x > 9) ? x + 'a' - 10 : x + '0';
        x = c & 0xf;
        *out++ = (x > 9) ? x + 'a' - 10 : x + '0';
    }
    return len;
}

