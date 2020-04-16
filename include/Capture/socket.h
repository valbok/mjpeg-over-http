/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_SOCKET_H
#define CAPTURE_SOCKET_H

namespace Capture {

class socket_private;
class socket
{
public:
    socket();
    ~socket();
    bool listen(const char *host, int port);
    void stop();
    int accept() const;

private:
    socket(const socket &other) = delete;
    socket &operator=(const socket &other) = delete;

    socket_private *m = nullptr;
};

} // Capture

#endif // CAPTURE_SOCKET_H
