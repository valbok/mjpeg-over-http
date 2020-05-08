/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_MPJEG_STREAM_H
#define CAPTURE_MPJEG_STREAM_H

#include <functional>
#include <string>

namespace Capture {

class mjpeg_stream_private;
class mjpeg_stream
{
public:
    mjpeg_stream(const std::function<void(const unsigned char *, size_t)> &cb);
    ~mjpeg_stream();
    bool read(const char *stream, size_t size);

private:
    mjpeg_stream(const mjpeg_stream &other) = delete;
    mjpeg_stream &operator=(const mjpeg_stream &other) = delete;

    mjpeg_stream_private *m = nullptr;
};

} // Capture

#endif
