/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "jpeg_utils.h"

#include <memory>
#include <vector>
#include <jpeglib.h>

#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <iostream>

namespace Capture {

int jpeg_data(unsigned pixel_format, unsigned char *input, size_t width, size_t height, unsigned char *&output, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    output = NULL;
    uint64_t output_size = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &output, &output_size);

    // jrow is a libjpeg row of samples array of 1 row pointer
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    int z = 0;
    std::vector<uint8_t> line_buffer(width * 3);    
    JSAMPROW row_pointer[1];
    
    if (pixel_format == V4L2_PIX_FMT_YUYV) {
        while (cinfo.next_scanline < cinfo.image_height) {
            unsigned char *ptr = line_buffer.data();
            for (size_t x = 0; x < width; ++x) {
                int r, g, b;
                int y, u, v;

                if (!z)
                    y = input[0] << 8;
                else
                    y = input[2] << 8;
                u = input[1] - 128;
                v = input[3] - 128;

                r = (y + (359 * v)) >> 8;
                g = (y - (88 * u) - (183 * v)) >> 8;
                b = (y + (454 * u)) >> 8;

                *(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
                *(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
                *(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

                if (z++) {
                    z = 0;
                    input += 4;
                }
            }
            
            row_pointer[0] = line_buffer.data();
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
    } else if (pixel_format == V4L2_PIX_FMT_RGB565) {
        while (cinfo.next_scanline < cinfo.image_height) {
            unsigned char *ptr = line_buffer.data();
            for (size_t x = 0; x < width; ++x) {
                unsigned int twoByte = (input[1] << 8) + input[0];
                *(ptr++) = (input[1] & 248);
                *(ptr++) = (unsigned char)((twoByte & 2016) >> 3);
                *(ptr++) = ((input[0] & 31) * 8);
                input += 2;
            }
            
            row_pointer[0] = line_buffer.data();
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }        
    } else if (pixel_format == V4L2_PIX_FMT_UYVY) {
        while (cinfo.next_scanline < cinfo.image_height) {
            unsigned char *ptr = line_buffer.data();
            for (size_t x = 0; x < width; ++x) {
                int r, g, b;
                int y, u, v;

                if (!z)
                    y = input[1] << 8;
                else
                    y = input[3] << 8;
                u = input[0] - 128;
                v = input[2] - 128;

                r = (y + (359 * v)) >> 8;
                g = (y - (88 * u) - (183 * v)) >> 8;
                b = (y + (454 * u)) >> 8;

                *(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
                *(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
                *(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

                if (z++) {
                    z = 0;
                    input += 4;
                }
            }

            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return output_size;
}

} // Capture