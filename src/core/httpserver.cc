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

void HTTPServer::handle_stream(IOStream *stream, const Str& address)
{
    new HTTPConnection(stream, address,
            request_callback_, no_keep_alive_, xheaders_);
}

HTTPConnection::HTTPConnection(IOStream *stream, const Str& address,
        cb_req_t request_callback, bool no_keep_alive, bool xheaders)
{
    log_verb("handle stream[%p] on connection[%p]", stream, this);

    stream_ = stream;
    address_ = address;
    request_callback_ = request_callback;
    no_keep_alive_ = no_keep_alive;
    xheaders_ = xheaders;
    request_ = nullptr;
    request_finished_ = false;
    header_callback_ = std::bind(&HTTPConnection::on_headers, this, _1);
    write_callback_ = nullptr;

    log_verb("connection[%p] handle HTTP request", this);

    stream_->read_until("\r\n\r\n", header_callback_);
}

HTTPConnection::~HTTPConnection()
{
    log_verb("free stream[%p]", stream_);
    delete stream_;
}

void HTTPConnection::free(HTTPConnection *connection)
{
    log_verb("free connection[%p]", connection);
    delete connection;
}

void HTTPConnection::close()
{
    log_verb("close connection[%p]", this);

    stream_->close();
    header_callback_ = nullptr;
    stream_->ioloop_->add_callback(std::bind(&HTTPConnection::free, this));
}

void HTTPConnection::write(const Str& chunk, cb_t callback)
{
    log_verb("connection[%p] write to stream", this);

    ASSERT(request_ != nullptr);

    if (!stream_->closed()) {
        write_callback_ = callback;
        stream_->write(chunk,
                std::bind(&HTTPConnection::on_write_complete, this));
    }
}

void HTTPConnection::finish()
{
    ASSERT(request_ != nullptr);

    request_finished_ = true;

    if (!stream_->writing()) {
        finish_request();
    }
}

void HTTPConnection::on_write_complete()
{
    cb_t callback;

    if (write_callback_ != nullptr) {
        callback = write_callback_;
        write_callback_ = nullptr;
        callback();
    }
    //
    // on_write_complete is enqueued on the IOLoop whenever the
    // IOStream's write buffer becomes empty, but it's possible for
    // another callback that runs on the IOLoop before it to
    // simultaneously write more data and finish the request.  If
    // there is still data in the IOStream, a future
    // on_write_complete will be responsible for calling
    // finish_request.
    //
    if (request_finished_ && !stream_->writing()) {
        finish_request();
    }
}

void HTTPConnection::finish_request()
{
    bool disconnect;

    if (no_keep_alive_) {
        disconnect = true;
    }
    else {
        Str connection_header = request_->headers_->get("Connection");
        if (connection_header.data_ != nullptr) {
            connection_header = connection_header.lower();
        }

        if (request_->supports_http_1_1()) {
            disconnect = connection_header.eq("close");
        }
        else if (request_->headers_->has("Content-Length") ||
                request_->method_.eq("HEAD") ||
                request_->method_.eq("GET")) {
            disconnect = !connection_header.eq("keep-alive");
        }
        else {
            disconnect = true;
        }
    }

    delete request_;
    request_ = nullptr;
    request_finished_ = false;

    if (disconnect) {
        log_verb("connection[%p] handle HTTP request finished", this);
        close();
        return;
    }
    log_verb("connection[%p] handle HTTP request finished, keep alive", this);

    stream_->read_until("\r\n\r\n", header_callback_);
}

void HTTPConnection::on_headers(const Str& data)
{
    int eol, content_length;
    int family;
    Str start_line, method, uri, version, remote_ip, content_length_str;
    HTTPHeaders *headers;

    log_verb("connection[%p] handle headers in HTTP request", this);

    eol = data.find("\r\n");
    start_line = data.substr(0, eol);
    auto strs = start_line.split(' ');

    if (strs.size() != 3) {
        log_info("Malformed HTTP request line from %.*s",
                address_.len_, address_.data_);
        close();
        return;
    }
    auto it = strs.begin();
    method = *it++;
    uri = *it++;
    version = *it++;

    if (!version.starts_with("HTTP/")) {
        log_info("Malformed HTTP version in HTTP request from %.*s",
                address_.len_, address_.data_);
        close();
        return;
    }
    headers = HTTPHeaders::parse(data.substr(eol, -1));

    // HTTPRequest wants an IP, not a full socket address
    family = stream_->socket_->family_;
    if (family == AF_INET || family == AF_INET6)
        remote_ip = address_;
    else
        remote_ip = "0.0.0.0";// Unix (or other) socket; fake the remote address

    request_ = new HTTPRequest(this, method, uri, version, headers, remote_ip);

    content_length_str = headers->get("Content-Length");

    if (!content_length_str.null()) {
        content_length = content_length_str.toi();

        if (content_length > stream_->max_buffer_size_) {
            log_info("Malformed HTTP request from %.*s:"
                    " Content-Length too long", address_.len_, address_.data_);
            close();
            return;
        }
        if (headers->get("Expect").eq("100-continue")) {
            stream_->write("HTTP/1.1 100 (Continue)\r\n\r\n");
        }
        stream_->read_bytes(content_length,
                std::bind(&HTTPConnection::on_request_body, this, _1));
    }
    else {
        request_callback_(request_);
    }
}

void HTTPConnection::on_request_body(const Str& data)
{
    request_->body_ = data;

    log_verb("connection[%p] handle body in HTTP request", this);

    if (request_->method_.eq("POST") ||
        request_->method_.eq("PATCH") ||
        request_->method_.eq("PUT")) {
        parse_body_arguments(request_->headers_->get("Content-Type", ""),
                data, request_->arguments_, request_->files_);
    }
    request_callback_(request_);
}

HTTPRequest::HTTPRequest(HTTPConnection *connection,
        const Str& method, const Str& uri, const Str& version,
        HTTPHeaders *headers, const Str& remote_ip, const Str& protocol,
        const Str& host, const Str& body, file_mmap_t *files)
{
    char *rip;

    connection_ = connection;
    method_ = method;
    uri_ = uri;
    version_ = version;
    body_ = body;
    headers_ = (headers != nullptr) ? headers : new HTTPHeaders();
    files_ = (files != nullptr) ? files : new file_mmap_t();
    host_ = (!host.null()) ? host : headers_->get("Host", "127.0.0.1");

    if (connection_ != nullptr && connection_->xheaders_) {
        // Squid uses X-Forward-For, others use X-Real-Ip
        remote_ip_ = headers_->get("X-Real-Ip",
                     headers_->get("X-Forwarded-For", remote_ip));

        rip = remote_ip_.str();
        if (!valid_ip(rip))
            remote_ip_ = remote_ip;
        free_w(rip);

        // AWS uses X-Forwarded-Proto
        protocol_ = headers_->get("X-Scheme",
                    headers_->get("X-Forwarded-Proto", protocol));

        if (!protocol_.eq("http") && !protocol_.eq("https"))
            protocol_ = "http";
    }
    else {
        remote_ip_ = remote_ip;
        protocol_ = (!protocol.null()) ? protocol : "http";
    }

    auto pair = uri_.split_pair('?');
    path_ = pair.first;
    query_ = pair.second;
    arguments_ = Query::parse(query_);

    start_time_ = msec_now();
    finish_time_ = 0;
    cookies_ = nullptr;
}

HTTPRequest::~HTTPRequest()
{
    delete headers_;
    delete arguments_;
    delete files_;
    if (cookies_)
        delete cookies_;
}

bool HTTPRequest::supports_http_1_1()
{
    return version_.eq("HTTP/1.1");
}

Cookie *HTTPRequest::get_cookies()
{
    if (cookies_ == nullptr) {
        cookies_ = new Cookie();
        if (headers_->has("Cookie")) {
            cookies_->load(headers_->get("Cookie"));
        }
    }
    return cookies_;
}

void HTTPRequest::write(const Str& chunk, cb_t callback)
{
    connection_->write(chunk, callback);
}

void HTTPRequest::finish()
{
    connection_->finish();
    finish_time_ = msec_now();
}

Str HTTPRequest::full_url()
{
    return Str::sprintf("%S://%S%S", &protocol_, &host_, &uri_);
}

int64_t HTTPRequest::request_time()
{
    if (finish_time_ == 0)
        return msec_now() - start_time_;
    else
        return finish_time_ - start_time_;
}

} // namespace
