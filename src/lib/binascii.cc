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

Str hexlify(const Str& str)
{
    str_buffer_t *buffer;
    size_t n;

    n = str.len() * 2;
    buffer = Str::alloc(n);

    if (hexlify(
            reinterpret_cast<uint8_t *>(buffer->data), n,
            reinterpret_cast<const uint8_t *>(str.data()), str.len()) == -1) {
        log_vverb("failed to hexlify '%s'", str.tos().c_str());
        return nullstr;
    }
    return Str(buffer, n);
}

Str base64_encode(const Str& str)
{
    str_buffer_t *buffer;
    size_t n;

    n = base64_encode_len(str.len());
    buffer = Str::alloc(n);

    if (base64_encode(
            reinterpret_cast<uint8_t *>(buffer->data), n,
            reinterpret_cast<const uint8_t *>(str.data()), str.len()) == -1) {
        log_vverb("failed to base64 encode '%s'", str.tos().c_str());
        return nullstr;
    }
    return Str(buffer, n);
}

Str base64_decode(const Str& str)
{
    str_buffer_t *buffer;
    size_t n;

    n = base64_decode_len(
            reinterpret_cast<const uint8_t *>(str.data()), str.len());
    buffer = Str::alloc(n);

    if (base64_decode(
            reinterpret_cast<uint8_t *>(buffer->data), n,
            reinterpret_cast<const uint8_t *>(str.data()), str.len()) == -1) {
        log_vverb("failed to base64 decode '%s'", str.tos().c_str());
        return nullstr;
    }
    return Str(buffer, n);
}

} // namespace
