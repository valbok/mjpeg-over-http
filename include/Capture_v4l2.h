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

    bool start(unsigned width_hint = 0, unsigned height_hint = 0, unsigned pixel_format = 0);
    void stop();
    bool isActive() const;

    bool readFrame(unsigned char *bits);

    std::string device() const;
    unsigned sizeImage() const;
    unsigned nativeWidth() const;
    unsigned nativeHeight() const;
    unsigned bytesPerline() const;

private:
    Capture_v4l2_private *m = nullptr;
};

#endif // CAPTURE_V4L2_H
