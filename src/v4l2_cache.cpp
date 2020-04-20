/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "Capture/v4l2.h"
#include "Capture/v4l2_cache.h"

#include <chrono>
#include <memory>
#include <algorithm>

namespace Capture {

struct v4l2_cache_private
{
    const Capture::v4l2 &capture;
    std::unique_ptr<unsigned char[]> data;
    size_t size = 0;
    std::chrono::steady_clock::time_point time;
};

v4l2_cache::v4l2_cache(const Capture::v4l2 &capture)
    : m(new v4l2_cache_private{ capture })
{
}

v4l2_cache::~v4l2_cache()
{
    delete m;
}

size_t v4l2_cache::read_frame(void *&dst, unsigned timeout) const
{
    if (!m->capture.is_active())
        return 0;

    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - m->time).count();
    if (diff > timeout)
        m->data.reset();

    if (!m->data) {
        void *data = nullptr;
        m->size = m->capture.read_frame(data);
        if (m->size) {
            m->data.reset(new unsigned char[m->size]);
            unsigned char *d = (unsigned char *)data;
            std::copy(d, d + m->size, m->data.get());
        }
        m->time = std::chrono::steady_clock::now();
    }
    
    dst = m->data.get();
    return m->size;
}

} // Capture