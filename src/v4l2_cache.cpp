/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "Capture/v4l2.h"
#include "Capture/v4l2_cache.h"

#include <chrono>
#include <memory>
#include <algorithm>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#include <iostream>

namespace Capture {

struct v4l2_cache_private
{
    const Capture::v4l2 &capture;
    v4l2_frame frame;
    std::chrono::steady_clock::time_point time;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread thread;
    std::atomic_bool stop{ true };

    ~v4l2_cache_private();
    void wait();
    void terminate();
};

v4l2_cache_private::~v4l2_cache_private()
{
    wait();
}

void v4l2_cache_private::wait()
{
    terminate();
    if (thread.joinable())
        thread.join();
}

void v4l2_cache_private::terminate()
{
    stop = true;
    cv.notify_all();
}

v4l2_cache::v4l2_cache(const Capture::v4l2 &capture)
    : m(new v4l2_cache_private{ capture })
{
}

v4l2_cache::~v4l2_cache()
{
    delete m;
}

bool v4l2_cache::start()
{
    if (!m->capture.is_active() || !m->stop)
        return false;

    m->stop = false;
    m->thread = std::thread([&] {
        while (!m->stop) {
            std::unique_lock<std::mutex> lock(m->mutex);
            m->cv.wait(lock, [&] { return m->stop || !m->frame; });
            if (m->stop)
                break;

            m->frame = m->capture.read_frame();
            lock.unlock();
            m->cv.notify_all();
        }
    });
    
    return true;
}

void v4l2_cache::stop()
{
    m->terminate();
}

v4l2_frame v4l2_cache::wait(bool update) const
{
    std::unique_lock<std::mutex> lock(m->mutex);
    m->cv.wait(lock, [&] { return m->stop || m->frame; });

    auto f = m->frame;
    if (update) {
        m->frame = v4l2_frame();
        m->cv.notify_one();
    }

    return f;
}

} // Capture