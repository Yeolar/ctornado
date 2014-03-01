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

#ifndef __GZIP_H
#define __GZIP_H

#include "ctornado.h"

//
// A wrapper of gzip compression, see /usr/include/zlib.h for more detail.
//

namespace ctornado {

int gz_compress_init(z_stream *stream, int compress_level);
int gz_compress(z_stream *stream, Bytef *out, uInt *out_len);
int gz_compress_flush(z_stream *stream, Bytef *out, uInt *out_len,
        int flush_mode);

int gz_decompress_init(z_stream *stream);
int gz_decompress(z_stream *stream, Bytef *out, uInt *out_len);
int gz_decompress_flush(z_stream *stream, Bytef *out, uInt *out_len);

class GZipCompressor : public BufferIO
{
public:
    GZipCompressor(int compress_level=Z_BEST_COMPRESSION);
    virtual ~GZipCompressor();

    void compress(const Str& data);
    void flush(int flush_mode=Z_SYNC_FLUSH);
    void close();

private:
    uint32_t crc_;
    size_t size_;
    z_stream stream_;
    str_buffer_t *tmp_buf_;
    uInt tmp_len_;
    int err_;

    void write_gzip_header();
};

class GZipDecompressor : public BufferIO
{
public:
    GZipDecompressor();
    virtual ~GZipDecompressor() {}

    void decompress(const Str& data);
    void flush();
    void close();

private:
    z_stream stream_;
    int err_;
};

} // namespace

#endif // __GZIP_H

