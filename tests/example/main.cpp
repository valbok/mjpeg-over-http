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

    int frame_count = argc > 1 ? strtol(argv[1], NULL, 0) : 10;
    int i = frame_count; 
    while (i --> 0) {
        unsigned char *buf = new unsigned char[cap.sizeImage()];
        if (cap.readFrame(buf)) {
            char filename[15];
            sprintf(filename, "frame-%d.jpg", frame_count - i);
            fprintf(stdout, "Writing %s\n", filename);
            FILE *fp = fopen(filename, "wb");
            fwrite(buf, cap.sizeImage(), 1, fp);
            fflush(fp);
            fclose(fp);
        } else {
            fprintf(stderr, "Could not read frame.\n");
        }
        delete [] buf;
    }

	return 0;
}