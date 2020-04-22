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

std::string socket::read_line() const
{
    struct timeval tv;
    fd_set fds;
    std::string r;
    char c = '\0';
    while (c != '\n') {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(m->fd, &fds);
        int rc = select(m->fd + 1, &fds, nullptr, nullptr, &tv);
        if (rc <= 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        int bytes = ::read(m->fd, &c, 1);
        if (bytes <= 0)
            break;
        r += c;
    }

    return r;
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

} // Capture