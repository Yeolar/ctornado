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

#ifndef __BUFFER_H
#define __BUFFER_H

#include "ctornado.h"

namespace ctornado {

class Buffer
{
public:
    Buffer() : size_(0) {}
    ~Buffer() {}

    void merge(Buffer *buffer);

    void merge_prefix(size_t size);
    void double_prefix();
    void remove_prefix(size_t size);

    void push(const Str& chunk);
    Str pop();
    Str top();
    size_t size();
    void clear();

private:
    deque<Str> chunk_dq_;
    size_t size_;
};

class BufferIO
{
public:
    BufferIO() {}
    virtual ~BufferIO() {}

    Buffer *get_buffer();
    Str get_value();
    void clear();

protected:
    Buffer buffer_;
};

} // namespace

#endif // __BUFFER_H

