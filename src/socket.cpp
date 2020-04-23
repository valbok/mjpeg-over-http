/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "Capture/socket.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netdb.h>
#include <errno.h>
#include <vector>

namespace Capture {

struct socket_listener_private
{
    std::vector<int> sockets;
};

socket_listener::socket_listener()
    : m(new socket_listener_private)
{
}

socket_listener::~socket_listener()
{
    close();
    delete m;
}

bool socket_listener::listen(const std::string &host, int port)
{
    char name[NI_MAXHOST];
    snprintf(name, sizeof(name), "%d", port);

    struct addrinfo hints;
    struct addrinfo *aip = nullptr;
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    int err = getaddrinfo(host.c_str(), name, &hints, &aip);
    if (err != 0) {
        perror(gai_strerror(err));
        return false;
    }

    for (auto ai = aip; ai; ai = ai->ai_next) {
        int sd = ::socket(ai->ai_family, ai->ai_socktype, 0);
        if (sd < 0)
            continue;

        // Ignore "socket already in use" errors.
        int on = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
            perror("setsockopt(SO_REUSEADDR) failed\n");

        // IPv6 socket should listen to IPv6 only, otherwise we will get "socket already in use"
        on = 1;
        if (ai->ai_family == AF_INET6 && setsockopt(sd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&on , sizeof(on)) < 0)
            perror("setsockopt(IPV6_V6ONLY) failed\n");

        if (bind(sd, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("bind");
            continue;
        }

        if (::listen(sd, 10) < 0) {
            perror("listen");
            continue;
        }

        m->sockets.push_back(sd);
    }

    return !m->sockets.empty();
}

void socket_listener::close()
{
    for (size_t i = 0; i < m->sockets.size(); ++i)
        ::close(m->sockets[i]);
}

void socket_listener::accept(const std::function<void(socket &&)> &f) const
{
    fd_set fds;
    int max_fds = 0;
    int err = 0;

    do {
        FD_ZERO(&fds);
        for (size_t i = 0; i < m->sockets.size(); ++i) {
            FD_SET(m->sockets[i], &fds);
            if (m->sockets[i] > max_fds)
                max_fds = m->sockets[i];
        }

        err = select(max_fds + 1, &fds, nullptr, nullptr, nullptr);
        if (err < 0) {
            if (errno != EINTR)
                perror("select");
            return;
        }
    } while (err <= 0);

    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    for (int i = 0; i < max_fds + 1; ++i) {
        if (!FD_ISSET(m->sockets[i], &fds))
            continue;

        int fd = ::accept(m->sockets[i], (struct sockaddr *)&client_addr, &addr_len);
        f(socket(fd));
    }
}

struct socket_private
{
    int fd = -1;
};

socket::socket(int fd)
    : m(new socket_private{ fd })
{
}

socket::socket(const socket &&other)
    : socket(-1)
{
    m->fd = other.m->fd;
    other.m->fd = -1;
}

socket::~socket()
{
    close();
    delete m;
}

int socket::fd() const
{
    return m->fd;
}

void socket::close()
{
    if (m->fd > 0)
        ::close(m->fd);

    m->fd = -1;
}

bool socket::write(const std::string &str)
{
    return write((void *)str.data(), str.size());
}

bool socket::write(const void *str, size_t size)
{
    if (::write(m->fd, str, size) < 0)
        return false;
    return true;
}

socket::operator bool() const
{
    return m->fd > 0;
}

struct http_request_private
{
    Capture::socket &socket;
    std::string header;
    std::string method;
    std::string uri;
    std::string basic_authorization;
    http_request_private(Capture::socket &s) : socket(s) { }
    std::string read_line() const;
};

std::string http_request_private::read_line() const
{
    std::string r;
    if (!socket)
        return r;

    struct timeval tv;
    fd_set fds;
    int fd = socket.fd();
    char c = '\0';
    while (c != '\n') {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        int rc = select(fd + 1, &fds, nullptr, nullptr, &tv);
        if (rc <= 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        int bytes = ::read(fd, &c, 1);
        if (bytes <= 0)
            break;
        r += c;
    }

    return r;
}

http_request::http_request(socket &socket)
    : m(new http_request_private(socket))
{
}

http_request::~http_request()
{
    delete m;
}

std::string http_request::header() const
{
    if (!m->header.empty())
        return m->header;
    std::string r;
    do {
        r = m->read_line();
        m->header += r;
    } while (!r.empty() && r != "\r\n");

    return m->header;
}

std::string http_request::method() const
{
    if (!m->method.empty())
        return m->method;

    std::string h = header();
    size_t b = h.find(' ');
    if (b != std::string::npos)
        m->method = h.substr(0, b);

    return m->method;
}

std::string http_request::uri() const
{
    if (!m->uri.empty())
        return m->uri;

    std::string h = header();
    size_t b = h.find(' ');
    size_t e = h.find(' ', b + 1);
    if (b != std::string::npos && e != std::string::npos)
        m->uri = h.substr(b + 1, e - b - 1);

    return m->uri;
}

// Taken from busybox, but it is GPL code
static void decodeBase64(char *data)
{
    const unsigned char *in = (const unsigned char *)data;
    /* The decoded size will be at most 3/4 the size of the encoded */
    unsigned ch = 0;
    int i = 0;

    while(*in) {
        int t = *in++;

        if(t >= '0' && t <= '9')
            t = t - '0' + 52;
        else if(t >= 'A' && t <= 'Z')
            t = t - 'A';
        else if(t >= 'a' && t <= 'z')
            t = t - 'a' + 26;
        else if(t == '+')
            t = 62;
        else if(t == '/')
            t = 63;
        else if(t == '=')
            t = 0;
        else
            continue;

        ch = (ch << 6) | t;
        i++;
        if(i == 4) {
            *data++ = (char)(ch >> 16);
            *data++ = (char)(ch >> 8);
            *data++ = (char) ch;
            i = 0;
        }
    }
    *data = '\0';
}

std::string http_request::basic_authorization() const
{
    if (!m->basic_authorization.empty())
        return m->basic_authorization;

    std::string h = header();
    std::string basic = "Authorization: Basic ";
    size_t b = h.find(basic);
    if (b != std::string::npos) {
        size_t e = h.find("\n", b);
        b += basic.size();
        std::string base64 = h.substr(b, e - b);
        char s[base64.size() + 1];
        std::copy(base64.data(), base64.data() + base64.size() + 1, s);
        decodeBase64(s);
        m->basic_authorization = s;
    }

    return m->basic_authorization;
}

} // Capture