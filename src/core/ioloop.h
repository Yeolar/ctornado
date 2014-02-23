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

#ifndef __IOLOOP_H
#define __IOLOOP_H

#include "ctornado.h"

namespace ctornado {

//
// An IOLoop timeout, a UNIX timestamp and a callback
//
class Timeout
{
public:
    Timeout(int64_t deadline, cb_t callback)
        : deadline_(deadline), callback_(callback) {}

    bool operator<(const Timeout& t) const
    { return deadline_ < t.deadline_; }

    int64_t deadline_;
    cb_t callback_;
};

struct TimeoutLess
{
    bool operator()(const Timeout& t1, const Timeout& t2) const
    { return t1 < t2; }
};

//
// A level-triggered I/O loop.
//
class IOLoop
{
public:
    IOLoop(bool edge_triggered);
    virtual ~IOLoop() {}

    //
    // Returns a global edge triggered IOLoop instance.
    //
    // Most single-threaded applications have a single, global IOLoop.
    //
    static IOLoop *instance();

    //
    // Returns true if the singleton instance has been created.
    //
    static bool initialized();

    //
    // Closes the IOLoop, freeing any resources used.
    //
    // If all_fds is true, all file descriptors registered on the
    // IOLoop will be closed (not just the ones created by the IOLoop itself).
    //
    // Many applications will only use a single IOLoop that runs for the
    // entire lifetime of the process.  In that case closing the IOLoop
    // is not necessary since everything will be cleaned up when the
    // process exits.
    //
    // An IOLoop must be completely stopped before it can be closed.  This
    // means that IOLoop.stop() must be called and IOLoop.start() must
    // be allowed to return before attempting to call IOLoop.close().
    // Therefore the call to close will usually appear just after
    // the call to start rather than near the call to stop.
    //
    void close(bool all_fds);

    //
    // Registers the given handler to receive the given events for fd.
    //
    void add_handler(int fd, cb_handler_t handler, uint32_t events);

    //
    // Changes the events we listen for fd.
    //
    void update_handler(int fd, uint32_t events);

    //
    // Stop listening for events on fd.
    //
    void remove_handler(int fd);

    //
    // Starts the I/O loop.
    //
    // The loop will run until one of the I/O handlers calls stop(), which
    // will make the loop stop after the current event iteration completes.
    //
    void start();

    //
    // Stop the loop after the current event loop iteration is complete.
    // If the event loop is not currently running, the next call to start()
    // will return immediately.
    //
    // Note that even after `stop` has been called, the IOLoop is not
    // completely stopped until `IOLoop.start` has also returned.
    //
    void stop();

    //
    // Returns true if this IOLoop is currently running.
    //
    bool running();

    //
    // Calls the given callback at the time deadline from the I/O loop.
    //
    // Returns a handle that may be passed to remove_timeout to cancel.
    //
    // deadline is a number denoting a unix timestamp (msec).
    //
    // Note that it is not safe to call add_timeout from other threads.
    // Instead, you must use add_callback to transfer control to the
    // IOLoop's thread, and then call add_timeout from there.
    //
    Timeout *add_timeout(int64_t deadline, cb_t callback);

    //
    // Cancels a pending timeout.
    //
    // The argument is a handle as returned by add_timeout.
    //
    void remove_timeout(Timeout *timeout);

    //
    // Calls the given callback on the next I/O loop iteration.
    //
    // It is safe to call this method from any thread at any time.
    // Note that this is the only method in IOLoop that makes this
    // guarantee; all other interaction with the IOLoop must be done
    // from that IOLoop's thread.  add_callback() may be used to transfer
    // control from other threads to the IOLoop's thread.
    //
    void add_callback(cb_t callback);

    //
    // This method is called whenever a callback run by the IOLoop
    // throws an exception.
    //
    // By default simply logs the exception as an error.  Subclasses
    // may override this method to customize reporting of exceptions.
    //
    virtual void handle_callback_exception(cb_t callback, const Error& error);

    static const uint32_t READ  = EPoll::READ;
    static const uint32_t WRITE = EPoll::WRITE;
    static const uint32_t ERROR = EPoll::ERROR;
    static const uint32_t ET    = EPoll::ET;

private:
    static IOLoop *instance_;

    EPoll poll_;
    bool edge_triggered_;
    map<int, cb_handler_t> handlers_;
    map<int, uint32_t> events_;
    list<cb_t> callbacks_;
    pthread_mutex_t callback_lock_;
    priority_queue<Timeout, vector<Timeout>, TimeoutLess> timeouts_;
    bool running_;
    bool stopped_;

    void run_callback(cb_t callback);
};

//
// Schedules the given callback to be called periodically.
//
// The callback is called every callback_time_ milliseconds.
//
// start must be called after the PeriodicCallback is created.
//
class PeriodicCallback
{
public:
    PeriodicCallback(cb_t callback, int64_t callback_time,
            IOLoop *ioloop=nullptr);

    void start();
    void stop();
    void run();

    cb_t callback_;
    int64_t callback_time_;
    IOLoop *ioloop_;

private:
    bool running_;
    Timeout *timeout_;
    int64_t next_timeout_;

    void schedule_next();
};

} // namespace

#endif // __IOLOOP_H

