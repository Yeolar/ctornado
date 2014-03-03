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

#ifndef __HASH_H
#define __HASH_H

#include "ctornado.h"

namespace ctornado {

typedef MD5_CTX md5_t;

#define MD5_BLOCK_SIZE      64
#define MD5_LEN             16

#define md5_init            MD5_Init
#define md5_update          MD5_Update
#define md5_final           MD5_Final

inline void md5(uint8_t *result, const uint8_t *key, size_t len)
{
    md5_t ctx;

    md5_init(&ctx);
    md5_update(&ctx, key, len);
    md5_final(result, &ctx);
}

typedef SHA1_CTX sha1_t;

#define SHA1_BLOCK_SIZE     64
#define SHA1_LEN            20

#define sha1_init           SHA1Init
#define sha1_update         SHA1Update
#define sha1_final          SHA1Final

inline void sha1(uint8_t *result, const uint8_t *key, size_t len)
{
    sha1_t ctx;

    sha1_init(&ctx);
    sha1_update(&ctx, key, len);
    sha1_final(result, &ctx);
}

Str create_signature(const Str& secret, const StrList& parts);

} // namespace

#endif // __HASH_H

