# M-JPG over HTTP

HTTP streaming creates packets of a sequence of JPEG images that can be received by clients.
Similar idea to [mjpg-streamer](https://github.com/jacksonliam/mjpg-streamer), but a bit simpler.

    $ ./bin/mjpg-over-http
    Host................: 0.0.0.0
    Port................: 8080
    Authorization: Basic: disabled
    Device..............: /dev/video0
    Image size..........: 640x480

Now you can access the video stream by openning http://127.0.0.1:8080/stream in a browser, or inserting html tag &lt;img src="http://127.0.0.1:8080/stream" /&gt; to your webpage.
http://127.0.0.1:8080/snapshot could be used to get a snapshot from the camera.

Some clients such as QuickTime or VLC also can be used to view the stream.

The solution consists of several separate parts:

# Capture::v4l2

A wrapper of [Video4Linux](https://en.wikipedia.org/wiki/Video4Linux) to simplify reading of video frames from a camera.

Currently supports only V4L2_BUF_TYPE_VIDEO_CAPTURE buffer type and V4L2_MEMORY_MMAP memory.

By default Motion-JPEG (V4L2_PIX_FMT_MJPEG) is used.

Useful when there is no [GStreamer](https://gstreamer.freedesktop.org/) available but need to process video buffers.

    Capture::v4l2 cap("/dev/video0");
    if (!cap.start(1920, 1080))
      return;

    auto frame = cap.read_frame();
    if (frame) {
      FILE *fp = fopen("frame.jpg", "wb");
      fwrite(frame.data(), frame.size(), 1, fp);
      fclose(fp);
    }

# Capture::socket

Used to handle TCP/IP connections.

    Capture::socket_listener s;
    // Opens port for connections
    if (!s.listen(hostname, port)))
      return;
    
    // A thread to handle connections
    Capture::socket_thread worker_thread;
    worker_thread.start(
      [&](auto &batch) {
        for (auto &socket : batch)
          socket.write(response);
      });
    
    while (!stop) {
      // Waits for new connection and pushes it to worker thread
      s.accept([&](auto socket) {
        worker_thread.push(std::move(socket));
      });
    }

Capture::http_request is also useful to handle http requests:

      Capture::http_request http(socket);

      if (credentials != http.basic_authorization()) {
        socket.write(access_denied);
        return;
      }

      if (http.uri() == "/data") {
          if (!socket.write(data))
              return;
      }


# Build

    $ meson build --prefix=/path/to/install
    $ ninja install -C build/
