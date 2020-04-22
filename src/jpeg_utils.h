/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#ifndef JPEG_UTILS_H
#define JPEG_UTILS_H

#include <string.h>

namespace Capture {

int jpeg_data(unsigned pixel_format, unsigned char *input, size_t width, size_t height, unsigned char *&output, int quality = 92);

}

#endif
