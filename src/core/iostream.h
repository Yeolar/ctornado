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

#ifndef __IOSTREAM_H
#define __IOSTREAM_H

#include "ctornado.h"

namespace ctornado {

//
// A utility class to write to and read from a non-blocking socket.
//
// We support a non-blocking write() and a family of read_*() methods.
// All of the methods take callbacks (since writing and reading are
// non-blocking and asynchronous).
//
// The socket_ parameter may either be connected or unconnected.  For
// server operations the socket is the result of calling socket.accept().
// For client operations the socket is created with socket.socket(),
// and may either be connected before passing it to the IOStream or
// connected with IOStream.connect.
//
// When a stream is closed due to an error, the IOStream's error_
// attribute contains the error object.
//
class IOStream
{
public:
    IOStream(Socket *socket, IOLoop *ioloop=nullptr,
            size_t max_buffer_size=104857600, size_t read_chunk_size=4096);
    virtual ~IOStream() {}

    //
    // Connects the socket to a remote address without blocking.
    //
    // May only be called if the socket passed to the constructor was
    // not previously connected.  If callback is specified, it will be
    // called when the connection is completed.
    //
    // Note that it is safe to call IOStream.write while the
    // connection is pending, in which case the data will be written
    // as soon as the connection is ready.  Calling IOStream read
    // methods before the socket is connected also works.
    //
    void connect(const char *host, int port, cb_t callback=nullptr);

    //
    // Call callback when we read the given regex pattern.
    //
    void read_until_regex(const char *regex, cb_stream_t callback);

    //
    // Call callback when we read the given delimiter.
    //
    void read_until(const char *delimiter, cb_stream_t callback);

    //
    // Call callback when we read the given number of bytes.
    //
    // If a streaming_callback is given, it will be called with chunks
    // of data as they become available, and the argument to the final
    // callback will be empty.
    //
    void read_bytes(size_t num_bytes, cb_stream_t callback,
            cb_stream_t streaming_callback=nullptr);

    //
    // Reads all data from the socket until it is closed.
    //
    // If a streaming_callback is given, it will be called with chunks
    // of data as they become available, and the argument to the final
    // callback will be empty.
    //
    // Subject to max_buffer_size limit from IOStream constructor if
    // a streaming_callback is not used.
    //
    void read_until_close(cb_stream_t callback,
            cb_stream_t streaming_callback=nullptr);

    //
    // Write the given data to this stream.
    //
    // If callback is given, we call it when all of the buffered write
    // data has been successfully written to the stream. If there was
    // previously buffered write data and an old write callback, that
    // callback is simply overwritten with this new callback.
    //
    void write(const Str& data, cb_t callback=nullptr);

    //
    // Call the given callback when the stream is closed.
    //
    void set_close_callback(cb_t callback);

    //
    // Close this stream.
    //
    void close();

    //
    // Returns true if we are currently reading from the stream.
    //
    bool reading();

    //
    // Returns true if we are currently writing to the stream.
    //
    bool writing();

    //
    // Returns true if the stream has been closed.
    //
    bool closed();

    Socket *socket_;
    IOLoop *ioloop_;
    size_t max_buffer_size_;
    size_t read_chunk_size_;
    Error *error_;

private:
    void maybe_run_close_callback();

    void handle_events(int fd, uint32_t events);

    void callback_wrapper(cb_t callback);
    void run_callback(cb_t callback);
    void run_callback(cb_stream_t callback, const Str& data);

    void handle_read();

    void set_read_callback(cb_stream_t callback);

    //
    // Attempt to complete the current read operation from buffered data.
    //
    // If the read can be completed without blocking, schedules the
    // read callback on the next IOLoop iteration; otherwise starts
    // listening for reads on the socket.
    //
    void try_inline_read();

    //
    // Attempts to read from the socket.
    //
    // Returns the data read or nullstr if there is nothing to read.
    // May be overridden in subclasses.
    //
    virtual Str read_from_socket();

    //
    // Reads from the socket and appends the result to the read buffer.
    //
    // Returns the number of bytes read.  Returns 0 if there is nothing
    // to read (i.e. the read returns EWOULDBLOCK or equivalent).  On
    // error closes the socket and throws an exception.
    //
    int read_to_buffer();

    //
    // Attempts to complete the currently-pending read from the buffer.
    //
    // Returns True if the read was completed.
    //
    bool read_from_buffer();

    void handle_connect();
    void handle_write();

    Str consume(int loc);

    void check_closed();

    void maybe_add_error_listener();

    //
    // Adds state (IOLoop::{READ,WRITE} flags) to our event handler.
    //
    // Implementation notes: Reads and writes have a fast path and a
    // slow path.  The fast path reads synchronously from socket
    // buffers, while the slow path uses add_io_state to schedule
    // an IOLoop callback.  Note that in both cases, the callback is
    // run asynchronously with run_callback.
    //
    // To detect closed connections, we must have called
    // add_io_state at some point, but we want to delay this as
    // much as possible so we don't have to set an IOLoop::ERROR
    // listener that will be overwritten by the next slow-path
    // operation.  As long as there are callbacks scheduled for
    // fast-path ops, those callbacks may do more reads.
    // If a sequence of fast-path ops do not end in a slow-path op,
    // (e.g. for an asynchronous long-poll request), we must add
    // the error handler.  This is done in run_callback and write
    // (since the write callback is optional so we can have a
    // fast-path write with no run_callback)
    //
    void add_io_state(uint32_t state);

    Buffer read_buffer_;
    Buffer write_buffer_;
    bool write_buffer_frozen_;
    const char *read_delimiter_;
    Regex *read_regex_;
    size_t read_bytes_;
    bool read_until_close_;
    cb_stream_t read_callback_;
    cb_stream_t streaming_callback_;
    cb_t write_callback_;
    cb_t close_callback_;
    cb_t connect_callback_;
    bool connecting_;
    uint32_t state_;
    int pending_callbacks_;
};

} // namespace

#endif // __IOSTREAM_H

