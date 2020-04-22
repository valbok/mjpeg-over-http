/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_SOCKET_THREAD_H
#define CAPTURE_SOCKET_THREAD_H

#include <string>
#include <functional>
#include <vector>

namespace Capture {

using sockets = std::vector<socket>;

class socket;
class socket_thread_private;
class socket_thread
{
public:
    socket_thread();
    socket_thread(socket_thread &&other);
    ~socket_thread();

    void push(socket &&s);
    void start(const std::function<void(sockets &)> &f);
    void stop();

private:
    socket_thread(const socket_thread &other) = delete;
    socket_thread &operator=(const socket_thread &other) = delete;

    socket_thread_private *m = nullptr;
};

} // Capture

#endif
