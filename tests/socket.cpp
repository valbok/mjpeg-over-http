/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include <Capture/socket.h>
#include <iostream>

int main(int argc, char **argv)
{
    Capture::socket s;
    if (!s.listen("127.0.0.1", 8080)) {
        std::cerr << "Could not open connection." << std::endl;
        return 1;
    }

    std::cout << "Waiting for connections to 127.0.0.1:8080..." << std::endl;
    int fd = -1;
    while ((fd = s.accept()) != -1) {
        std::cout << "connection:"<<fd<<std::endl;    
    }
    return 0;
}