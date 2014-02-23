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

using namespace std::placeholders;

IOStream::IOStream(Socket *socket, IOLoop *ioloop,
        size_t max_buffer_size, size_t read_chunk_size)
{
    socket_ = socket;
    socket_->set_nonblocking();
    ioloop_ = ioloop != nullptr ? ioloop : IOLoop::instance();
    max_buffer_size_ = max_buffer_size;
    read_chunk_size_ = read_chunk_size;
    error_ = nullptr;
    write_buffer_frozen_ = false;
    read_delimiter_ = nullptr;
    read_regex_ = nullptr;
    read_bytes_ = 0;
    read_until_close_ = false;
    read_callback_ = nullptr;
    streaming_callback_ = nullptr;
    write_callback_ = nullptr;
    close_callback_ = nullptr;
    connect_callback_ = nullptr;
    connecting_ = false;
    state_ = 0;
    pending_callbacks_ = 0;
}

void IOStream::connect(const char *host, int port, cb_t callback)
{
    log_verb("connect to %s:%d", host, port);

    connecting_ = true;

    try {
        socket_->connect(host, port);
    }
    catch (SocketError& e) {
        //
        // In non-blocking mode we expect connect() to throw an
        // exception with EINPROGRESS or EWOULDBLOCK.
        //
        // On freebsd, other errors such as ECONNREFUSED may be
        // returned immediately when attempting to connect to
        // localhost, so handle them the same way as an error
        // reported later in _handle_connect.
        //
        if (e.no() != EINPROGRESS && e.no() != EWOULDBLOCK) {
            log_warn("connect error on fd(%d): %s", socket_->fd_, e.what());
            close();
            return;
        }
    }
    connect_callback_ = callback;
    add_io_state(IOLoop::WRITE);
}

void IOStream::read_until_regex(const char *regex, cb_stream_t callback)
{
#ifdef DEBUG_LOG
    log_verb("read until regex '%s'", Str(regex).escape().tos().c_str());
#endif

    set_read_callback(callback);
    if (read_regex_ != nullptr)
        delete read_regex_;
    read_regex_ = Regex::compile(regex);
    try_inline_read();
}

void IOStream::read_until(const char *delimiter, cb_stream_t callback)
{
#ifdef DEBUG_LOG
    log_verb("read until '%s'", Str(delimiter).escape().tos().c_str());
#endif

    set_read_callback(callback);
    read_delimiter_ = delimiter;
    try_inline_read();
}

void IOStream::read_bytes(size_t num_bytes, cb_stream_t callback,
        cb_stream_t streaming_callback)
{
    log_verb("read %zu bytes", num_bytes);

    set_read_callback(callback);
    read_bytes_ = num_bytes;
    streaming_callback_ = streaming_callback;
    try_inline_read();
}

void IOStream::read_until_close(cb_stream_t callback,
        cb_stream_t streaming_callback)
{
    log_verb("read until close");

    set_read_callback(callback);
    streaming_callback_ = streaming_callback;

    if (closed()) {
        if (streaming_callback_ != nullptr) {
            run_callback(streaming_callback_, consume(read_buffer_.size()));
        }
        run_callback(read_callback_, consume(read_buffer_.size()));
        streaming_callback_ = nullptr;
        read_callback_ = nullptr;
        return;
    }
    read_until_close_ = true;
    add_io_state(IOLoop::READ);
}

void IOStream::write(const Str& data, cb_t callback)
{
    check_closed();

    log_verb("write %zu bytes to buffer", data.len_);

    if (data.len_ > 0) {
        write_buffer_.push(data);
    }
    write_callback_ = callback;

    if (!connecting_) {
        handle_write();
        if (write_buffer_.size()) {
            add_io_state(IOLoop::WRITE);
        }
        maybe_add_error_listener();
    }
}

void IOStream::set_close_callback(cb_t callback)
{
    close_callback_ = callback;
}

void IOStream::close()
{
    cb_stream_t callback;

    log_verb("close stream[%p]", this);

    if (socket_ != nullptr) {
        if (errno) {
            if (error_ != nullptr)
                delete error_;
            error_ = new Error(errno);
        }
        if (read_until_close_) {
            callback = read_callback_;
            read_callback_ = nullptr;
            read_until_close_ = false;
            run_callback(callback, consume(read_buffer_.size()));
        }
        if (state_ != 0) {
            ioloop_->remove_handler(socket_->fd_);
            state_ = 0;
        }
        socket_->close();
        delete socket_;
        socket_ = nullptr;
    }
    if (read_regex_ != nullptr) {
        delete read_regex_;
        read_regex_ = nullptr;
    }
    maybe_run_close_callback();
}

void IOStream::maybe_run_close_callback()
{
    cb_t cb;

    if (socket_ == nullptr && close_callback_ != nullptr &&
            pending_callbacks_ == 0) {
        //
        // if there are pending callbacks, don't run the close callback
        // until they're done (see maybe_add_error_listener)
        //
        cb = close_callback_;
        close_callback_ = nullptr;
        run_callback(cb);
    }
}

bool IOStream::reading()
{
    return read_callback_ != nullptr;
}

bool IOStream::writing()
{
    return write_buffer_.size() > 0;
}

bool IOStream::closed()
{
    return socket_ == nullptr;
}

void IOStream::handle_events(int fd, uint32_t events)
{
    uint32_t state;

    if (socket_ == nullptr) {
        log_warn("got events for closed stream on fd(%d)", fd);
        return;
    }
    try {
        if (events & IOLoop::READ) {
            handle_read();
        }
        if (socket_ == nullptr) {
            return;
        }
        if (events & IOLoop::WRITE) {
            if (connecting_) {
                handle_connect();
            }
            handle_write();
        }
        if (socket_ == nullptr) {
            return;
        }
        if (events & IOLoop::ERROR) {
            if (error_ != nullptr)
                delete error_;
            error_ = new SocketError(socket_->get_soerror());
            //
            // We may have queued up a user callback in handle_read or
            // handle_write, so don't close the IOStream until those
            // callbacks have had a chance to run.
            //
            ioloop_->add_callback(bind(&IOStream::close, this));
            return;
        }

        state = IOLoop::ERROR;

        if (reading()) {
            state |= IOLoop::READ;
        }
        if (writing()) {
            state |= IOLoop::WRITE;
        }
        if (state == IOLoop::ERROR) {
            state |= IOLoop::READ;
        }
        if (state != state_) {
            ASSERT(state_ != 0);
            state_ = state;
            ioloop_->update_handler(socket_->fd_, state_);
        }
    }
    catch (Error& e) {
        log_error("uncaught exception, closing connection: %s", e.what());
        close();
        throw;
    }
}

void IOStream::callback_wrapper(cb_t callback)
{
    pending_callbacks_ -= 1;
    try {
        callback();
    }
    catch (Error& e) {
        log_error("uncaught exception, closing connection: %s", e.what());;
        // Close the socket on an uncaught exception from a user callback
        close();
        // Re-throw the exception so that IOLoop.handle_callback_exception
        // can see it and log the error
        throw;
    }
    maybe_add_error_listener();
}

void IOStream::run_callback(cb_t callback)
{
    //
    // We schedule callbacks to be run on the next IOLoop iteration
    // rather than running them directly for several reasons:
    //
    // * Prevents unbounded stack growth when a callback calls an
    //   IOLoop operation that immediately runs another callback
    // * Provides a predictable execution context for e.g.
    //   non-reentrant mutexes
    // * Ensures that the try/except in wrapper() is run outside
    //   of the application's StackContexts
    //
    pending_callbacks_ += 1;
    ioloop_->add_callback(bind(&IOStream::callback_wrapper, this, callback));
}

void IOStream::run_callback(cb_stream_t callback, const Str& data)
{
    //
    // The same as above.
    //
    pending_callbacks_ += 1;
    cb_t cb = bind(callback, data);
    ioloop_->add_callback(bind(&IOStream::callback_wrapper, this, cb));
}

void IOStream::handle_read()
{
    try {
        //
        // Pretend to have a pending callback so that an EOF in
        // read_to_buffer doesn't trigger an immediate close
        // callback.  At the end of this method we'll either
        // estabilsh a real pending callback via
        // read_from_buffer or run the close callback.
        //
        pending_callbacks_ += 1;

        while (true) {
            // Read from the socket until we get EWOULDBLOCK or equivalent.
            if (read_to_buffer() == 0)
                break;
        }
    }
    catch (Error& e) {
        pending_callbacks_ -= 1;

        log_warn("error on read: %s", e.what());
        close();
        return;
    }
    pending_callbacks_ -= 1;

    if (read_from_buffer())
        return;
    else
        maybe_run_close_callback();
}

void IOStream::set_read_callback(cb_stream_t callback)
{
    ASSERT(read_callback_ == nullptr);
    read_callback_ = callback;
}

void IOStream::try_inline_read()
{
    // See if we've already got the data from a previous read
    if (read_from_buffer())
        return;

    check_closed();

    try {
        pending_callbacks_ += 1;

        while (true) {
            if (read_to_buffer() == 0) {
                break;
            }
            check_closed();
        }
    }
    catch (Error& e) {
        pending_callbacks_ -= 1;
        throw;
    }
    pending_callbacks_ -= 1;

    if (read_from_buffer())
        return;

    maybe_add_error_listener();
}

Str IOStream::read_from_socket()
{
    str_buffer_t *chunk;
    int n;

    chunk = Str::alloc(read_chunk_size_);

    try {
        n = socket_->recv(chunk->data, read_chunk_size_);
    }
    catch (SocketError& e) {
        Str(chunk, 0);      // to be freed

        if (e.no() == EWOULDBLOCK || e.no() == EAGAIN)
            return nullstr;
        else
            throw;
    }
    if (n == 0) {
        Str(chunk, 0);      // to be freed
        close();

        return nullstr;
    }
    return Str(chunk, n);
}

int IOStream::read_to_buffer()
{
    Str chunk;

    try {
        chunk = read_from_socket();
    }
    catch (SocketError& e) {
        log_warn("read error on %d: %s", socket_->fd_, e.what());
        close();
        throw;
    }

    if (chunk.len_ == 0) {
        return 0;
    }

    log_verb("read %zu bytes data (socket -> buffer)", chunk.len_);
    read_buffer_.push(chunk);

    if (read_buffer_.size() >= max_buffer_size_) {
        log_error("reached maximum read buffer size");
        close();
        throw IOError("Reached maximum read buffer size");
    }
    return chunk.len_;
}

bool IOStream::read_from_buffer()
{
    Str str;
    size_t bytes_to_consume, num_bytes;
    int pos, delimiter_len;
    RegexMatch *m;
    cb_stream_t callback;

    log_verb("consume data from buffer");

    if (streaming_callback_ != nullptr && read_buffer_.size() > 0) {
        bytes_to_consume = read_buffer_.size();

        if (read_bytes_ != 0) {
            bytes_to_consume = min(read_bytes_, bytes_to_consume);
            read_bytes_ -= bytes_to_consume;
        }
        run_callback(streaming_callback_, consume(bytes_to_consume));
    }
    if (read_bytes_ != 0 && read_buffer_.size() >= read_bytes_) {
        num_bytes = read_bytes_;
        callback = read_callback_;

        read_callback_ = nullptr;
        streaming_callback_ = nullptr;
        read_bytes_ = 0;

        run_callback(callback, consume(num_bytes));
        return true;
    }
    else if (read_delimiter_ != nullptr) {
        //
        // Multi-byte delimiters (e.g. '\r\n') may straddle two
        // chunks in the read buffer, so we can't easily find them
        // without collapsing the buffer.  However, since protocols
        // using delimited reads (as opposed to reads of a known
        // length) tend to be "line" oriented, the delimiter is likely
        // to be in the first few chunks.  Merge the buffer gradually
        // since large merges are relatively expensive and get undone in
        // consume().
        //
        if (read_buffer_.size() > 0) {
            while (true) {
                str = read_buffer_.top();
                pos = str.find(read_delimiter_);

                if (pos != -1) {
                    callback = read_callback_;
                    delimiter_len = strlen(read_delimiter_);

                    read_callback_ = nullptr;
                    streaming_callback_ = nullptr;
                    read_delimiter_ = nullptr;

                    run_callback(callback, consume(pos + delimiter_len));
                    return true;
                }
                if (read_buffer_.size() == 1) {
                    break;
                }
                read_buffer_.double_prefix();
            }
        }
    }
    else if (read_regex_ != nullptr) {
        if (read_buffer_.size() > 0) {
            m = nullptr;

            while (true) {
                if (m != nullptr)
                    delete m;

                str = read_buffer_.top();
                try {
                    m = read_regex_->exec(str, 1);
                }
                catch (Error& e) {
                    log_verb("IOStream read_regex exec error: %s", e.what());
                    break;
                }

                if (!m->empty()) {
                    callback = read_callback_;

                    read_callback_ = nullptr;
                    streaming_callback_ = nullptr;
                    delete read_regex_;
                    read_regex_ = nullptr;

                    run_callback(callback, consume(m->get(0).end));
                    delete m;
                    return true;
                }
                if (read_buffer_.size() == 1) {
                    break;
                }
                read_buffer_.double_prefix();
            }
            if (m != nullptr)
                delete m;
        }
    }
    return false;
}

void IOStream::handle_connect()
{
    cb_t callback;
    int e;

    log_verb("handle connect");

    e = socket_->get_soerror();
    if (e) {
        if (error_ != nullptr)
            delete error_;
        error_ = new SocketError(e);
        //
        // IOLoop implementations may vary: some of them return
        // an error state before the socket becomes writable, so
        // in that case a connection failure would be handled by the
        // error path in handle_events instead of here.
        //
        log_warn("connect error on fd(%d): %s", socket_->fd_, strerror(e));
        close();
    }
    if (connect_callback_ != nullptr) {
        callback = connect_callback_;
        connect_callback_ = nullptr;
        run_callback(callback);
    }
    connecting_ = false;
}

void IOStream::handle_write()
{
    Str chunk;
    int num_bytes;
    cb_t callback;

    log_verb("write data (buffer -> socket)");

    while (write_buffer_.size() > 0) {
        if (!write_buffer_frozen_) {
            write_buffer_.merge_prefix(1048576);
        }
        chunk = write_buffer_.top();

        try {
            num_bytes = socket_->send(chunk.data_, chunk.len_);
        }
        catch (SocketError& e) {
            if (e.no() == EWOULDBLOCK || e.no() == EAGAIN) {
                write_buffer_frozen_ = true;
                break;
            }
            else {
                log_warn("write error on fd(%d): %s", socket_->fd_, e.what());
                close();
                return;
            }
        }
        if (num_bytes == 0) {
            write_buffer_frozen_ = true;
            break;
        }
        write_buffer_frozen_ = false;
        write_buffer_.remove_prefix(num_bytes);
    }
    if (write_buffer_.size() == 0 && write_callback_ != nullptr) {
        callback = write_callback_;
        write_callback_ = nullptr;
        run_callback(callback);
    }
}

Str IOStream::consume(int loc)
{
    if (loc == 0)
        return nullstr;

    read_buffer_.merge_prefix(loc);

    return read_buffer_.pop();
}

void IOStream::check_closed()
{
    if (socket_ == nullptr)
        throw IOError("Stream is closed");
}

void IOStream::maybe_add_error_listener()
{
    if (state_ == 0 && pending_callbacks_ == 0) {
        if (socket_ == nullptr)
            maybe_run_close_callback();
        else
            add_io_state(IOLoop::READ);
    }
}

void IOStream::add_io_state(uint32_t state)
{
    if (socket_ == nullptr)
        // connection has been closed, so there can be no future events
        return;

    log_verb("add IO state(%s) to fd(%d)", strevent(state), socket_->fd_);

    if (state_ == 0) {
        state_ = IOLoop::ERROR | state;
        ioloop_->add_handler(socket_->fd_,
                bind(&IOStream::handle_events, this, _1, _2), state_);
    }
    else if ((state_ & state) == 0) {
        state_ |= state;
        ioloop_->update_handler(socket_->fd_, state_);
    }
}

} // namespace
