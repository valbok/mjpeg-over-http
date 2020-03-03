/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include "Capture_v4l2.h"

int main(int argc, char **argv)
{
    Capture_v4l2 cap("/dev/video0");
    if (!cap.start(1920, 1080)) {
        fprintf(stderr, "Could not start capturing.\n");
        return 1;
    }

    void *buf = nullptr;
    int frame_count = argc > 1 ? strtol(argv[1], NULL, 0) : 10;
    char filename[15];
    for (int i = 0; i < frame_count; ++i) {
        size_t len = cap.read_frame(buf);
        if (!len) {
            fprintf(stderr, "Could not read frame.\n");
            continue;
        }

        sprintf(filename, "frame-%d.jpg", i);
        fprintf(stdout, "Writing %s\n", filename);
        FILE *fp = fopen(filename, "wb");
        fwrite(buf, len, 1, fp);
        fflush(fp);
        fclose(fp);
    }

	return 0;
}