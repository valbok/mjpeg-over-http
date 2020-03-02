# Capture v4l2

A wrapper of [Video4Linux](https://en.wikipedia.org/wiki/Video4Linux) to simplify reading of video frames from a camera.

Currently supports only V4L2_BUF_TYPE_VIDEO_CAPTURE buffer type and V4L2_MEMORY_MMAP memory.

By default Motion-JPEG (V4L2_PIX_FMT_MJPEG) is used.

Useful when there is no [GStreamer](https://gstreamer.freedesktop.org/) available but need to process video buffers.

# Build
  
    $ meson build --prefix=/path/to/install
    $ meson install -C build/
   
# Example

Following will show you a list of supported fps per pixel format.

    $ v4l2-ctl -d /dev/video0 --list-formats-ext
    Type        : Video Capture
    Pixel Format: 'MJPG' (compressed)
    Name        : Motion-JPEG
      Size: Discrete 1920x1080
      Interval: Discrete 0.033s (30.000 fps)

So using Motion-JPEG and 1920x1080, the driver provides video frames in 30 fps. Let's try:

    Capture_v4l2 cap("/dev/video0");
    if (!cap.start(1920, 1080))       
      return 1;
    
    unsigned char *buf = new unsigned char[cap.sizeImage()];
    if (cap.readFrame(buf)) {
      FILE *fp = fopen("frame.jpg", "wb");
      fwrite(buf, cap.sizeImage(), 1, fp);
      fflush(fp);
      fclose(fp);
    }

