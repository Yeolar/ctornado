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

// Global lock for creating global IOLoop instance
pthread_mutex_t _ioloop_instance_lock = PTHREAD_MUTEX_INITIALIZER;

IOLoop *IOLoop::instance_ = nullptr;

const uint32_t IOLoop::READ;
const uint32_t IOLoop::WRITE;
const uint32_t IOLoop::ERROR;

IOLoop::IOLoop()
{
    pthread_mutex_init(&callback_lock_, nullptr);
    running_ = false;
    stopped_ = false;
}

IOLoop *IOLoop::instance()
{
    if (instance_ == nullptr) {
        pthread_mutex_lock(&_ioloop_instance_lock);
        if (instance_ == nullptr) {
            instance_ = new IOLoop();
        }
        pthread_mutex_unlock(&_ioloop_instance_lock);
    }
    return instance_;
}

bool IOLoop::initialized()
{
    return (instance_ != nullptr);
}

void IOLoop::close(bool all_fds)
{
    if (all_fds) {
        for (auto& pair : handlers_) {
            if (::close(pair.first) == -1) {
                log_debug("error closing fd(%d): %s",
                        pair.first, strerror(errno));
            }
        }
    }
    poll_.close();
}

void IOLoop::add_handler(int fd, cb_handler_t handler, uint32_t events)
{
    log_verb("add handler on fd(%d) with events(%#x)", fd, events);

    handlers_.insert({ fd, handler });
    poll_.add(fd, events | ERROR);
}

void IOLoop::update_handler(int fd, uint32_t events)
{
    log_verb("update handler on fd(%d) with events(%#x)", fd, events);

    poll_.modify(fd, events | ERROR);
}

void IOLoop::remove_handler(int fd)
{
    log_verb("remove handler on fd(%d)", fd);

    handlers_.erase(fd);
    events_.erase(fd);

    try {
        poll_.remove(fd);
    }
    catch (IOError& e) {
        log_debug("delete fd(%d) from ioloop failed: %s", fd, e.what());
    }
}

void IOLoop::start()
{
    int64_t now, msecs, poll_timeout;
    event_list_t *event_list;
    int fd;
    uint32_t events;
    cb_handler_t handler;

    if (stopped_) {
        stopped_ = false;
        return;
    }
    running_ = true;

    while (true) {
        poll_timeout = 3600000LL;
        //
        // Prevent IO event starvation by delaying new callbacks
        // to the next iteration of the event loop.
        //
        pthread_mutex_lock(&callback_lock_);
        std::list<cb_t> callbacks(callbacks_);
        callbacks_.clear();
        pthread_mutex_unlock(&callback_lock_);

        for (auto& cb : callbacks) {
            run_callback(cb);
        }
        if (!timeouts_.empty()) {
            now = msec_now();

            while (!timeouts_.empty()) {
                Timeout timeout = timeouts_.top();

                if (timeout.callback_ == nullptr) {
                    // the timeout was cancelled
                    timeouts_.pop();
                }
                else if (timeout.deadline_ <= now) {
                    run_callback(timeout.callback_);
                }
                else {
                    msecs = timeout.deadline_ - now;
                    poll_timeout = std::min(msecs, poll_timeout);
                    break;
                }
            }
        }
        if (!callbacks_.empty()) {
            // If any callbacks or timeouts called add_callback,
            // we don't want to wait in poll() before we run them.
            poll_timeout = 0;
        }
        if (!running_)
            break;

        try {
            event_list = poll_.poll(poll_timeout);
        }
        catch (IOError& e) {
            delete event_list;

            if (e.no() == EINTR)
                continue;
            else
                throw;
        }
        //
        // Pop one fd at a time from the set of pending fds and run
        // its handler. Since that handler may perform actions on
        // other file descriptors, there may be reentrant calls to
        // this IOLoop that update self._events
        //
        events_.insert(event_list->begin(), event_list->end());
        delete event_list;

        while (!events_.empty()) {
            auto it = events_.begin();
            events_.erase(it);

            fd = it->first;
            events = it->second;
            handler = handlers_[fd];

            log_verb("run handler on fd(%d) with events(%#x)", fd, events);
            try {
                handler(fd, events);
            }
            catch (Error& e) {
                // EPIPE happens when the client closes the connection
                if (e.no() != EPIPE)
                    log_error("exception in I/O handler for fd(%d): %s",
                            fd, e.what());
            }
        }
    }
    // reset the stopped flag so another start/stop pair can be issued
    stopped_ = false;
}

void IOLoop::stop()
{
    running_ = false;
    stopped_ = true;
}

bool IOLoop::running()
{
    return running_;
}

Timeout *IOLoop::add_timeout(int64_t deadline, cb_t callback)
{
    Timeout *timeout = new Timeout(deadline, callback);
    timeouts_.push(*timeout);
    return timeout;
}

void IOLoop::remove_timeout(Timeout *timeout)
{
    timeout->callback_ = nullptr;
}

void IOLoop::add_callback(cb_t callback)
{
    pthread_mutex_lock(&callback_lock_);
    callbacks_.push_back(callback);
    pthread_mutex_unlock(&callback_lock_);
}

void IOLoop::run_callback(cb_t callback)
{
    try {
        callback();
    }
    catch (Error& e) {
        handle_callback_exception(callback, e);
    }
}

void IOLoop::handle_callback_exception(cb_t callback, const Error& error)
{
    log_error("exception in callback: %s", error.what());
}

PeriodicCallback::PeriodicCallback(cb_t callback, int64_t callback_time,
        IOLoop *ioloop)
{
    callback_ = callback;
    callback_time_ = callback_time;
    ioloop = ioloop != nullptr ? ioloop : IOLoop::instance();
    running_ = false;
    timeout_ = nullptr;
    next_timeout_ = 0;
}

void PeriodicCallback::start()
{
    running_ = true;
    next_timeout_ = msec_now();
    schedule_next();
}

void PeriodicCallback::stop()
{
    running_ = false;
    if (timeout_ != nullptr) {
        ioloop_->remove_timeout(timeout_);
        timeout_ = nullptr;
    }
}

void PeriodicCallback::run()
{
    if (!running_)
        return;
    try {
        callback_();
    }
    catch (Error& e) {
        log_error("error in periodic callback: %s", e.what());
    }
    schedule_next();
}

void PeriodicCallback::schedule_next()
{
    if (running_) {
        int64_t current_time = msec_now();
        while (next_timeout_ <= current_time) {
            next_timeout_ += callback_time_;
        }
        timeout_ = ioloop_->add_timeout(next_timeout_,
                std::bind(&PeriodicCallback::run, this));
    }
}

} // namespace
