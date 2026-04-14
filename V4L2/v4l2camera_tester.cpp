/**
 * v4l2camera_tester.cpp
 *
 * One test case per V4L2Camera API call.
 *
 * Build (on target):
 *   g++ -std=c++17 -o v4l2camera_tester v4l2camera_tester.cpp V4L2Camera.cpp
 *
 * Run:
 *   ./v4l2camera_tester [/dev/videoN]
 *
 * Exit code:
 *   0  – all tests passed
 *   N  – N tests failed
 */

#include "V4L2Camera.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <linux/videodev2.h>

// ---------------------------------------------------------------------------
// Minimal test harness
// ---------------------------------------------------------------------------

static int  g_passed = 0;
static int  g_failed = 0;

#define TEST(name, expr)                                                    \
    do {                                                                    \
        const bool _ok = (expr);                                           \
        if (_ok) {                                                          \
            printf("  [PASS] %s\n", (name));                               \
            ++g_passed;                                                     \
        } else {                                                            \
            printf("  [FAIL] %s  (line %d)\n", (name), __LINE__);         \
            ++g_failed;                                                     \
        }                                                                   \
    } while (0)

static void section(const char* name)
{
    printf("\n=== %s ===\n", name);
}

// ---------------------------------------------------------------------------
// Individual test functions
// ---------------------------------------------------------------------------

// --- scanDevices ---
static void test_scanDevices()
{
    section("scanDevices");

    auto devices = V4L2Camera::scanDevices();

    TEST("at least one /dev/videoN found", !devices.empty());
    if (!devices.empty()) {
        TEST("first device has a non-empty path",   !devices[0].path.empty());
        TEST("first device fd is valid",             devices[0].fd >= 0);
        TEST("first device has a driver string",    !devices[0].driver.empty());
    }

    // Close every fd opened by the scan that we won't use.
    for (auto& d : devices) d.close();
}

// --- camInit (auto-detect) ---
static void test_camInit_auto(V4L2Camera& cam)
{
    section("camInit (auto-detect)");

    int rc = cam.camInit();
    TEST("camInit() returns 0",   rc == 0);
    TEST("camIsOpen() is true",   cam.camIsOpen());
}

// --- camInit on already-open device returns -EBUSY ---
static void test_camOpen_busy(V4L2Camera& cam)
{
    section("camOpen on already-open device");

    int rc = cam.camOpen("/dev/video0");
    TEST("camOpen on open device returns -EBUSY", rc == -EBUSY);
}

// --- checkQueryCapabilities ---
static void test_checkQueryCapabilities(V4L2Camera& cam)
{
    section("checkQueryCapabilities");

    int rc = cam.checkQueryCapabilities();
    TEST("checkQueryCapabilities() returns 0", rc == 0);

    const auto& caps = cam.getCapabilities();
    TEST("driver string non-empty",
         caps.driver[0] != '\0');

    // At least one of the capability bits for video capture must be set.
    const bool hasCapture =
        (caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
        ((caps.capabilities & V4L2_CAP_DEVICE_CAPS) &&
         (caps.device_caps  & V4L2_CAP_VIDEO_CAPTURE));
    TEST("device reports VIDEO_CAPTURE capability", hasCapture);
}

// --- getCamFormat ---
static void test_getCamFormat(V4L2Camera& cam)
{
    section("getCamFormat");

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int rc = cam.getCamFormat(&fmt);
    TEST("getCamFormat() returns 0",        rc == 0);
    TEST("width  is non-zero",              fmt.fmt.pix.width  > 0);
    TEST("height is non-zero",              fmt.fmt.pix.height > 0);
    TEST("pixelformat is non-zero",         fmt.fmt.pix.pixelformat != 0);
}

// --- getCamFormat with null pointer ---
static void test_getCamFormat_null(V4L2Camera& cam)
{
    section("getCamFormat (null pointer guard)");

    int rc = cam.getCamFormat(nullptr);
    TEST("getCamFormat(nullptr) returns -EINVAL", rc == -EINVAL);
}

// --- setCamFormat (round-trip: read, write back the same values) ---
static void test_setCamFormat(V4L2Camera& cam)
{
    section("setCamFormat (round-trip)");

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    cam.getCamFormat(&fmt);  // read current values

    int rc = cam.setCamFormat(&fmt);
    TEST("setCamFormat() returns 0 for existing format", rc == 0);
}

// --- setCamFormat with null pointer ---
static void test_setCamFormat_null(V4L2Camera& cam)
{
    section("setCamFormat (null pointer guard)");

    int rc = cam.setCamFormat(nullptr);
    TEST("setCamFormat(nullptr) returns -EINVAL", rc == -EINVAL);
}

// --- getCamFormatList ---
static void test_getCamFormatList(V4L2Camera& cam)
{
    section("getCamFormatList");

    struct v4l2_fmtdesc desc = {};
    desc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    desc.index = 0;

    int rc = cam.getCamFormatList(&desc);
    TEST("first format entry returns 0",          rc == 0);
    TEST("first format description non-empty",
         desc.description[0] != '\0');

    // Walk the full list; driver returns -EINVAL at end.
    uint32_t count = 1;
    while (true) {
        desc.index = count;
        if (cam.getCamFormatList(&desc) < 0) break;
        ++count;
    }
    printf("    formats enumerated: %u\n", count);
    TEST("at least one format enumerated", count >= 1);
}

// --- initCamMMap + queryCamBuf (low-level wrappers) ---
static void test_lowlevel_mmap_wrappers(V4L2Camera& cam)
{
    section("initCamMMap + queryCamBuf (low-level wrappers)");

    struct v4l2_requestbuffers reqbuf = {};
    reqbuf.count  = 2;
    reqbuf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    int rc = cam.initCamMMap(&reqbuf);
    TEST("initCamMMap() returns 0", rc == 0);
    TEST("driver granted at least 1 buffer", reqbuf.count >= 1);

    if (reqbuf.count >= 1) {
        struct v4l2_buffer buf = {};
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = 0;

        rc = cam.queryCamBuf(&buf);
        TEST("queryCamBuf() for index 0 returns 0", rc == 0);
        TEST("buffer length is non-zero",           buf.length > 0);
    }

    // Release the kernel allocation so mmapBuffers() can proceed cleanly.
    reqbuf.count = 0;
    cam.initCamMMap(&reqbuf);
}

// --- mmapBuffers + unmapBuffers ---
static void test_mmapBuffers(V4L2Camera& cam)
{
    section("mmapBuffers");

    int rc = cam.mmapBuffers(4);
    TEST("mmapBuffers(4) returns 0",           rc == 0);
    TEST("getBufferCount() == 4 (or granted)", cam.getBufferCount() >= 1);

    cam.unmapBuffers();
    TEST("unmapBuffers() clears count",        cam.getBufferCount() == 0);
}

// --- mmapBuffers called twice without unmapping ---
static void test_mmapBuffers_double(V4L2Camera& cam)
{
    section("mmapBuffers double-alloc guard");

    cam.mmapBuffers(2);
    int rc = cam.mmapBuffers(2);
    TEST("second mmapBuffers() without unmapping returns -EBUSY", rc == -EBUSY);
    cam.unmapBuffers();
}

// --- streamOn / streamOff ---
static void test_stream(V4L2Camera& cam)
{
    section("streamOn / streamOff");

    // Need buffers before streaming.
    cam.mmapBuffers(4);

    int rc = cam.streamOn();
    TEST("streamOn() returns 0",        rc == 0);
    TEST("isStreaming() is true",       cam.isStreaming());

    rc = cam.streamOff();
    TEST("streamOff() returns 0",      rc == 0);
    TEST("isStreaming() is false",     !cam.isStreaming());

    cam.unmapBuffers();
}

// --- streamOn without buffers ---
static void test_streamOn_no_buffers(V4L2Camera& cam)
{
    section("streamOn without buffers");

    // Ensure no buffers are allocated.
    if (cam.getBufferCount() > 0) cam.unmapBuffers();

    int rc = cam.streamOn();
    TEST("streamOn() without buffers returns -ENOMEM", rc == -ENOMEM);
}

// --- dequeueFrame + requeueBuffer ---
static void test_dequeue_requeue(V4L2Camera& cam)
{
    section("dequeueFrame + requeueBuffer");

    cam.mmapBuffers(4);
    cam.streamOn();

    FrameView frame = cam.dequeueFrame();
    TEST("dequeueFrame() returns valid frame", frame.valid);

    if (frame.valid) {
        TEST("frame data pointer non-null",  frame.data     != nullptr);
        TEST("frame bytesUsed > 0",          frame.byteUsed > 0);
        TEST("frame bufIndex in range",      frame.bufIndex < cam.getBufferCount());

        int rc = cam.requeueBuffer(frame.bufIndex);
        TEST("requeueBuffer() returns 0", rc == 0);

        // releaseFrame() is an alias; test it with the same index to ensure
        // the guard against double-queue fires correctly.
        rc = cam.requeueBuffer(frame.bufIndex);
        TEST("double-queue same buffer returns -EBUSY", rc == -EBUSY);
    }

    cam.streamOff();
    cam.unmapBuffers();
}

// --- releaseFrame alias ---
static void test_releaseFrame(V4L2Camera& cam)
{
    section("releaseFrame (alias)");

    cam.mmapBuffers(4);
    cam.streamOn();

    FrameView frame = cam.dequeueFrame();
    if (frame.valid) {
        int rc = cam.releaseFrame(frame.bufIndex);
        TEST("releaseFrame() returns 0", rc == 0);
    } else {
        TEST("dequeueFrame() returned a valid frame for releaseFrame test", false);
    }

    cam.streamOff();
    cam.unmapBuffers();
}

// --- queryControl ---
static void test_queryControl(V4L2Camera& cam)
{
    section("queryControl");

    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CID_BRIGHTNESS;

    int rc = cam.queryControl(&qctrl);
    if (rc == 0) {
        TEST("queryControl(BRIGHTNESS) returns 0",      rc == 0);
        TEST("BRIGHTNESS type is integer or boolean",
             qctrl.type == V4L2_CTRL_TYPE_INTEGER ||
             qctrl.type == V4L2_CTRL_TYPE_BOOLEAN);
    } else {
        // Some cameras genuinely do not have brightness; skip without failing.
        printf("    BRIGHTNESS not supported on this device (rc=%d) – skipped\n", rc);
        TEST("queryControl: skipped (not supported)", true);
    }

    TEST("queryControl(nullptr) returns -EINVAL", cam.queryControl(nullptr) == -EINVAL);
}

// --- getControl + setControl (brightness round-trip) ---
static void test_getControl_setControl(V4L2Camera& cam)
{
    section("getControl / setControl (brightness round-trip)");

    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CID_BRIGHTNESS;

    if (cam.queryControl(&qctrl) < 0) {
        printf("    BRIGHTNESS not supported – skipping get/set tests\n");
        TEST("get/setControl: skipped (not supported)", true);
        return;
    }

    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_BRIGHTNESS;

    int rc = cam.getControl(&ctrl);
    TEST("getControl(BRIGHTNESS) returns 0", rc == 0);

    const int32_t original = ctrl.value;
    printf("    Original brightness: %d\n", original);

    // Write the same value back.
    rc = cam.setControl(&ctrl);
    TEST("setControl(BRIGHTNESS, same value) returns 0", rc == 0);

    // Read back and compare.
    rc = cam.getControl(&ctrl);
    TEST("getControl after setControl returns 0",     rc == 0);
    TEST("value unchanged after round-trip write",    ctrl.value == original);

    TEST("getControl(nullptr)  returns -EINVAL", cam.getControl(nullptr)  == -EINVAL);
    TEST("setControl(nullptr)  returns -EINVAL", cam.setControl(nullptr)  == -EINVAL);
}

// --- camClose ---
static void test_camClose(V4L2Camera& cam)
{
    section("camClose");

    int rc = cam.camClose();
    TEST("camClose() returns 0",     rc == 0);
    TEST("camIsOpen() is false",    !cam.camIsOpen());

    rc = cam.camClose();
    TEST("double camClose() returns -ENODEV", rc == -ENODEV);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    const std::string devicePath = (argc > 1) ? argv[1] : "";

    printf("V4L2Camera tester\n");
    if (devicePath.empty()) {
        printf("No device path given; using auto-detect.\n");
    } else {
        printf("Using device: %s\n", devicePath.c_str());
    }

    // ---- Tests that do not need an open device ----
    test_scanDevices();

    // ---- Tests that share one V4L2Camera instance ----
    V4L2Camera cam;

    if (devicePath.empty()) {
        test_camInit_auto(cam);
    } else {
        section("camOpen");
        int rc = cam.camOpen(devicePath);
        TEST("camOpen() returns 0",  rc == 0);
        TEST("camIsOpen() is true",  cam.camIsOpen());
    }

    if (!cam.camIsOpen()) {
        printf("\nERROR: could not open a camera; aborting remaining tests.\n");
        printf("\nResult: %d passed, %d failed\n", g_passed, g_failed);
        return g_failed;
    }

    test_camOpen_busy(cam);
    test_checkQueryCapabilities(cam);
    test_getCamFormat(cam);
    test_getCamFormat_null(cam);
    test_setCamFormat(cam);
    test_setCamFormat_null(cam);
    test_getCamFormatList(cam);
    test_lowlevel_mmap_wrappers(cam);
    test_mmapBuffers(cam);
    test_mmapBuffers_double(cam);
    test_stream(cam);
    test_streamOn_no_buffers(cam);
    test_dequeue_requeue(cam);
    test_releaseFrame(cam);
    test_queryControl(cam);
    test_getControl_setControl(cam);
    test_camClose(cam);

    printf("\n=== SUMMARY ===\n");
    printf("Passed: %d\n", g_passed);
    printf("Failed: %d\n", g_failed);

    return g_failed;  // 0 means all tests passed
}