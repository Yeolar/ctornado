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

#ifndef __LOG_H
#define __LOG_H

#include "ctornado.h"

namespace ctornado {

//
// Log rules:
//
//  - 'string'          show string
//  - name(value)       show value of name
//  - func(value...)    exec function with value...
//  - name[address]     show memory address of name
//  - @[file:line]      at line in file
//  - (key, value)      show key-value pair
//

class Logger
{
public:
    Logger();
    ~Logger();

    static int initialize(int level, char *filename=nullptr);
    static void set_level(int level);
    static bool loggable(int level);
    static void _log(int level, const char *file, int line, int panic,
            const char *fmt, ...);
    static void _log_stderr(const char *fmt, ...);
    static void _log_hexdump(const char *file, int line,
            char *data, int datalen, const char *fmt, ...);

    static const int PANIC  = 0;        // system in unusable
    static const int CRIT   = 1;        // critical conditions
    static const int ERROR  = 2;        // error conditions
    static const int WARN   = 3;        // warning conditions
    static const int INFO   = 4;        // informational
    static const int DEBUG  = 5;        // debug messages
    static const int VERB   = 6;        // verbose all messages of core
    static const int VVERB  = 7;        // verbose all messages of core, lib

    static const size_t LOG_MAX_LEN = 256;  // max length of log message

private:
    char *name_;        // log file name
    int  level_;        // log level
    int  fd_;           // log file descriptor
    int  num_error_;    // # log error
};

//
// log_stderr   - log to stderr
// loga         - log always
// loga_hexdump - log hexdump always
// log          - interface of Logger::_log
// log_critical - critical log messages
// log_error    - error log messages
// log_warn     - warning log messages
// log_info     - informational messages
// log_panic    - log messages followed by a panic
// ...
// log_debug    - debug log messages
// log_verb     - verbose all log messages of core
// log_vverb    - verbose all log messages of core and lib
// log_hexdump  - hexadump -C of a log buffer
//
#ifdef DEBUG_LOG

#define log_debug(...) do {                                             \
    if (Logger::loggable(Logger::DEBUG)) {                              \
        Logger::_log(Logger::DEBUG, __FILE__, __LINE__, 0, __VA_ARGS__);\
    }                                                                   \
} while (0)

#define log_verb(...) do {                                              \
    if (Logger::loggable(Logger::VERB)) {                               \
        Logger::_log(Logger::VERB, __FILE__, __LINE__, 0, __VA_ARGS__); \
    }                                                                   \
} while (0)

#define log_vverb(...) do {                                             \
    if (Logger::loggable(Logger::VVERB)) {                              \
        Logger::_log(Logger::VVERB, __FILE__, __LINE__, 0, __VA_ARGS__);\
    }                                                                   \
} while (0)

#define log_hexdump(_level, _data, _datalen, ...) do {                  \
    if (Logger::loggable(_level)) {                                     \
        Logger::_log(_level, __FILE__, __LINE__, 0, __VA_ARGS__);       \
        Logger::_log_hexdump(__FILE__, __LINE__,                        \
                static_cast<char *>(_data), static_cast<int>(_datalen), \
                __VA_ARGS__);                                           \
    }                                                                   \
} while (0)

#else

#define log_debug(...)
#define log_verb(...)
#define log_vverb(...)
#define log_hexdump(_level, _data, _datalen, ...)

#endif

#define log_stderr(...) do {                                            \
    Logger::_log_stderr(__VA_ARGS__);                                   \
} while (0)

#define loga(...) do {                                                  \
    Logger::_log(-1, __FILE__, __LINE__, 0, __VA_ARGS__);               \
} while (0)

#define loga_hexdump(_data, _datalen, ...) do {                         \
    Logger::_log(-1, __FILE__, __LINE__, 0, __VA_ARGS__);               \
    Logger::_log_hexdump(__FILE__, __LINE__,                            \
            static_cast<char *>(_data), static_cast<int>(_datalen),     \
            __VA_ARGS__);                                               \
} while (0)                                                             \

#define log(_level, _file, _line, ...) do {                             \
    if (Logger::loggable(_level)) {                                     \
        Logger::_log(_level, _file, _line, 0, __VA_ARGS__);             \
    }                                                                   \
} while (0)

#define log_critical(...) do {                                          \
    if (Logger::loggable(Logger::CRIT)) {                               \
        Logger::_log(Logger::CRIT, __FILE__, __LINE__, 0, __VA_ARGS__); \
    }                                                                   \
} while (0)

#define log_error(...) do {                                             \
    if (Logger::loggable(Logger::ERROR)) {                              \
        Logger::_log(Logger::ERROR, __FILE__, __LINE__, 0, __VA_ARGS__);\
    }                                                                   \
} while (0)

#define log_warn(...) do {                                              \
    if (Logger::loggable(Logger::WARN)) {                               \
        Logger::_log(Logger::WARN, __FILE__, __LINE__, 0, __VA_ARGS__); \
    }                                                                   \
} while (0)

#define log_info(...) do {                                              \
    if (Logger::loggable(Logger::INFO)) {                               \
        Logger::_log(Logger::INFO, __FILE__, __LINE__, 0, __VA_ARGS__); \
    }                                                                   \
} while (0)

#define log_panic(...) do {                                             \
    if (Logger::loggable(Logger::PANIC)) {                              \
        Logger::_log(Logger::PANIC, __FILE__, __LINE__, 1, __VA_ARGS__);\
    }                                                                   \
} while (0)

} // namespace

#endif // __LOG_H

