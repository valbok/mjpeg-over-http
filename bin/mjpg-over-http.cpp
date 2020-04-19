/**
 * Copyright (C) 2020, Val Doroshchuk <valbok@gmail.com>
 */

#include <Capture/socket.h>
#include <Capture/v4l2.h>

#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

static void help()
{
    std::cerr <<
        " ---------------------------------------------------------------\n" \
        " [-l | --listen]........: Listen on Hostname / IP\n" \
        " [-p | --port]..........: Port for this HTTP server\n" \
        " [-c | --credentials]...: Authorization: Basic \"username:password\"\n" \
        " [-d | --device]........: Camera device. By default \"/dev/video0'\"\n" \
        " ---------------------------------------------------------------\n";
}

#define HEADER_DEFAULT "Connection: close\r\n" \
    "Server: Capture/mjpg-over-http/0.0\r\n" \
    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
    "Pragma: no-cache\r\n" \
    "Expires: Mon, 3 Jan 2000 00:00:00 GMT\r\n"

#define BOUNDARY "mjpg-over-http-boundary"
#define HEADER_STREAM "HTTP/1.0 200 OK\r\n" \
    "Access-Control-Allow-Origin: *\r\n" \
    HEADER_DEFAULT \
    "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
    "\r\n" \
    "--" BOUNDARY "\r\n"

#define HEADER_404 "HTTP/1.0 404 Not Found\r\n"

static std::atomic<bool> stop(false);
static std::mutex socket_mutex;
static std::condition_variable socket_cv;
static std::queue<Capture::socket> socket_queue;

static void signal_handler(int sig)
{
    stop = true;
    socket_cv.notify_all();
}

static void install_signal_handler()
{
    // Ignore SIGPIPE (send by OS if transmitting to closed TCP sockets)
    signal(SIGPIPE, SIG_IGN);

    // Register signal handler for <CTRL>+C in order to clean up.
    if (signal(SIGINT, signal_handler) == SIG_ERR)
        exit(EXIT_FAILURE);
}

static bool parse_opts(int argc, char **argv, std::string &hostname, int &port, std::string &credentials, std::string &device)
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
        }
    }

    return true;
}

static void send_error(Capture::socket &socket, std::string header, const std::string &mess)
{
    header += "Content-type: text/plain\r\n";
    header += "Content-length: ";
    header += std::to_string(mess.size()) + "\r\n";
    header += HEADER_DEFAULT;
    header += "\r\n";
    header += mess;

    socket.write(header);
}

void worker_thread()
{
    int frame_size = 800*600*4;
    unsigned char *frame = new unsigned char[frame_size] {0};

    while (!stop) {
        std::unique_lock<std::mutex> lock(socket_mutex);
        socket_cv.wait(lock, [&]{ return stop || !socket_queue.empty(); });
        if (stop)
            break;

        auto socket = std::move(socket_queue.front());
        socket_queue.pop();
        lock.unlock();

        std::string header = "Content-Type: image/jpeg\r\n";
        header += "Content-Length: ";
        header += std::to_string(frame_size) + "\r\n";
        header += "\r\n";

        if (!socket.write(header))
            continue;

        if (!socket.write(frame, frame_size))
            continue;

        if (!socket.write("\r\n--" BOUNDARY "\r\n"))
            continue;

        lock.lock();
        socket_queue.push(std::move(socket));
        lock.unlock();
    }
}

int main(int argc, char **argv)
{
    install_signal_handler();

    int port = 8080;
    std::string credentials, hostname = "0.0.0.0", device = "/dev/video0";

    if (!parse_opts(argc, argv, hostname, port, credentials, device))
        return 1;

    std::cout << "Host................: " << hostname << std::endl;
    std::cout << "Port................: " << port << std::endl;
    std::cout << "Authorization: Basic: " << (credentials.empty() ? "disabled" : credentials) << std::endl;
    std::cout << "Device..............: " << device << std::endl << std::endl;

    Capture::socket_listener s;
    if (!s.listen(hostname.c_str(), port)) {
        std::cerr << "Could not open connection." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for connections to " << hostname << ":" << port << "..." << std::endl;    

    std::thread worker(worker_thread);
    while (!stop) {
        s.accept([&](auto socket) {
            std::string req = socket.read_line();
            if (req.find("GET /stream ") != std::string::npos) {
                if (!socket.write(HEADER_STREAM))
                    return;

                socket_mutex.lock();
                socket_queue.push(std::move(socket));
                socket_mutex.unlock();
                socket_cv.notify_one();
                return;
            }

            send_error(socket, HEADER_404, "Service is not registered");
        });
    }

    worker.join();
    return 0;
}