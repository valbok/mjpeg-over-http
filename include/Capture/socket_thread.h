/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_SOCKET_THREAD_H
#define CAPTURE_SOCKET_THREAD_H

#include <string>
#include <functional>

namespace Capture {

class socket;
class socket_thread_private;
class socket_thread
{
public:
    socket_thread();
    socket_thread(socket_thread &&other);
    ~socket_thread();

    void push(socket &&s);
    void start(const std::function<bool(socket &)> &f);
    void stop();
    size_t size() const;

private:
    socket_thread(const socket_thread &other) = delete;
    socket_thread &operator=(const socket_thread &other) = delete;

    socket_thread_private *m = nullptr;
};

} // Capture

#endif
