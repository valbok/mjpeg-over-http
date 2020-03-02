# Capture v4l2

Wrapper to v4l2 to simplify reading of video frames.
Uses V4L2_BUF_TYPE_VIDEO_CAPTURE buffer type and V4L2_MEMORY_MMAP memory.

# Build
  
    $ meson build
    $ ninja -C build
   
# Example

    Capture_v4l2 cap("/dev/video0");
    if (!cap.start(1920, 1080))       
      return 1;
    
    unsigned char *buf = new unsigned char[cap.sizeImage()];
    if (cap.readFrame(buf)) {
      char filename[15];
      FILE *fp = fopen("frame.jpg", "wb");
      fwrite(buf, cap.sizeImage(), 1, fp);
      fflush(fp);
      fclose(fp);
    }

