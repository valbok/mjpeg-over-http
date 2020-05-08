/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "Capture/mjpeg_stream.h"

#include <vector>
#include <iostream>

namespace Capture {

struct mjpeg_stream_private
{
	std::function<void(const unsigned char *, size_t)> parsed;
    std::vector<unsigned char> vec;
    size_t content_length = 0;

    void parse_headers(const char *&b, const char *e);
};

mjpeg_stream::mjpeg_stream(const std::function<void(const unsigned char *, size_t)> &cb)
	: m(new mjpeg_stream_private{cb})
{
}

mjpeg_stream::~mjpeg_stream()
{
	delete m;
}

static std::string read_line(const char *&b, const char *e)
{
    std::string r;
    while (b && b < e && *b != '\r' && *b != '\n') {
        r += *b;
        ++b;
    }

    return r;
}

void mjpeg_stream_private::parse_headers(const char *&b, const char *e)
{
    std::string r;
    do {
        r = read_line(b, e);
        if (r.find("Content-Length: ") != std::string::npos)
            content_length = size_t(std::atoi(r.substr(r.find(' ')).c_str()));
        b += 2;
    } while (!r.empty());
}

bool mjpeg_stream::read(const char *stream, size_t size)
{
    const char *end = stream + size;
    while (stream < end) {
        if (!m->content_length)
            m->parse_headers(stream, end);

        if (!m->content_length)
            return false;

        size_t len = std::min(m->content_length, size_t(end - stream));
        m->vec.insert(m->vec.end(), stream, stream + len);
        if (m->vec.size() == m->content_length) {
            m->parsed(m->vec.data(), m->vec.size());
            m->vec.clear();
            m->content_length = 0;
        }
        stream += len;
    }

    return true;
}

} // Capture