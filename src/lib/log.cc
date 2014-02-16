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

static Logger logger;

const int Logger::PANIC;
const int Logger::CRIT;
const int Logger::ERROR;
const int Logger::WARN;
const int Logger::INFO;
const int Logger::DEBUG;
const int Logger::VERB;
const int Logger::VVERB;

const size_t Logger::LOG_MAX_LEN;

static const char *LOG_NAME[] =
    { "PANIC", "CRIT", "ERROR", "WARN", "INFO", "DEBUG", "VERB", "VVERB" };

Logger::Logger()
{
    name_ = nullptr;
    level_ = 0;
    fd_ = -1;
    num_error_ = 0;
}

Logger::~Logger()
{
    if (fd_ >= 0 && fd_ != STDERR_FILENO) {
        close(fd_);
    }
}

int Logger::initialize(int level, char *name)
{
    Logger *l = &logger;

    l->level_ = max(PANIC, min(level, VVERB));
    l->name_ = name;

    if (name == nullptr || !strlen(name)) {
        l->fd_ = STDERR_FILENO;
    }
    else {
        l->fd_ = open(name, O_WRONLY | O_APPEND | O_CREAT, 0644);

        if (l->fd_ < 0) {
            log_stderr("opening log file '%s' failed: %s",
                    name, strerror(errno));
            return -1;
        }
    }
    return 0;
}

void Logger::set_level(int level)
{
    Logger *l = &logger;

    l->level_ = max(PANIC, min(level, VVERB));
    loga("set log level to %s", LOG_NAME[l->level_]);
}

bool Logger::loggable(int level)
{
    Logger *l = &logger;

    return (level <= l->level_);
}

void Logger::_log(int level, const char *file, int line, int panic,
        const char *fmt, ...)
{
    Logger *l = &logger;
    int len, size, errno_save;
    char buf[LOG_MAX_LEN];
    char level_c;
    va_list args;
    struct tm *local;
    time_t t;
    ssize_t n;

    if (l->fd_ < 0) {
        return;
    }

    errno_save = errno;
    len = 0;            // length of output buffer
    size = LOG_MAX_LEN; // size of output buffer

    t = time(nullptr);
    local = localtime(&t);

    level_c = (level < 0) ? '-' : LOG_NAME[level][0];

    len += scnprintf(buf + len, size - len, "[%c ", level_c);
    len += strftime(buf + len, size - len, "%y%m%d %H:%M:%S ", local);
    len += scnprintf(buf + len, size - len, "%s:%d] ", file, line);

    va_start(args, fmt);
    len += vscnprintf(buf + len, size - len, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(l->fd_, buf, len);
    if (n < 0) {
        l->num_error_++;
    }

    errno = errno_save;

    if (panic) {
        abort();
    }
}

void Logger::_log_stderr(const char *fmt, ...)
{
    Logger *l = &logger;
    int len, size, errno_save;
    char buf[4 * LOG_MAX_LEN];
    va_list args;
    ssize_t n;

    errno_save = errno;
    len = 0;                // length of output buffer
    size = 4 * LOG_MAX_LEN; // size of output buffer

    va_start(args, fmt);
    len += vscnprintf(buf, size, fmt, args);
    va_end(args);

    buf[len++] = '\n';

    n = write(STDERR_FILENO, buf, len);
    if (n < 0) {
        l->num_error_++;
    }

    errno = errno_save;
}

//
// Hexadecimal dump in the canonical hex + ascii display
// See -C option in man hexdump
//
void Logger::_log_hexdump(const char *file, int line, char *data, int datalen,
        const char *fmt, ...)
{
    Logger *l = &logger;
    char buf[8 * LOG_MAX_LEN];
    int i, off, len, size, errno_save;
    ssize_t n;

    if (l->fd_ < 0) {
        return;
    }
    // log hexdump
    errno_save = errno;
    off = 0;                  // data offset
    len = 0;                  // length of output buffer
    size = 8 * LOG_MAX_LEN;   // size of output buffer

    while (datalen != 0 && (len < size - 1)) {
        char *save;
        const char *str;
        unsigned char c;
        int savelen;

        len += scnprintf(buf + len, size - len, "%08x  ", off);

        save = data;
        savelen = datalen;

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++) {
            c = *data;
            str = (i == 7) ? "  " : " ";
            len += scnprintf(buf + len, size - len, "%02x%s", c, str);
        }
        for ( ; i < 16; i++) {
            str = (i == 7) ? "  " : " ";
            len += scnprintf(buf + len, size - len, "  %s", str);
        }

        data = save;
        datalen = savelen;

        len += scnprintf(buf + len, size - len, "  |");

        for (i = 0; datalen != 0 && i < 16; data++, datalen--, i++) {
            c = isprint(*data) ? *data : '.';
            len += scnprintf(buf + len, size - len, "%c", c);
        }
        len += scnprintf(buf + len, size - len, "|\n");

        off += 16;
    }

    n = write(l->fd_, buf, len);
    if (n < 0) {
        l->num_error_++;
    }

    errno = errno_save;
}

} // namespace
