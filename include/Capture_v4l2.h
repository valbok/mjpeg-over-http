/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef CAPTURE_V4L2_H
#define CAPTURE_V4L2_H

#include <string>

class Capture_v4l2_private;
class Capture_v4l2
{
public:
    Capture_v4l2(const std::string &device);
    ~Capture_v4l2();

    bool start(size_t width_hint = 0, size_t height_hint = 0, size_t pixel_format = 0, size_t buffers_count = 5);
    void stop();
    bool is_active() const;

    unsigned read_frame(void *&dst) const;

    std::string device() const;
    size_t image_size() const;
    size_t native_width() const;
    size_t native_height() const;
    size_t bytes_perline() const;
    unsigned pixel_format() const;

private:
    Capture_v4l2(const Capture_v4l2 &other) = delete;
    Capture_v4l2 &operator=(const Capture_v4l2 &other) = delete;

    Capture_v4l2_private *m = nullptr;
};

#endif // CAPTURE_V4L2_H
