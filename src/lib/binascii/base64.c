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

static uint8_t b64_enc_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char b64_dec_table[] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1, 0,-1,-1, /* Note PAD->0 */
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

#define BASE64_PAD      '='

/*
 * Find and return the next valid character for base64, or -1 if none
 */
static int _find_valid(uint8_t *s, size_t len)
{
    int ret = -1;
    uint8_t c;

    while (len > 0 && ret == -1) {
        c = *s++;
        if (c <= 0x7f && b64_dec_table[c & 0x7f] != -1) {
            ret = c;
        }
        len--;
    }
    return ret;
}

size_t base64_encode_len(size_t len)
{
    return len > 0 ? (len + 2) / 3 * 4 : 0;
}

ssize_t base64_encode(uint8_t *out, size_t out_len,
        const uint8_t *in, size_t in_len)
{
    ssize_t len;
    unsigned int leftchar = 0;
    int leftbits = 0;

    len = base64_encode_len(in_len);

    if (out_len < len)
        return -1;

    for (; in_len > 0; in_len--, in++) {
        /* shift the data into buffer */
        leftchar = (leftchar << 8) | *in;
        leftbits += 8;

        /* see if there are 6-bit groups ready */
        while (leftbits >= 6) {
            leftbits -= 6;
            *out++ = b64_enc_table[(leftchar >> leftbits) & 0x3f];
        }
    }
    if (leftbits == 2) {
        *out++ = b64_enc_table[(leftchar & 0x3) << 4];
        *out++ = BASE64_PAD;
        *out++ = BASE64_PAD;
    }
    else if (leftbits == 4) {
        *out++ = b64_enc_table[(leftchar & 0xf) << 2];
        *out++ = BASE64_PAD;
    }
    return len;
}

size_t base64_decode_len(const uint8_t *s, size_t len)
{
    const uint8_t *t = s;

    if (len == 0)
        return 0;

    while (len > 0 && b64_dec_table[*t] != -1 && *t != BASE64_PAD) {
        t++;
        len--;
    }
    len = t - s;

    return (len + 3) / 4 * 3 - (4 - len % 4) % 4;
}

ssize_t base64_decode(uint8_t *out, size_t out_len, uint8_t *in, size_t in_len)
{
    ssize_t len = 0;
    uint8_t c;
    unsigned int leftchar = 0;
    int leftbits = 0;
    int quad_pos = 0;

    for (; in_len > 0; in_len--, in++) {
        c = *in;

        if (c > 0x7f || c == '\r' || c == '\n' || c == ' ')
            continue;

        /* check for pad sequences and ignore the invalid ones */
        if (c == BASE64_PAD) {
            if (quad_pos < 2 ||
                    (quad_pos == 2 &&
                     _find_valid(in + 1, in_len - 1) != BASE64_PAD))
                continue;
            else {
                /* a pad sequence means no more input.
                 * we've already interpreted the data
                 * from the quad at this point.
                 */
                leftbits = 0;
                break;
            }
        }
        c = b64_dec_table[*in];
        if (c == (uint8_t) -1)
            continue;

        /* shift it in on the low end, and see if there's
         * a byte ready for output.
         */
        quad_pos = (quad_pos + 1) & 0x3;
        leftchar = (leftchar << 6) | c;
        leftbits += 6;

        if (leftbits >= 8) {
            if (len == out_len)
                break;
            leftbits -= 8;
            *out++ = (leftchar >> leftbits) & 0xff;
            len++;
            leftchar &= (1 << leftbits) - 1;
        }
    }
    if (leftbits != 0)
        return -1;

    return len;
}

