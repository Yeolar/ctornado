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

void Buffer::merge_prefix(size_t size)
{
    Str chunk;
    str_buffer_t *prefix_buf;
    char *pos;
    size_t remaining;

    log_vverb("merge prefix %zu bytes (at most) of buffer", size);

    if (chunk_dq_.size() == 1 && chunk_dq_[0].len_ <= size)
        return;

    if (chunk_dq_.size() > 0 && size > 0) {
        if (chunk_dq_[0].len_ == size)
            return;

        if (chunk_dq_[0].len_ > size) {
            chunk = chunk_dq_[0];
            chunk_dq_.pop_front();
            chunk_dq_.push_front(chunk.substr(size, -1));
            chunk_dq_.push_front(chunk.substr(0, size));
            return;
        }

        size = min(size, size_);

        prefix_buf = Str::alloc(size);
        pos = prefix_buf->data;
        remaining = size;

        while (remaining > 0 /* && chunk_dq_.size() > 0 */) {
            chunk = chunk_dq_[0];
            chunk_dq_.pop_front();

            if (chunk.len_ > remaining) {
                chunk_dq_.push_front(chunk.substr(remaining, -1));

                memcpy(pos, chunk.data_, remaining);
                pos += remaining;
                remaining = 0;
            }
            else {
                memcpy(pos, chunk.data_, chunk.len_);
                pos += chunk.len_;
                remaining -= chunk.len_;
            }
        }
        chunk_dq_.push_front(Str(prefix_buf, size));
    }
    if (chunk_dq_.size() == 0)
        chunk_dq_.push_front("");
}

void Buffer::double_prefix()
{
    size_t n;

    if (chunk_dq_.size() < 2)
        return;

    n = max(chunk_dq_[0].len_ * 2, chunk_dq_[0].len_ + chunk_dq_[1].len_);
    merge_prefix(n);
}

void Buffer::remove_prefix(size_t size)
{
    Str chunk;
    size_t remaining;

    log_vverb("remove prefix %zu bytes (at most) from buffer", size);

    remaining = size;

    while (chunk_dq_.size() > 0 && remaining > 0) {
        chunk = chunk_dq_[0];
        chunk_dq_.pop_front();

        if (chunk.len_ > remaining) {
            chunk_dq_.push_front(chunk.substr(remaining, -1));
            size_ -= remaining;
            remaining = 0;
        }
        else {
            size_ -= chunk.len_;
            remaining -= chunk.len_;
        }
    }
}

void Buffer::push(const Str& chunk)
{
    log_vverb("push %zu bytes to buffer", chunk.len_);

    size_ += chunk.len_;
    chunk_dq_.push_back(chunk);
}

Str Buffer::pop()
{
    Str chunk;

    ASSERT(chunk_dq_.size() > 0);

    chunk = chunk_dq_[0];
    chunk_dq_.pop_front();
    size_ -= chunk.len_;

    log_vverb("pop %zu bytes from buffer", chunk.len_);

    return chunk;
}

Str Buffer::top()
{
    ASSERT(chunk_dq_.size() > 0);

    return chunk_dq_[0];
}

size_t Buffer::size()
{
    return size_;
}

void Buffer::clear()
{
    chunk_dq_.clear();
    size_ = 0;
}

} // namespace
