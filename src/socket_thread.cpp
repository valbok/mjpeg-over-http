/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "Capture/socket.h"
#include "Capture/socket_thread.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include <iostream>
namespace Capture {

struct socket_thread_private
{
    std::thread thread;
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<socket> queue;
    std::atomic_bool stop{ false };
    std::function<void(socket &&)> callback;
};

socket_thread::socket_thread()
    : m(new socket_thread_private)
{
}

socket_thread::~socket_thread()
{
    stop();
    if (m->thread.joinable())
        m->thread.join();
    delete m;
}

void socket_thread::push(socket &&s)
{
    m->mutex.lock();
    m->queue.push(std::move(s));
    m->mutex.unlock();
    m->cv.notify_one();
}

void socket_thread::start(const std::function<void(socket &&)> &f)
{
    m->stop = false;
    m->callback = f;
    m->thread = std::thread([&] {        
        while (!m->stop) {
            std::unique_lock<std::mutex> lock(m->mutex);
            m->cv.wait(lock, [&]{ return m->stop || !m->queue.empty(); });
            if (m->stop)
                break;

            auto socket = std::move(m->queue.front());
            m->queue.pop();
            lock.unlock();

            m->callback(std::move(socket));
        }
    });
}

void socket_thread::stop()
{
    m->stop = true;
    m->cv.notify_one();
}

} // Capture