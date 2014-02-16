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

#ifndef __HTTPSERVER_H
#define __HTTPSERVER_H

#include "ctornado.h"

namespace ctornado {

//
// A non-blocking, single-threaded HTTP server.
//
// A server is defined by a request callback that takes an HTTPRequest
// instance as an argument and writes a valid HTTP response with
// HTTPRequest.write. HTTPRequest.finish finishes the request (but does
// not necessarily close the connection in the case of HTTP/1.1 keep-alive
// requests).
//
// HTTPServer is a very basic connection handler. Beyond parsing the
// HTTP request body and headers, the only HTTP semantics implemented
// in HTTPServer is HTTP/1.1 keep-alive connections. We do not, however,
// implement chunked encoding, so the request callback must provide a
// Content-Length header or implement chunked encoding for HTTP/1.1
// requests for the server to run correctly for HTTP/1.1 clients. If
// the request handler is unable to do this, you can provide the
// no_keep_alive argument to the HTTPServer constructor, which will
// ensure the connection is closed on every request no matter what HTTP
// version the client is using.
//
// If xheaders is True, we support the X-Real-Ip and X-Scheme headers,
// which override the remote IP and HTTP scheme for all requests.
// These headers are useful when running Tornado behind a reverse proxy or
// load balancer.
//
class HTTPServer : public TCPServer
{
public:
    HTTPServer(cb_req_t request_callback, IOLoop *ioloop=nullptr,
            bool no_keep_alive=false, bool xheaders=false)
        : TCPServer(ioloop)
        , request_callback_(request_callback)
        , no_keep_alive_(no_keep_alive)
        , xheaders_(xheaders) {}
    virtual ~HTTPServer() {}

    virtual void handle_stream(IOStream *stream, const Str& address);

    cb_req_t request_callback_;
    bool no_keep_alive_;
    bool xheaders_;
};

//
// Handles a connection to an HTTP client, executing HTTP requests.
//
// We parse HTTP headers and bodies, and execute the request callback
// until the HTTP conection is closed.
//
class HTTPConnection
{
public:
    HTTPConnection(IOStream *stream,
            const Str& address, cb_req_t request_callback,
            bool no_keep_alive=false, bool xheaders=false);
    ~HTTPConnection();

    static void free(HTTPConnection *connection);

    void close();

    //
    // Writes a chunk of output to the stream.
    //
    void write(const Str& chunk, cb_t callback=nullptr);

    //
    // Finishes the request.
    //
    void finish();

    IOStream *stream_;
    Str address_;
    cb_req_t request_callback_;
    bool no_keep_alive_;
    bool xheaders_;

private:
    HTTPRequest *request_;
    bool request_finished_;
    cb_stream_t header_callback_;
    cb_t write_callback_;

    void on_write_complete();
    void finish_request();
    void on_headers(const Str& data);
    void on_request_body(const Str& data);
};

//
// A single HTTP request.
//
// All attributes are type Str unless otherwise noted.
//
//  method
//      HTTP request method, e.g. "GET" or "POST"
//
//  uri
//      The requested uri.
//
//  path
//      The path portion of uri
//
//  query
//      The query portion of uri
//
//  version
//      HTTP version specified in request, e.g. "HTTP/1.1"
//
//  headers
//      HTTPHeader object for request headers.  Acts case-insensitively,
//      with methods for repeated headers.
//
//  body
//      Request body.
//
//  remote_ip
//      Client's IP address as a string.  If HTTPServer.xheaders is set,
//      will pass along the real IP address provided by a load balancer
//      in the X-Real-Ip header
//
//  protocol
//      The protocol used, either "http" or "https".  If HTTPServer.xheaders
//      is set, will pass along the protocol used by a load balancer if
//      reported via an X-Scheme header.
//
//  host
//      The requested hostname, usually taken from the Host header.
//
//  arguments
//      GET/POST arguments are available in the arguments property, which
//      maps arguments names to lists of values (to support multiple values
//      for individual names).
//
//  files
//      File uploads are available in the files property, which maps file
//      names to lists of HTTPFile.
//
//  connection
//      An HTTP request is attached to a single HTTP connection, which can
//      be accessed through the "connection" attribute. Since connections
//      are typically kept open in HTTP/1.1, multiple requests can be handled
//      sequentially on a single connection.
//
class HTTPRequest
{
public:
    HTTPRequest(HTTPConnection *connection, const Str& method, const Str& uri,
            const Str& version="HTTP/1.0", HTTPHeaders *headers=nullptr,
            const Str& remote_ip=nullstr, const Str& protocol=nullstr,
            const Str& host=nullstr, const Str& body=nullstr,
            FileMMap *files=nullptr);
    ~HTTPRequest();

    //
    // Returns True if this request supports HTTP/1.1 semantics
    //
    bool supports_http_1_1();

    //
    // Get Cookie object, keeps a map of Cookie.Morsel objects.
    //
    Cookie *get_cookies();

    //
    // Writes the given chunk to the response stream.
    //
    void write(const Str& chunk, cb_t callback=nullptr);

    //
    // Finishes this HTTP request on the open connection.
    //
    void finish();

    //
    // Reconstructs the full URL for this request.
    //
    Str full_url();

    //
    // Returns the amount of time it took for this request to execute.
    //
    int64_t request_time();

    HTTPConnection *connection_;
    Str method_;
    Str uri_;
    Str version_;
    HTTPHeaders *headers_;
    Str remote_ip_;
    Str protocol_;
    Str host_;
    Str body_;
    Str path_;
    Str query_;
    Query *arguments_;
    FileMMap *files_;

private:
    int64_t start_time_;
    int64_t finish_time_;
    Cookie *cookies_;
};

} // namespace

#endif // __HTTPSERVER_H

