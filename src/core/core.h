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

#ifndef __CORE_H
#define __CORE_H

//
// Request use of C99 limit macros.
//
// The ISO C99 standard specifies that in C++ implementations those macros
// should only be defined if explicitly requested.
//
#define __STDC_LIMIT_MACROS

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <zlib.h>
#include <pcre.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include <exception>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <functional>
#include <string>
#include <list>
#include <vector>
#include <deque>
#include <queue>
#include <map>

namespace ctornado {

//
// Length of 1 byte, 2 bytes, 4 bytes, 8 bytes and largest integral
// type (uintmax_t) in ascii, including the null terminator '\0'
//
// From stdint.h, we have:
// # define UINT8_MAX	(255)
// # define UINT16_MAX	(65535)
// # define UINT32_MAX	(4294967295U)
// # define UINT64_MAX	(__UINT64_C(18446744073709551615))
//
#define UINT8_MAXLEN        (3 + 1)
#define UINT16_MAXLEN       (5 + 1)
#define UINT32_MAXLEN       (10 + 1)
#define UINT64_MAXLEN       (20 + 1)
#define UINTMAX_MAXLEN      UINT64_MAXLEN

#define PTR_SIZE            8

#define nullstr             nullptr

using std::out_of_range;
using std::min;
using std::max;
using std::function;
using std::bind;
using std::string;
using std::list;
using std::vector;
using std::deque;
using std::priority_queue;
using std::map;
using std::multimap;
using std::pair;
using std::make_pair;

class Str;
struct StrLess;
class Socket;
class HTTPFile;
class HTTPRequest;

typedef pair<int, uint32_t> event_t;
typedef vector<event_t> event_list_t;
typedef list<Socket *> socket_list_t;
typedef pair<Str, Str> ss_t;
typedef vector<Str> s_vect_t;
typedef list<Str> s_list_t;
typedef list<ss_t> ss_list_t;
typedef map<Str, Str, StrLess> ss_map_t;
typedef multimap<Str, Str, StrLess> ss_mmap_t;
typedef multimap<Str, HTTPFile, StrLess> file_mmap_t;

typedef function<void (void)> cb_t;
typedef function<void (int, uint32_t)> cb_handler_t;
typedef function<void (const Str&)> cb_stream_t;
typedef function<void (HTTPRequest *)> cb_req_t;

} // namespace

#endif // __CORE_H

