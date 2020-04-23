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
    bool listen(const std::string &host, int port);
    void close();
    void accept(const std::function<void(socket &&)> &f) const;

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
    bool write(const std::string &str);
    bool write(const void *str, size_t size);
    operator bool() const;

private:
    socket(int fd);
    socket(const socket &other) = delete;
    socket &operator=(const socket &other) = delete;

	socket_private *m = nullptr;
    friend class socket_listener;
};

class http_request_private;
class http_request
{
public:
    http_request(socket &socket);
    ~http_request();

    std::string header() const;
    std::string method() const;
    std::string uri() const;
    std::string basic_authorization() const;

private:
    http_request(const http_request &other) = delete;
    http_request &operator=(const http_request &other) = delete;

    http_request_private *m = nullptr;
};

} // Capture

#endif
