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

using namespace ctornado;

#define BUF_SIZE    4096

void print_md5(uint8_t hash[])
{
    size_t i;

    for (i = 0; i < MD5_LEN; i++)
        printf("%02x", hash[i]);

    printf("\n");
}

int main()
{
    uint8_t hash[MD5_LEN], buf[BUF_SIZE];
    size_t i;

    for (i = 0; i < BUF_SIZE; i++)
        buf[i] = i;

    md5(hash, buf, BUF_SIZE);
    print_md5(hash);

    md5(hash, buf, 63);
    print_md5(hash);

    md5(hash, buf, 64);
    print_md5(hash);

    md5(hash, buf, 65);
    print_md5(hash);

    for (i = 0; i < BUF_SIZE; i++)
        buf[i] = i % 127;

    md5(hash, buf, BUF_SIZE);
    print_md5(hash);

    return 0;
}
