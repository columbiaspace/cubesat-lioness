#ifndef V4L2CAMERABASE_HPP
#define V4L2CAMERABASE_HPP

#include "V4L2Device.hpp"
#include <vector>
#include <array>
#include <cstdint>
#include <linux/videodev2.h>
#include <sys/mman.h>

/* Base Functionality For Testing:
 - Opening Device
 - Check querying capabilities
 - Set Formatting
 - Start Streaming
 - End Streaming
 - Close Device
*/

// ---------------------------------------------------------------------------
// V4L2CameraBase
// V4L2 Hal Testing
// No F Prime dependencies.
//
// Typical usage:
//   V4L2Camera cam;
//   cam.camInit();
//   cam.setCamFormat(&fmt);
//   cam.streamOn();
//   cam.streamOff();
//   cam.camClose();
// ---------------------------------------------------------------------------
class V4L2CameraBase
{
public:

    V4L2CameraBase() : openFlag(false), streaming(false) {}

    ~V4L2CameraBase()
    {
        if (streaming)   streamOff();
        if (openFlag)    camClose();
    }

    // ------------------------------------------------------------------
    // Device lifecycle
    // ------------------------------------------------------------------

    /**
     * Open devicePath
     * Returns 0 on success, -errno on failure.
     */
    int camInit(const std::string& devicePath = "");

    /** Close the device, releasing the file descriptor. */
    int camClose();

    bool camIsOpen() const { return openFlag; }

    // ------------------------------------------------------------------
    // Capability query  (VIDIOC_QUERYCAP)
    // ------------------------------------------------------------------

    /** Query and verify the device supports video capture.  Returns 0 or -errno. */
    int checkQueryCapabilities();

    const struct v4l2_capability& getCapabilities() const { return cameraCapabilities; }

    // ------------------------------------------------------------------
    // Format negotiation  (VIDIOC_G_FMT / VIDIOC_S_FMT / VIDIOC_ENUM_FMT)
    // ------------------------------------------------------------------

    /** Read the current pixel format / resolution into *format. */
    int getCamFormat(struct v4l2_format* format);

    /**
     * Propose a pixel format / resolution.  The driver may adjust the
     * values; always read back *format after this call.
     */
    int setCamFormat(struct v4l2_format* format);

    /**
     * Enumerate supported pixel formats.
     * Set formatDesc->index before each call; returns -EINVAL when the list
     * is exhausted.
     */
    int getCamFormatList(struct v4l2_fmtdesc* formatDesc);
    
    // ------------------------------------------------------------------
    // Stream control  (VIDIOC_STREAMON / VIDIOC_STREAMOFF)
    // ------------------------------------------------------------------

    /**
     * Start streaming
     * Returns 0 or -errno.
     */
    int streamOn();

    /** Stop the capture stream.  Returns 0 or -errno. */
    int streamOff();

    bool isStreaming() const { return streaming; }

    // ------------------------------------------------------------------
    // Controls  (VIDIOC_G_CTRL / VIDIOC_S_CTRL / VIDIOC_QUERYCTRL)
    // ------------------------------------------------------------------

    int getControl  (struct v4l2_control*   ctrl);
    int setControl  (struct v4l2_control*   ctrl);
    int queryControl(struct v4l2_queryctrl* qctrl);

private:
    V4L2Device             device;
    bool                   openFlag;
    bool                   streaming;
    struct v4l2_capability cameraCapabilities;
};

#endif // V4L2CAMERA_HPP