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

#define BLOCK_SIZE      STR_BUF_1K

int gz_compress_init(z_stream *stream, int compress_level)
{
    int err;

    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;
    //
    // Set windowBits == -MAX_WBITS to generate raw deflate data with
    // no zlib header or trailer. Add 16 (MAX_WBITS + 16) can write
    // a simple gzip header and trailer around the compressed data.
    //
    err = deflateInit2(stream,
            compress_level,         // valid: 0-9
            Z_DEFLATED,
            -MAX_WBITS,             // raw deflate
            8,                      // mem level
            Z_DEFAULT_STRATEGY);

    if (err != Z_OK) {
        log_vverb("gzip compress init error: %s", zError(err));
    }
    return err;
}

int gz_compress(z_stream *stream, Bytef *out, uInt *out_len)
{
    uLong start_total_out;
    int err;

    start_total_out = stream->total_out;
    stream->next_out = out;
    stream->avail_out = *out_len;

    err = deflate(stream, Z_NO_FLUSH);
    *out_len = stream->total_out - start_total_out;

    if (err != Z_OK && err != Z_BUF_ERROR) {
        log_vverb("gzip compress error: %s", zError(err));
    }
    return err;
}

int gz_compress_flush(z_stream *stream, Bytef *out, uInt *out_len,
        int flush_mode)
{
    uLong start_total_out;
    int err;

    if (flush_mode == Z_NO_FLUSH)
        return Z_OK;

    start_total_out = stream->total_out;
    stream->avail_in = 0;
    stream->next_out = out;
    stream->avail_out = *out_len;

    err = deflate(stream, flush_mode);
    *out_len = stream->total_out - start_total_out;

    if (err == Z_OK && stream->avail_out == 0)
        return Z_OK;

    if (err == Z_STREAM_END && flush_mode == Z_FINISH) {
        err = deflateEnd(stream);
        if (err != Z_OK) {
            log_vverb("gzip compress end error: %s", zError(err));
        }
        else
            err = Z_STREAM_END;
    }
    else if (err != Z_OK && err != Z_BUF_ERROR) {
        log_vverb("gzip compress flush error: %s", zError(err));
    }
    return err;
}

int gz_decompress_init(z_stream *stream)
{
    int err;

    stream->zalloc = Z_NULL;
    stream->zfree = Z_NULL;
    stream->opaque = Z_NULL;
    stream->next_in = Z_NULL;
    stream->avail_in = 0;
    //
    // Set windowBits == 16 + MAX_WBITS to process gzip
    //
    err = inflateInit2(stream, 16 + MAX_WBITS);

    if (err != Z_OK) {
        log_vverb("gzip decompress init error: %s", zError(err));
    }
    return err;
}

int gz_decompress(z_stream *stream, Bytef *out, uInt *out_len)
{
    uLong start_total_out;
    int err;

    start_total_out = stream->total_out;
    stream->next_out = out;
    stream->avail_out = *out_len;

    err = inflate(stream, Z_SYNC_FLUSH);
    *out_len = stream->total_out - start_total_out;

    if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR) {
        log_vverb("gzip decompress error: %s", zError(err));
    }
    return err;
}

int gz_decompress_flush(z_stream *stream, Bytef *out, uInt *out_len)
{
    uLong start_total_out;
    int err;

    start_total_out = stream->total_out;
    stream->next_out = out;
    stream->avail_out = *out_len;

    err = inflate(stream, Z_FINISH);
    *out_len = stream->total_out - start_total_out;

    if ((err == Z_OK || err == Z_BUF_ERROR) && stream->avail_out == 0)
        return err;

    if (err == Z_STREAM_END) {
        err = inflateEnd(stream);
        if (err != Z_OK) {
            log_vverb("gzip decompress end error: %s", zError(err));
        }
        else
            err = Z_STREAM_END;
    }
    else if (err != Z_OK && err != Z_BUF_ERROR) {
        log_vverb("gzip decompress flush error: %s", zError(err));
    }
    return err;
}

GZipCompressor::GZipCompressor(int compress_level)
{
    crc_ = crc32(0, Z_NULL, 0) & 0xffffffff;
    size_ = 0;
    tmp_buf_ = Str::alloc(BLOCK_SIZE);
    tmp_len_ = 0;

    err_ = gz_compress_init(&stream_, compress_level);
    if (err_ != Z_OK)
        throw IOError(err_, zError(err_));

    write_gzip_header();
}

GZipCompressor::~GZipCompressor()
{
    if (tmp_buf_ != nullptr)
        FREE(tmp_buf_);
}

void GZipCompressor::write_gzip_header()
{
    char *pos;

    pos = tmp_buf_->data + tmp_len_;

    memcpy(pos, "\x1f\x8b", 2);             // magic header
    pos += 2;
    *pos++ = 0x08;                          // compression method
    *pos++ = 0x00;                          // flag
    memsetu(pos, sec_now(), 4);             // timestamp
    pos += 4;
    *pos++ = 0x02;
    *pos++ = 0xff;

    tmp_len_ += 10;
}

void GZipCompressor::compress(const Str& data)
{
    Bytef *in, *out;
    uInt out_len;

    in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));

    if (data.len() > 0) {
        size_ += data.len();
        crc_ = crc32(crc_, in, data.len()) & 0xffffffff;

        stream_.next_in = in;
        stream_.avail_in = data.len();

        do {
            if (tmp_len_ == BLOCK_SIZE) {
                if (tmp_buf_ != nullptr) {
                    buffer_.push(Str(tmp_buf_, tmp_len_));
                }
                tmp_buf_ = Str::alloc(BLOCK_SIZE);
                tmp_len_ = 0;
            }
            out = reinterpret_cast<Bytef *>(tmp_buf_->data + tmp_len_);
            out_len = BLOCK_SIZE - tmp_len_;

            err_ = gz_compress(&stream_, out, &out_len);
            tmp_len_ += out_len;

            if (err_ != Z_OK)   // break on error
                break;

        } while (stream_.avail_out == 0);

        if (err_ != Z_OK && err_ != Z_BUF_ERROR)
            throw IOError(err_, zError(err_));
    }
}

void GZipCompressor::flush(int flush_mode)
{
    Bytef *out;
    uInt out_len;

    do {
        if (tmp_len_ == BLOCK_SIZE) {
            if (tmp_buf_ != nullptr) {
                buffer_.push(Str(tmp_buf_, tmp_len_));
            }
            tmp_buf_ = Str::alloc(BLOCK_SIZE);
            tmp_len_ = 0;
        }
        out = reinterpret_cast<Bytef *>(tmp_buf_->data + tmp_len_);
        out_len = BLOCK_SIZE - tmp_len_;

        err_ = gz_compress_flush(&stream_, out, &out_len, flush_mode);
        tmp_len_ += out_len;

        if (err_ != Z_OK)   // break at end or on error
            break;

    } while (stream_.avail_out == 0);

    if (tmp_len_ != 0) {
        buffer_.push(Str(tmp_buf_, tmp_len_));
        tmp_buf_ = nullptr;
        tmp_len_ = BLOCK_SIZE;      // fake full block
    }

    if (err_ != Z_OK && err_ != Z_STREAM_END && err_ != Z_BUF_ERROR)
        throw IOError(err_, zError(err_));
}

void GZipCompressor::close()
{
    str_buffer_t *str_buf;

    flush(Z_FINISH);

    str_buf = Str::alloc(8);
    memsetu(str_buf->data, crc_, 4);
    memsetu(str_buf->data + 4, size_, 4);

    buffer_.push(Str(str_buf, 8));
}

GZipDecompressor::GZipDecompressor()
{
    err_ = gz_decompress_init(&stream_);
    if (err_ != Z_OK)
        throw IOError(err_, zError(err_));
}

void GZipDecompressor::decompress(const Str& data)
{
    str_buffer_t *str_buf;
    Bytef *in, *out;
    uInt out_len;

    in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));

    if (data.len() > 0) {
        stream_.next_in = in;
        stream_.avail_in = data.len();

        do {
            str_buf = Str::alloc(BLOCK_SIZE);
            out = reinterpret_cast<Bytef *>(str_buf->data);
            out_len = BLOCK_SIZE;

            err_ = gz_decompress(&stream_, out, &out_len);
            buffer_.push(Str(str_buf, out_len));

            if (err_ != Z_OK)   // break at end or on error
                break;

        } while (stream_.avail_out == 0);

        if (err_ != Z_OK && err_ != Z_STREAM_END && err_ != Z_BUF_ERROR)
            throw IOError(err_, zError(err_));
    }
}

void GZipDecompressor::flush()
{
    str_buffer_t *str_buf;
    Bytef *out;
    uInt out_len;

    do {
        str_buf = Str::alloc(BLOCK_SIZE);
        out = reinterpret_cast<Bytef *>(str_buf->data);
        out_len = BLOCK_SIZE;

        err_ = gz_decompress_flush(&stream_, out, &out_len);
        buffer_.push(Str(str_buf, out_len));

        if (err_ != Z_OK && err_ != Z_BUF_ERROR)    // break at end or on error
            break;

    } while (stream_.avail_out == 0);

    if (err_ != Z_OK && err_ != Z_STREAM_END && err_ != Z_BUF_ERROR)
        throw IOError(err_, zError(err_));
}

void GZipDecompressor::close() {}

} // namespace
