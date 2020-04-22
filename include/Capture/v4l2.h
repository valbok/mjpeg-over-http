/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_V4L2_H
#define CAPTURE_V4L2_H

#include <string>

namespace Capture {

class v4l2_frame_private;
class v4l2_frame
{
public:
    v4l2_frame();
    ~v4l2_frame();

    v4l2_frame(const v4l2_frame &&other);
    v4l2_frame(const v4l2_frame &other);
    v4l2_frame &operator=(const v4l2_frame &other);
    operator bool() const;

    const void *data() const;
    size_t size() const;
    struct timeval timestamp() const;

private:
    v4l2_frame_private *m = nullptr;
    friend class v4l2;
};

class v4l2_private;
class v4l2
{
public:
    v4l2(const std::string &device);
    ~v4l2();

    bool start(size_t width_hint = 0, size_t height_hint = 0, size_t pixel_format = 0, size_t buffers_count = 5);
    void stop();
    bool is_active() const;

    v4l2_frame read_frame() const;

    std::string device() const;
    size_t image_size() const;
    size_t native_width() const;
    size_t native_height() const;
    size_t bytes_perline() const;
    unsigned pixel_format() const;

private:
    v4l2(const v4l2 &other) = delete;
    v4l2 &operator=(const v4l2 &other) = delete;

    v4l2_private *m = nullptr;
};

} // Capture

#endif
