#include "V4L2Camera.hpp"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <filesystem>

// ============================================================================
// Device discovery
// ============================================================================

std::vector<V4L2Device> V4L2Camera::scanDevices()
{
    std::vector<V4L2Device> devices;

    try {
        for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
            const std::string filename = entry.path().filename().string();
            if (filename.find("video") != 0) {
                continue;
            }

            const std::string full_path = entry.path().string();
            int fd = open(full_path.c_str(), O_RDWR);
            if (fd < 0) {
                continue;
            }

            struct v4l2_capability cap;
            if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
                V4L2Device dev;
                dev.path         = full_path;
                dev.driver       = reinterpret_cast<char*>(cap.driver);
                dev.card         = reinterpret_cast<char*>(cap.card);
                dev.bus_info     = reinterpret_cast<char*>(cap.bus_info);
                dev.capabilities = cap.capabilities;
                dev.fd           = fd;

                devices.push_back(dev);

                std::cout << "Found V4L2 device: " << full_path << "\n"
                          << "  Driver: "       << dev.driver   << "\n"
                          << "  Card: "         << dev.card     << "\n"
                          << "  Bus Info: "     << dev.bus_info << "\n"
                          << "  Capabilities: 0x" << std::hex
                          << dev.capabilities   << std::dec     << "\n";
            } else {
                ::close(fd);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
    }

    return devices;
}

// ============================================================================
// Device lifecycle
// ============================================================================

int V4L2Camera::camInit(const std::string& devicePath)
{
    if (devicePath.empty()) {
        auto devices = scanDevices();
        for (auto& dev : devices) {
            if (dev.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
                device   = std::move(dev);
                openFlag = true;
                return checkQueryCapabilities();
            }
            dev.close();
        }
        return -ENODEV;
    }
    return camOpen(devicePath);
}

int V4L2Camera::camOpen(const std::string& devicePath)
{
    if (openFlag) {
        return -EBUSY;
    }

    device.path = devicePath;
    device.fd   = open(devicePath.c_str(), O_RDWR);

    if (device.fd < 0) {
        std::cerr << "Failed to open " << devicePath
                  << ": " << strerror(errno) << "\n";
        return -errno;
    }

    openFlag = true;
    return checkQueryCapabilities();
}

int V4L2Camera::camClose()
{
    if (!openFlag) {
        return -ENODEV;
    }

    device.close();
    openFlag = false;
    return 0;
}

// ============================================================================
// Capability query
// ============================================================================

int V4L2Camera::checkQueryCapabilities()
{
    if (!openFlag) {
        return -ENODEV;
    }

    if (ioctl(device.fd, VIDIOC_QUERYCAP, &cameraCapabilities) < 0) {
        std::cerr << "VIDIOC_QUERYCAP failed: " << strerror(errno) << "\n";
        return -errno;
    }

    // Use device_caps when V4L2_CAP_DEVICE_CAPS is set; fall back to
    // capabilities for older kernels.
    const uint32_t caps = (cameraCapabilities.capabilities & V4L2_CAP_DEVICE_CAPS)
                          ? cameraCapabilities.device_caps
                          : cameraCapabilities.capabilities;

    if (!(caps & V4L2_CAP_VIDEO_CAPTURE)) {
        std::cerr << "Device does not support video capture\n";
        return -EINVAL;
    }

    device.driver       = reinterpret_cast<char*>(cameraCapabilities.driver);
    device.card         = reinterpret_cast<char*>(cameraCapabilities.card);
    device.bus_info     = reinterpret_cast<char*>(cameraCapabilities.bus_info);
    device.capabilities = cameraCapabilities.capabilities;

    std::cout << "Device capabilities verified:\n"
              << "  Driver: " << device.driver << "\n"
              << "  Card: "   << device.card   << "\n"
              << "  Version: "
              << (cameraCapabilities.version >> 16) << "."
              << ((cameraCapabilities.version >> 8) & 0xFF) << "."
              << (cameraCapabilities.version & 0xFF) << "\n";

    return 0;
}

// ============================================================================
// Format negotiation
// ============================================================================

int V4L2Camera::getCamFormat(struct v4l2_format* format)
{
    if (!openFlag || !format) return -EINVAL;
    return ioctl(device.fd, VIDIOC_G_FMT, format);
}

int V4L2Camera::setCamFormat(struct v4l2_format* format)
{
    if (!openFlag || !format) return -EINVAL;
    return ioctl(device.fd, VIDIOC_S_FMT, format);
}

int V4L2Camera::getCamFormatList(struct v4l2_fmtdesc* formatDesc)
{
    if (!openFlag || !formatDesc) return -EINVAL;
    return ioctl(device.fd, VIDIOC_ENUM_FMT, formatDesc);
}

// ============================================================================
// Buffer management – low-level ioctl wrappers
// ============================================================================

int V4L2Camera::initCamMMap(struct v4l2_requestbuffers* reqbuf)
{
    if (!openFlag || !reqbuf) return -EINVAL;
    return ioctl(device.fd, VIDIOC_REQBUFS, reqbuf);
}

int V4L2Camera::queryCamBuf(struct v4l2_buffer* buffer)
{
    if (!openFlag || !buffer) return -EINVAL;
    return ioctl(device.fd, VIDIOC_QUERYBUF, buffer);
}

// ============================================================================
// Buffer management – high-level mmap helpers
// ============================================================================

int V4L2Camera::mmapBuffers(uint32_t count)
{
    if (!openFlag) return -ENODEV;
    if (count == 0 || count > MAX_BUFFERS) return -EINVAL;
    if (bufferCount > 0) {
        std::cerr << "mmapBuffers: buffers already allocated; call unmapBuffers() first\n";
        return -EBUSY;
    }

    // --- 1. Request `count` MMAP buffers from the driver ---
    struct v4l2_requestbuffers reqbuf = {};
    reqbuf.count  = count;
    reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(device.fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
        std::cerr << "VIDIOC_REQBUFS failed: " << strerror(errno) << "\n";
        return -errno;
    }

    if (reqbuf.count < 1) {
        std::cerr << "Driver returned 0 buffers\n";
        return -ENOMEM;
    }

    const uint32_t granted = std::min(reqbuf.count, static_cast<uint32_t>(MAX_BUFFERS));

    // --- 2. Query each buffer and mmap it into userspace ---
    for (uint32_t i = 0; i < granted; ++i) {
        struct v4l2_buffer buf = {};
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;

        if (ioctl(device.fd, VIDIOC_QUERYBUF, &buf) < 0) {
            std::cerr << "VIDIOC_QUERYBUF[" << i << "] failed: "
                      << strerror(errno) << "\n";
            // Unmap what we have so far before returning.
            for (uint32_t j = 0; j < i; ++j) ring[j].unmap();
            bufferCount = 0;
            return -errno;
        }

        void* addr = mmap(nullptr,
                          buf.length,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          device.fd,
                          buf.m.offset);

        if (addr == MAP_FAILED) {
            std::cerr << "mmap[" << i << "] failed: " << strerror(errno) << "\n";
            for (uint32_t j = 0; j < i; ++j) ring[j].unmap();
            bufferCount = 0;
            return -errno;
        }

        ring[i].start  = addr;
        ring[i].length = buf.length;
        ring[i].index  = i;
        ring[i].queued = false;

        std::cout << "Buffer[" << i << "] mapped: "
                  << buf.length << " bytes at " << addr << "\n";
    }

    bufferCount = granted;
    return 0;
}

void V4L2Camera::unmapBuffers()
{
    for (uint32_t i = 0; i < bufferCount; ++i) {
        ring[i].unmap();
        ring[i].queued = false;
    }
    bufferCount = 0;
}

// ============================================================================
// Stream control
// ============================================================================

int V4L2Camera::streamOn()
{
    if (!openFlag)    return -ENODEV;
    if (streaming)    return -EBUSY;
    if (!bufferCount) return -ENOMEM;

    // Queue all mapped buffers before turning the stream on.
    for (uint32_t i = 0; i < bufferCount; ++i) {
        const int rc = queueBuffer(i);
        if (rc < 0) {
            std::cerr << "Failed to queue buffer " << i
                      << " before STREAMON: " << strerror(-rc) << "\n";
            return rc;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(device.fd, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "VIDIOC_STREAMON failed: " << strerror(errno) << "\n";
        return -errno;
    }

    streaming = true;
    std::cout << "Stream started\n";
    return 0;
}

int V4L2Camera::streamOff()
{
    if (!openFlag)  return -ENODEV;
    if (!streaming) return -EINVAL;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(device.fd, VIDIOC_STREAMOFF, &type) < 0) {
        std::cerr << "VIDIOC_STREAMOFF failed: " << strerror(errno) << "\n";
        return -errno;
    }

    // Mark all ring buffers as no longer queued to the driver.
    for (uint32_t i = 0; i < bufferCount; ++i) {
        ring[i].queued = false;
    }

    streaming = false;
    std::cout << "Stream stopped\n";
    return 0;
}

// ============================================================================
// Frame queue / dequeue
// ============================================================================

int V4L2Camera::queueBuffer(uint32_t bufIndex)
{
    if (!openFlag)            return -ENODEV;
    if (bufIndex >= bufferCount) return -EINVAL;
    if (ring[bufIndex].queued)   return -EBUSY;

    struct v4l2_buffer buf = {};
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index  = bufIndex;

    if (ioctl(device.fd, VIDIOC_QBUF, &buf) < 0) {
        std::cerr << "VIDIOC_QBUF[" << bufIndex << "] failed: "
                  << strerror(errno) << "\n";
        return -errno;
    }

    ring[bufIndex].queued = true;
    return 0;
}

FrameView V4L2Camera::dequeueFrame()
{
    FrameView view;

    if (!openFlag || !streaming) {
        std::cerr << "dequeueFrame: device not open or not streaming\n";
        return view;
    }

    struct v4l2_buffer buf = {};
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(device.fd, VIDIOC_DQBUF, &buf) < 0) {
        std::cerr << "VIDIOC_DQBUF failed: " << strerror(errno) << "\n";
        return view;  // valid == false
    }

    ring[buf.index].queued = false;

    view.data      = ring[buf.index].start;
    view.byteUsed  = buf.bytesused;
    view.bufIndex  = buf.index;
    view.timestamp = buf.timestamp;
    view.valid     = true;

    return view;
}

// ============================================================================
// Controls
// ============================================================================

int V4L2Camera::getControl(struct v4l2_control* ctrl)
{
    if (!openFlag || !ctrl) return -EINVAL;
    return ioctl(device.fd, VIDIOC_G_CTRL, ctrl);
}

int V4L2Camera::setControl(struct v4l2_control* ctrl)
{
    if (!openFlag || !ctrl) return -EINVAL;
    return ioctl(device.fd, VIDIOC_S_CTRL, ctrl);
}

int V4L2Camera::queryControl(struct v4l2_queryctrl* qctrl)
{
    if (!openFlag || !qctrl) return -EINVAL;
    return ioctl(device.fd, VIDIOC_QUERYCTRL, qctrl);
}