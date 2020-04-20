/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_V4L2_CACHE_H
#define CAPTURE_V4L2_CACHE_H

#include <cstddef>

namespace Capture {

class v4l2;
class v4l2_cache_private;
class v4l2_cache
{
public:
    v4l2_cache(const Capture::v4l2 &capture);
    ~v4l2_cache();

    size_t read_frame(void *&dst, unsigned timeout = 30) const;

private:
    v4l2_cache(const v4l2_cache &other) = delete;
    v4l2_cache &operator=(const v4l2_cache &other) = delete;

    v4l2_cache_private *m = nullptr;
};

} // Capture

#endif
