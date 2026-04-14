#ifndef V4L2CAMERA_HPP
#define V4L2CAMERA_HPP

#include "V4L2Device.hpp"
#include <vector>
#include <array>
#include <cstdint>
#include <linux/videodev2.h>
#include <sys/mman.h>

// ---------------------------------------------------------------------------
// MappedBuffer
// Represents a single kernel MMAP buffer that has been mapped into userspace.
// ---------------------------------------------------------------------------
struct MappedBuffer {
    void*    start  = nullptr;   // mmap'd userspace pointer
    size_t   length = 0;         // byte length of the mapping
    uint32_t index  = 0;         // V4L2 buffer index (0-based)
    bool     queued = false;     // true while owned by the driver

    bool isMapped() const { return start != nullptr && start != MAP_FAILED; }

    void unmap() {
        if (isMapped()) {
            munmap(start, length);
            start  = nullptr;
            length = 0;
        }
    }
};

// ---------------------------------------------------------------------------
// FrameView
// A lightweight, non-owning handle returned by dequeueFrame().
// The caller must call releaseFrame() / requeueBuffer() before the next
// dequeueFrame(), otherwise the driver will starve of buffers.
// ---------------------------------------------------------------------------
struct FrameView {
    const void*      data      = nullptr;
    size_t           byteUsed  = 0;       // bytes actually filled by driver
    uint32_t         bufIndex  = 0;       // index into the MappedBuffer ring
    struct timeval   timestamp = {};      // capture timestamp from V4L2
    bool             valid     = false;
};

// ---------------------------------------------------------------------------
// V4L2Camera
// Hardware-agnostic abstraction over the Linux V4L2 capture pipeline.
// No F Prime dependencies.
//
// Typical usage:
//   V4L2Camera cam;
//   cam.camInit();                               // auto-detect first capture device
//   cam.setCamFormat(&fmt);
//   cam.mmapBuffers(4);                          // allocate & mmap 4 kernel buffers
//   cam.streamOn();
//   while (running) {
//       FrameView f = cam.dequeueFrame();
//       if (f.valid) {
//           process(f.data, f.byteUsed);
//           cam.requeueBuffer(f.bufIndex);       // hand buffer back to driver
//       }
//   }
//   cam.streamOff();
//   cam.unmapBuffers();
//   cam.camClose();
// ---------------------------------------------------------------------------
class V4L2Camera
{
public:
    // Maximum number of MMAP buffers supported by the circular ring.
    static constexpr uint32_t MAX_BUFFERS = 8;

    V4L2Camera() : openFlag(false), streaming(false), bufferCount(0) {}

    ~V4L2Camera()
    {
        if (streaming)   streamOff();
        if (bufferCount) unmapBuffers();
        if (openFlag)    camClose();
    }

    // ------------------------------------------------------------------
    // Device lifecycle
    // ------------------------------------------------------------------

    /**
     * Scan /dev for V4L2 devices and return metadata for each one found.
     * File descriptors opened during the scan are stored in the returned
     * V4L2Device objects; callers that do not pass a device to camInit()
     * should call V4L2Device::close() on unused entries.
     */
    static std::vector<V4L2Device> scanDevices();

    /**
     * Open the first capture-capable device found if devicePath is empty,
     * otherwise open the specified path.  Returns 0 on success, -errno on
     * failure.
     */
    int camInit(const std::string& devicePath = "");

    /** Open a specific device path.  Returns 0 or -errno. */
    int camOpen(const std::string& devicePath);

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
    // Buffer management  (VIDIOC_REQBUFS / VIDIOC_QUERYBUF + mmap)
    // ------------------------------------------------------------------

    /**
     * Request `count` MMAP buffers from the kernel and mmap() each one
     * into userspace.  Stores mapped pointers in the internal ring.
     * `count` must be in [1, MAX_BUFFERS].
     * Returns 0 on success, -errno on failure.
     */
    int mmapBuffers(uint32_t count);

    /** Unmap all previously mmap'd buffers and release them. */
    void unmapBuffers();

    /** Return the number of successfully mapped buffers. */
    uint32_t getBufferCount() const { return bufferCount; }

    /**
     * Low-level wrappers kept for direct ioctl access if needed.
     * Prefer mmapBuffers() for normal use.
     */
    int initCamMMap(struct v4l2_requestbuffers* reqbuf);
    int queryCamBuf(struct v4l2_buffer* buffer);

    // ------------------------------------------------------------------
    // Stream control  (VIDIOC_STREAMON / VIDIOC_STREAMOFF)
    // ------------------------------------------------------------------

    /**
     * Queue all mapped buffers to the driver and start the capture stream.
     * mmapBuffers() must be called first.  Returns 0 or -errno.
     */
    int streamOn();

    /** Stop the capture stream.  Returns 0 or -errno. */
    int streamOff();

    bool isStreaming() const { return streaming; }

    // ------------------------------------------------------------------
    // Frame queue / dequeue  (VIDIOC_QBUF / VIDIOC_DQBUF)
    // ------------------------------------------------------------------

    /**
     * Queue a single buffer back to the driver by its ring index.
     * Typically called after the caller is done processing a FrameView
     * obtained from dequeueFrame().
     * Returns 0 or -errno.
     */
    int queueBuffer(uint32_t bufIndex);

    /**
     * Dequeue the next completed frame from the driver (blocking).
     * Returns a FrameView with valid == true on success.
     * On error, returns a FrameView with valid == false.
     * The caller MUST call requeueBuffer() when done with the frame data.
     */
    FrameView dequeueFrame();

    /**
     * Convenience alias for queueBuffer(); mirrors the language of the
     * GitHub issue ("requeueBuffer() or releaseFrame()").
     */
    int requeueBuffer(uint32_t bufIndex) { return queueBuffer(bufIndex); }

    /**
     * Alias kept for symmetry with requeueBuffer().
     * Identical behaviour: re-enqueues the buffer at bufIndex.
     */
    int releaseFrame(uint32_t bufIndex)  { return queueBuffer(bufIndex); }

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

    // Circular ring of mmap'd kernel buffers.
    std::array<MappedBuffer, MAX_BUFFERS> ring;
    uint32_t bufferCount;   // number of entries actually populated in ring[]
};

#endif // V4L2CAMERA_HPP