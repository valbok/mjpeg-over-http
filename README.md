# M-JPG over HTTP

HTTP streaming creates packets of a sequence of JPEG images that can be received by clients.
Similar idea to [mjpg-streamer](https://github.com/jacksonliam/mjpg-streamer), but a bit simpler.

    $ ./bin/mjpg-over-http
    Host................: 0.0.0.0
    Port................: 8080
    Authorization: Basic: disabled
    Device..............: /dev/video0
    Image size..........: 640x480

Now you can access the video stream by openning http://127.0.0.1:8080/stream in a browser, or inserting html tag <img src="http://127.0.0.1:8080/stream" /> to your webpage.

Some clients such as QuickTime or VLC also can be used to view the stream.

The solution consists of several separate parts:

# Capture/v4l2

A wrapper of [Video4Linux](https://en.wikipedia.org/wiki/Video4Linux) to simplify reading of video frames from a camera.

Currently supports only V4L2_BUF_TYPE_VIDEO_CAPTURE buffer type and V4L2_MEMORY_MMAP memory.

By default Motion-JPEG (V4L2_PIX_FMT_MJPEG) is used.

Useful when there is no [GStreamer](https://gstreamer.freedesktop.org/) available but need to process video buffers.

# Example

Following will show you a list of supported fps per pixel format.

    $ v4l2-ctl -d /dev/video0 --list-formats-ext
    Type        : Video Capture
    Pixel Format: 'MJPG' (compressed)
    Name        : Motion-JPEG
      Size: Discrete 1920x1080
      Interval: Discrete 0.033s (30.000 fps)

So using Motion-JPEG and 1920x1080, the driver provides video frames in 30 fps. Let's try:

    Capture::v4l2 cap("/dev/video0");
    if (!cap.start(1920, 1080))
      return;

    auto frame = cap.read_frame(buf);
    if (frame) {
      FILE *fp = fopen("frame.jpg", "wb");
      fwrite(frame.data(), frame.size(), 1, fp);
      fclose(fp);
    }

# Capture/socket

List of socket based handlers.

    Capture::socket_listener s;
    // Opens port for connections
    s.listen(hostname, port));
    // A thread to handle connections
    Capture::socket_thread worker_thread;
    worker_thread.start([&](auto &socket) { socket.write(response); return false; }
    while (!stop) {
      // Waits for new connection and pushes it to worker thread
      s.accept([&](auto socket) {
        worker_thread.push(std::move(socket));
      });
    }

# Build

    $ meson build --prefix=/path/to/install
    $ ninja install -C build/
