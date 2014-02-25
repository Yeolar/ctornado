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

#ifndef __BINASCII_H
#define __BINASCII_H

#include "ctornado.h"

namespace ctornado {

#ifdef __cplusplus
extern "C" {
#endif

ssize_t hexlify(uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len);

size_t base64_encode_len(size_t len);
ssize_t base64_encode(uint8_t *out, size_t out_len,
        const uint8_t *in, size_t in_len);

size_t base64_decode_len(const uint8_t *s, size_t len);
ssize_t base64_decode(uint8_t *out, size_t out_len,
        const uint8_t *in, size_t in_len);

#ifdef __cplusplus
} // extern "C"
#endif

//
// Return the hexadecimal representation of the binary data.
//
// Every byte of data is converted into the corresponding 2-digit
// hex representation. The resulting string is therefore twice
// as long as the length of data.
//
Str hexlify(const Str& str);

//
// Encode string str using the standard Base64 alphabet.
//
Str base64_encode(const Str& str);

//
// Decode string str using the standard Base64 alphabet.
//
Str base64_decode(const Str& str);

} // namespace

#endif // __BINASCII_H

