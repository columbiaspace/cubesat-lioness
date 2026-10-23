#include "V4L2CameraBase.hpp"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <filesystem>


int V4L2CameraBase::camInit(const std::string& devicePath) {
    if (openFlag) {
        return -EBUSY;
    }

    if (devicePath.empty()) {
        std::cerr << "camInit: device path must not be empty\n";
        return -EINVAL;
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

/** Close the device, releasing the file descriptor. */
int V4L2CameraBase::camClose() {
    if (!openFlag) {
        return -ENODEV;
    }

    device.close();
    openFlag = false;
    return 0;
}

// ------------------------------------------------------------------
// Capability query  (VIDIOC_QUERYCAP)
// ------------------------------------------------------------------

/** Query and verify the device supports video capture.  Returns 0 or -errno. */
int V4L2CameraBase::checkQueryCapabilities() {
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
 
    if (caps & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        std::cout << "Device supports video capture\n";
        std::cout << "  Driver:  " << cameraCapabilities.driver << "\n";
        std::cout << "  Card:    " << cameraCapabilities.card   << "\n";
        std::cout << "  Version: "
                  << (cameraCapabilities.version >> 16) << "."
                  << ((cameraCapabilities.version >> 8) & 0xFF) << "."
                  << (cameraCapabilities.version & 0xFF) << "\n";
        return 0;
    }
 
    std::cerr << "Device does not support video capture\n";
    return -EINVAL;
}

// ------------------------------------------------------------------
// Format negotiation  (VIDIOC_G_FMT / VIDIOC_S_FMT / VIDIOC_ENUM_FMT)
// ------------------------------------------------------------------

int V4L2CameraBase::getCamFormat(struct v4l2_format* format)
{
    if (!openFlag || !format) return -EINVAL;
    return ioctl(device.fd, VIDIOC_G_FMT, format);
}

int V4L2CameraBase::setCamFormat(struct v4l2_format* format)
{
    if (!openFlag || !format) return -EINVAL;
    return ioctl(device.fd, VIDIOC_S_FMT, format);
}

int V4L2CameraBase::getCamFormatList(struct v4l2_fmtdesc* formatDesc)
{
    if (!openFlag || !formatDesc) return -EINVAL;
    return ioctl(device.fd, VIDIOC_ENUM_FMT, formatDesc);
}

// ------------------------------------------------------------------
// Stream control  (VIDIOC_STREAMON / VIDIOC_STREAMOFF)
// ------------------------------------------------------------------

int V4L2CameraBase::streamOn() {
    if (!openFlag)    return -ENODEV;
    if (streaming)    return -EBUSY;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(device.fd, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "VIDIOC_STREAMON failed: " << strerror(errno) << "\n";
        return -errno;  
    }

    streaming = true;
    std::cout << "Stream started\n";
    return 0;
}

int V4L2CameraBase::streamOff() {
    if (!openFlag)  return -ENODEV;
    if (!streaming) return -EINVAL;

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(device.fd, VIDIOC_STREAMOFF, &type) < 0) {
        std::cerr << "VIDIOC_STREAMOFF failed: " << strerror(errno) << "\n";
        return -errno;
    }

    streaming = false;
    std::cout << "Stream stopped\n";
    return 0;
}

// ------------------------------------------------------------------
// Controls  (VIDIOC_G_CTRL / VIDIOC_S_CTRL / VIDIOC_QUERYCTRL)
// ------------------------------------------------------------------

int V4L2CameraBase::getControl(struct v4l2_control* ctrl)
{
    if (!openFlag || !ctrl) return -EINVAL;
    return ioctl(device.fd, VIDIOC_G_CTRL, ctrl);
}

int V4L2CameraBase::setControl(struct v4l2_control* ctrl)
{
    if (!openFlag || !ctrl) return -EINVAL;
    return ioctl(device.fd, VIDIOC_S_CTRL, ctrl);
}

int V4L2CameraBase::queryControl(struct v4l2_queryctrl* qctrl)
{
    if (!openFlag || !qctrl) return -EINVAL;
    return ioctl(device.fd, VIDIOC_QUERYCTRL, qctrl);
}