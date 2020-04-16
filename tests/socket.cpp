/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include <Capture/socket.h>
#include <iostream>

int main(int argc, char **argv)
{
    Capture::socket_listener s;
    if (!s.listen("127.0.0.1", 8080)) {
        std::cerr << "Could not open connection." << std::endl;
        return 1;
    }

    std::cout << "Waiting for connections to 127.0.0.1:8080..." << std::endl;
    while (1) {
        s.accept([](auto socket) {
            std::cout << "connection:" << socket.fd() << std::endl;
            auto line = socket.read_line();
            std::cout << line << std::endl;
            if (line.find("GET / ") != std::string::npos) {
                std::string resp = "HTTP/1.1 200 OK\r\n" \
                            "Connection: keep-alive\r\n" \
                            "Content-Type: text/html\r\n" \
                            "Content-Length: 14\r\n" \
                            "Cache-control: no-cache, must revalidate\r\n" \
                            "\r\n" \
                            "Capture/socket";
                socket.write(resp);
            }
        });
    }

    return 0;
}