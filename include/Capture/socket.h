/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_SOCKET_H
#define CAPTURE_SOCKET_H

#include <string>
#include <functional>

namespace Capture {

class socket;
class socket_listener_private;
class socket_listener
{
public:
    socket_listener();
    ~socket_listener();
    bool listen(const char *host, int port);
    void close();
    void accept(std::function<void(socket &&)> f) const;

private:
    socket_listener(const socket_listener &other) = delete;
    socket_listener &operator=(const socket_listener &other) = delete;

    socket_listener_private *m = nullptr;
};

class socket_private;
class socket
{
public:
    socket(const socket &&other);
    ~socket();

    int fd() const;
    void close();
    std::string read_line() const;
    bool write(const std::string &str);

private:
    socket(int fd);
    socket(const socket &other) = delete;
    socket &operator=(const socket &other) = delete;

	socket_private *m = nullptr;
    friend class socket_listener;
};

} // Capture

#endif // CAPTURE_SOCKET_H
