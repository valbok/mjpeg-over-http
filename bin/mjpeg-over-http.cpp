/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include <Capture/socket.h>
#include <Capture/socket_thread.h>
#include <Capture/v4l2.h>

#include <getopt.h>
#include <signal.h>

#include <mutex>
#include <iostream>
#include <chrono>
#include <vector>

#include <linux/videodev2.h>

static void help()
{
    std::cerr <<
        " ---------------------------------------------------------------\n" \
        " [-l | --listen]........: Listen on Hostname / IP\n" \
        " [-p | --port]..........: Port for this HTTP server\n" \
        " [-c | --credentials]...: Authorization: Basic \"username:password\"\n" \
        " [-d | --device]........: Camera device. By default \"/dev/video0'\"\n" \
        " [-s | --size]..........: Size of the image. By default 640x480\n" \
        " ---------------------------------------------------------------\n";
}

#define HEADER_DEFAULT "Connection: close\r\n" \
    "Server: MJPEG-Over-HTTP\r\n" \
    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
    "Pragma: no-cache\r\n" \
    "Expires: Mon, 3 Jan 2000 00:00:00 GMT\r\n"

#define BOUNDARY "mjpeg-over-http-boundary"
#define HEADER_STREAM "HTTP/1.0 200 OK\r\n" \
    "Access-Control-Allow-Origin: *\r\n" \
    HEADER_DEFAULT \
    "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
    "\r\n" \
    "--" BOUNDARY "\r\n"

#define HEADER_SNAPSHOT "HTTP/1.0 200 OK\r\n" \
    "Access-Control-Allow-Origin: *\r\n" \
    HEADER_DEFAULT \
    "Content-Type: image/jpeg\r\n"

#define HEADER_OK "HTTP/1.1 200 OK\r\n"
#define HEADER_404 "HTTP/1.0 404 Not Found\r\n"
#define HEADER_401 "HTTP/1.0 401 Unauthorized\r\n" \
    "WWW-Authenticate: Basic realm=\"MJPEG-Over-HTTP\"\r\n"

#define INFO "Motion-JPEG over HTTP<br/>" \
    "<a href=\"/stream\">Live video stream</a><br/>" \
    "<img src=\"/stream\"/><br/><br/>" \
    "<a href=\"/snapshot\" onclick=\"document.getElementById('img').src='/snapshot';return false;\">Take snapshot</a><br/>" \
    "<img id=\"img\" /><br/>"

static bool stop = false;

static void signal_handler(int sig)
{
    stop = true;
}

static void install_signal_handler()
{
    // Ignore SIGPIPE (send by OS if transmitting to closed TCP sockets)
    signal(SIGPIPE, SIG_IGN);

    // Register signal handler for <CTRL>+C in order to clean up.
    if (signal(SIGINT, signal_handler) == SIG_ERR)
        exit(EXIT_FAILURE);
}

static bool parse_opts(int argc, char **argv,
    std::string &hostname, int &port, std::string &credentials,
    std::string &device, int &width, int &height)
{
    while (1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"h", no_argument, 0, 0},
            {"help", no_argument, 0, 0},
            {"p", required_argument, 0, 0},
            {"port", required_argument, 0, 0},
            {"l", required_argument , 0, 0},
            {"listen", required_argument, 0, 0},
            {"c", required_argument, 0, 0},
            {"credentials", required_argument, 0, 0},
            {"d", required_argument, 0, 0},
            {"device", required_argument, 0, 0},
            {"s", required_argument, 0, 0},
            {"size", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        c = getopt_long_only(argc, argv, "", long_options, &option_index);

        /* no more options to parse */
        if (c == -1)
            break;

        /* unrecognized option */
        if (c == '?') {
            help();
            return false;
        }

        switch(option_index) {
        /* h, help */
        case 0:
        case 1:
            help();
            return false;
        break;

        /* p, port */
        case 2:
        case 3:
            port = atoi(optarg);
        break;

        /* Interface name */
        case 4:
        case 5:
            hostname = optarg;
        break;

        /* c, credentials */
        case 6:
        case 7:
            credentials = optarg;
        break;

        /* d, device */
        case 8:
        case 9:
            device = optarg;
        break;

        /* s, size */
        case 10:
        case 11:
            std::string size = optarg;
            auto pos = size.find('x');
            if (pos != std::string::npos) {
                width = atoi(size.substr(0, pos).c_str());
                height = atoi(size.substr(pos + 1).c_str());
            }
        break;
        }
    }

    return true;
}

static void send(Capture::socket &socket, std::string header, const std::string &mess)
{
    header += "Content-type: text/html\r\n";
    header += "Content-length: ";
    header += std::to_string(mess.size()) + "\r\n";
    header += HEADER_DEFAULT;
    header += "\r\n";
    header += mess;

    socket.write(header);
}

static std::mutex frame_mutex;
static Capture::v4l2_frame read_frame(Capture::v4l2 &v4l2)
{
    std::lock_guard<std::mutex> lock(frame_mutex);
    auto frame = v4l2.read_frame();
    if (frame && frame.pixel_format() != V4L2_PIX_FMT_MJPEG)
        frame = frame.convert(V4L2_PIX_FMT_MJPEG);

    return frame;
}

int main(int argc, char **argv)
{
    install_signal_handler();

    int port = 8080;
    std::string credentials, hostname = "0.0.0.0", device = "/dev/video0";
    int width = 640, height = 480;

    if (!parse_opts(argc, argv, hostname, port, credentials, device, width, height))
        return 1;

    Capture::v4l2 v4l2(device);
    if (!v4l2.start(width, height, V4L2_PIX_FMT_MJPEG)) {
        std::cerr << "Could not start capturing." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (v4l2.pixel_format() != V4L2_PIX_FMT_MJPEG)
        std::cout << "Motion-JPEG is not supported by the device, video frames will be converted to jpeg." << std::endl;

    Capture::socket_listener s;
    if (!s.listen(hostname.c_str(), port)) {
        std::cerr << "Could not open connection." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Host................: " << hostname << std::endl;
    std::cout << "Port................: " << port << std::endl;
    std::cout << "Authorization: Basic: " << (credentials.empty() ? "disabled" : credentials) << std::endl;
    std::cout << "Device..............: " << device << std::endl;
    std::cout << "Image size..........: " << v4l2.native_width() << "x" << v4l2.native_height() << std::endl;
    std::cout << std::endl;

    Capture::socket_thread snapshot_thread;
    snapshot_thread.start([&](auto &batch) {
        if (!v4l2.is_active()) {
            batch.clear();
            return;
        }

        auto frame = read_frame(v4l2);
        if (!frame)
            return;

        std::string header = HEADER_SNAPSHOT;
        header += "Content-Length: ";
        header += std::to_string(frame.size()) + "\r\n";
        header += "X-Timestamp: ";
        header += std::to_string(frame.timestamp().tv_sec) + "." + std::to_string(frame.timestamp().tv_usec) + "\r\n";
        header += "\r\n";

        for (auto &socket : batch) {
            if (!socket.write(header)) {
                socket.close();
                continue;
            }

            if (!socket.write(frame.data(), frame.size())) {
                socket.close();
                continue;
            }
        }
    });

    Capture::socket_thread stream_thread;
    stream_thread.start([&](auto &batch) {
        if (!v4l2.is_active()) {
            batch.clear();
            return;
        }

        auto frame = read_frame(v4l2);
        if (!frame)
            return;

        std::string header = "Content-Type: image/jpeg\r\n";
        header += "Content-Length: ";
        header += std::to_string(frame.size()) + "\r\n";
        header += "X-Timestamp: ";
        header += std::to_string(frame.timestamp().tv_sec) + "." + std::to_string(frame.timestamp().tv_usec) + "\r\n";
        header += "\r\n";

        for (auto &socket : batch) {
            if (!socket.write(header)) {
                socket.close();
                continue;
            }

            if (!socket.write(frame.data(), frame.size())) {
                socket.close();
                continue;
            }

            if (!socket.write("\r\n--" BOUNDARY "\r\n")) {
                socket.close();
                continue;
            }
        }
    });

    while (!stop) {
        s.accept([&](auto socket) {
            Capture::http_request http(socket);

            if (!credentials.empty() && credentials != http.basic_authorization()) {
                send(socket, HEADER_401, "Access denied");
                return;
            }
            if (http.uri() == "/") {
                send(socket, HEADER_OK, INFO);
                return;
            }
            if (http.uri() == "/stream") {
                if (!socket.write(HEADER_STREAM))
                    return;

                stream_thread.push(std::move(socket));
                return;
            }
            if (http.uri() == "/snapshot") {
                snapshot_thread.push(std::move(socket));
                return;
            }

            send(socket, HEADER_404, "Service is not registered");
        });
    }

    std::cout <<"exiting..." << std::endl;
    return 0;
}