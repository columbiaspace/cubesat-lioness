#ifndef V4L2DEVICE_HPP
#define V4L2DEVICE_HPP

#include <string>
#include <unistd.h>
#include <linux/videodev2.h>

struct V4L2Device
{
    std::string path;
    std::string driver;
    std::string card;
    std::string bus_info;
    __u32 capabilities;
    int fd;

    V4L2Device() : fd(-1), capabilities(0) {}

    bool isOpen() const { return fd >= 0; }

    void close()
    {
        if (fd >= 0)
        {
            ::close(fd);
            fd = -1;
        }
    }
};

#endif