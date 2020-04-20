/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include <Capture/v4l2.h>
#include <Capture/v4l2_cache.h>
#include <chrono>
#include <iostream>

template <class C>
void run(C &c, int frame_count)
{
    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < frame_count; ++i) {
        //auto fbegin = std::chrono::steady_clock::now();    
        void *buf = nullptr;
        size_t len = c.read_frame(buf);
        if (!len) {
            std::cerr << "Could not read frame." << std::endl;
            continue;
        }

        //auto fend = std::chrono::steady_clock::now();
        //std::cout << "[" << i << "]:" << std::chrono::duration_cast<std::chrono::milliseconds>(fend - fbegin).count() << " ms" << std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Fetched " << frame_count << " frames in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << std::endl;
}

int main(int argc, char **argv)
{
    Capture::v4l2 cap("/dev/video0");
    if (!cap.start(1920, 1080)) {
        std::cerr << "Could not start capturing." << std::endl;
        return 1;
    }

    Capture::v4l2_cache cache(cap);

    int frame_count = argc > 1 ? strtol(argv[1], NULL, 0) : 10;
    
    std::cout << "Checking v4l2..." << std::endl;
    run(cap, frame_count);
    std::cout << std::endl;
    std::cout << "Checking v4l2_cache..." << std::endl;
    run(cache, frame_count * 100000);

    return 0;
}