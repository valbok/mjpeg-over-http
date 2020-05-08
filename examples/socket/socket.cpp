/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include <Capture/socket.h>

#include <signal.h>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

static std::atomic<bool> stop;
static std::mutex mutex;
static std::condition_variable cv;

static void signal_handler(int sig)
{
    stop = true;
    cv.notify_all();
}

int main(int argc, char **argv)
{
    // Ignore SIGPIPE (send by OS if transmitting to closed TCP sockets)
    signal(SIGPIPE, SIG_IGN);

    // Register signal handler for <CTRL>+C in order to clean up.
    if (signal(SIGINT, signal_handler) == SIG_ERR)
        exit(EXIT_FAILURE);

    Capture::socket_listener s;
    if (!s.listen("127.0.0.1", 8080)) {
        std::cerr << "Could not open connection." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for connections to 127.0.0.1:8080..." << std::endl;

    std::queue<Capture::socket> queue;

    std::thread worker(
        [&] {
            while (!stop) {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock, [&]{ return stop || !queue.empty(); });
                if (stop)
                    break;
                auto socket = std::move(queue.front());
                queue.pop();
                lock.unlock();

                static const std::string resp =
                    "HTTP/1.1 200 OK\r\n" \
                    "Access-Control-Allow-Origin: *\r\n" \
                    "Content-type: text/html\r\n" \
                    "Content-Length: 16\r\n" \
                    "Connection: close\r\n" \
                    "Server: Capture/socket/0.0\r\n" \
                    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
                    "Pragma: no-cache\r\n" \
                    "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n" \
                    "\r\n" \
                    "Capture/socket\r\n";

                socket.write(resp);
            }
        }
    );

    while (!stop) {
        s.accept([&](auto socket) {
            mutex.lock();
            queue.push(std::move(socket));
            mutex.unlock();
            cv.notify_one();
        });
    }

    worker.join();
    return 0;
}