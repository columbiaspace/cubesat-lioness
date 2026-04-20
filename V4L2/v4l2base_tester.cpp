/**
 * v4l2camerabase_tester.cpp
 *
 * Hardware tester for V4L2CameraBase.
 * Tests basic open/caps/format/stream/close pipeline against a real device.
 * No mmap or frame capture — purely control-plane validation.
 *
 * Build:
 *   g++ -std=c++17 -o v4l2camerabase_tester v4l2camerabase_tester.cpp V4L2CameraBase.cpp
 *
 * Run:
 *   ./v4l2camerabase_tester              # uses default path below
 *   ./v4l2camerabase_tester /dev/video2  # override at runtime
 *
 * Exit code: number of failed tests (0 = all passed)
 */

#include "V4L2CameraBase.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <linux/videodev2.h>

// ---------------------------------------------------------------------------
// Hardcoded device path — change this to match your board
// ---------------------------------------------------------------------------
static constexpr const char* DEFAULT_DEVICE = "/dev/video0";

// ---------------------------------------------------------------------------
// Minimal test harness
// ---------------------------------------------------------------------------

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name, expr)                                                \
    do {                                                                \
        const bool _ok = (expr);                                       \
        if (_ok) {                                                      \
            printf("  [PASS] %s\n", (name));                           \
            ++g_passed;                                                 \
        } else {                                                        \
            printf("  [FAIL] %s  (line %d)\n", (name), __LINE__);     \
            ++g_failed;                                                 \
        }                                                               \
    } while (0)

static void section(const char* name)
{
    printf("\n=== %s ===\n", name);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

// --- Empty path guard ---
static void test_empty_path()
{
    section("camInit empty path guard");
    V4L2CameraBase cam;
    int rc = cam.camInit("");
    TEST("camInit(\"\") returns -EINVAL", rc == -EINVAL);
    TEST("camIsOpen() is false after failed init", !cam.camIsOpen());
}

// --- Double open guard ---
static void test_double_open(const std::string& path)
{
    section("camInit double-open guard");
    V4L2CameraBase cam;
    cam.camInit(path);
    int rc = cam.camInit(path);
    TEST("second camInit() on open device returns -EBUSY", rc == -EBUSY);
}

// --- Basic open / close ---
static void test_open_close(const std::string& path)
{
    section("camInit / camClose");
    V4L2CameraBase cam;

    int rc = cam.camInit(path);
    TEST("camInit() returns 0",  rc == 0);
    TEST("camIsOpen() is true",  cam.camIsOpen());

    rc = cam.camClose();
    TEST("camClose() returns 0",    rc == 0);
    TEST("camIsOpen() is false",   !cam.camIsOpen());

    rc = cam.camClose();
    TEST("double camClose() returns -ENODEV", rc == -ENODEV);
}

// --- Capability query ---
static void test_capabilities(const std::string& path)
{
    section("checkQueryCapabilities");
    V4L2CameraBase cam;
    cam.camInit(path);

    int rc = cam.checkQueryCapabilities();
    TEST("checkQueryCapabilities() returns 0", rc == 0);

    const auto& caps = cam.getCapabilities();
    TEST("driver string is non-empty", caps.driver[0] != '\0');
    TEST("card string is non-empty",   caps.card[0]   != '\0');

    const uint32_t effective = (caps.capabilities & V4L2_CAP_DEVICE_CAPS)
                               ? caps.device_caps
                               : caps.capabilities;
    TEST("VIDEO_CAPTURE capability present",
         effective & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_CAPTURE_MPLANE));

    printf("    Driver:  %s\n", caps.driver);
    printf("    Card:    %s\n", caps.card);
    printf("    Version: %u.%u.%u\n",
           caps.version >> 16,
           (caps.version >> 8) & 0xFF,
           caps.version & 0xFF);
}

// --- getCamFormat ---
static void test_get_format(const std::string& path)
{
    section("getCamFormat");
    V4L2CameraBase cam;
    cam.camInit(path);

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    int rc = cam.getCamFormat(&fmt);
    TEST("getCamFormat() returns 0",     rc == 0);
    TEST("width  > 0",                   fmt.fmt.pix.width  > 0);
    TEST("height > 0",                   fmt.fmt.pix.height > 0);
    TEST("pixelformat != 0",             fmt.fmt.pix.pixelformat != 0);

    // Print the fourcc as a human-readable string
    uint32_t pf = fmt.fmt.pix.pixelformat;
    printf("    Resolution:  %ux%u\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    printf("    Pixelformat: %c%c%c%c\n",
           pf & 0xFF, (pf >> 8) & 0xFF, (pf >> 16) & 0xFF, (pf >> 24) & 0xFF);
    printf("    Bytes/line:  %u\n", fmt.fmt.pix.bytesperline);
    printf("    Image size:  %u bytes\n", fmt.fmt.pix.sizeimage);
}

// --- getCamFormat null guard ---
static void test_get_format_null(const std::string& path)
{
    section("getCamFormat null guard");
    V4L2CameraBase cam;
    cam.camInit(path);

    TEST("getCamFormat(nullptr) returns -EINVAL", cam.getCamFormat(nullptr) == -EINVAL);
}

// --- setCamFormat round-trip ---
static void test_set_format(const std::string& path)
{
    section("setCamFormat (round-trip with existing values)");
    V4L2CameraBase cam;
    cam.camInit(path);

    // Read current format, write it back — driver must accept its own values
    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    cam.getCamFormat(&fmt);

    const uint32_t orig_w  = fmt.fmt.pix.width;
    const uint32_t orig_h  = fmt.fmt.pix.height;
    const uint32_t orig_pf = fmt.fmt.pix.pixelformat;

    int rc = cam.setCamFormat(&fmt);
    TEST("setCamFormat() returns 0", rc == 0);

    // Read back and confirm driver didn't silently change something unexpected
    struct v4l2_format readback = {};
    readback.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    cam.getCamFormat(&readback);

    TEST("width  unchanged after round-trip",       readback.fmt.pix.width       == orig_w);
    TEST("height unchanged after round-trip",       readback.fmt.pix.height      == orig_h);
    TEST("pixelformat unchanged after round-trip",  readback.fmt.pix.pixelformat == orig_pf);
}

// --- setCamFormat null guard ---
static void test_set_format_null(const std::string& path)
{
    section("setCamFormat null guard");
    V4L2CameraBase cam;
    cam.camInit(path);

    TEST("setCamFormat(nullptr) returns -EINVAL", cam.setCamFormat(nullptr) == -EINVAL);
}

// --- getCamFormatList ---
static void test_format_list(const std::string& path)
{
    section("getCamFormatList");
    V4L2CameraBase cam;
    cam.camInit(path);

    struct v4l2_fmtdesc desc = {};
    desc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    desc.index = 0;

    int rc = cam.getCamFormatList(&desc);
    TEST("first format entry returns 0",       rc == 0);
    TEST("first format description non-empty", desc.description[0] != '\0');

    // Walk the full list
    printf("    Supported formats:\n");
    uint32_t count = 0;
    while (true) {
        desc.index = count;
        if (cam.getCamFormatList(&desc) < 0) break;
        uint32_t pf = desc.pixelformat;
        printf("      [%u] %c%c%c%c  %s\n", count,
               pf & 0xFF, (pf >> 8) & 0xFF, (pf >> 16) & 0xFF, (pf >> 24) & 0xFF,
               desc.description);
        ++count;
    }
    TEST("at least one format enumerated", count >= 1);
}

// --- streamOn / streamOff ---
// Note: on an i.MX camera the pipeline must be configured with media-ctl
// before STREAMON will succeed. If this test fails with EPIPE or EINVAL,
// run media-ctl to link the subdevices first.
static void test_stream(const std::string& path)
{
    section("streamOn / streamOff");
    V4L2CameraBase cam;
    cam.camInit(path);

    int rc = cam.streamOn();
    TEST("streamOn() returns 0",   rc == 0);
    TEST("isStreaming() is true",  cam.isStreaming());

    rc = cam.streamOff();
    TEST("streamOff() returns 0",  rc == 0);
    TEST("isStreaming() is false", !cam.isStreaming());
}

// --- streamOn on closed device ---
static void test_stream_closed()
{
    section("streamOn on closed device");
    V4L2CameraBase cam;   // never opened
    TEST("streamOn() on closed device returns -ENODEV", cam.streamOn() == -ENODEV);
}

// --- double streamOn guard ---
static void test_double_stream(const std::string& path)
{
    section("streamOn double-start guard");
    V4L2CameraBase cam;
    cam.camInit(path);
    cam.streamOn();

    int rc = cam.streamOn();
    TEST("second streamOn() returns -EBUSY", rc == -EBUSY);

    cam.streamOff();
}

// --- queryControl ---
static void test_query_control(const std::string& path)
{
    section("queryControl");
    V4L2CameraBase cam;
    cam.camInit(path);

    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CID_BRIGHTNESS;

    int rc = cam.queryControl(&qctrl);
    if (rc == 0) {
        TEST("queryControl(BRIGHTNESS) returns 0", true);
        TEST("BRIGHTNESS type is integer or boolean",
             qctrl.type == V4L2_CTRL_TYPE_INTEGER ||
             qctrl.type == V4L2_CTRL_TYPE_BOOLEAN);
        printf("    Brightness range: [%d, %d]  default: %d\n",
               qctrl.minimum, qctrl.maximum, qctrl.default_value);
    } else {
        printf("    BRIGHTNESS not supported on this device — skipped\n");
        TEST("queryControl: skipped", true);
    }

    TEST("queryControl(nullptr) returns -EINVAL", cam.queryControl(nullptr) == -EINVAL);
}

// --- getControl / setControl brightness round-trip ---
static void test_get_set_control(const std::string& path)
{
    section("getControl / setControl (brightness round-trip)");
    V4L2CameraBase cam;
    cam.camInit(path);

    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CID_BRIGHTNESS;
    if (cam.queryControl(&qctrl) < 0) {
        printf("    BRIGHTNESS not supported — skipping\n");
        TEST("get/setControl: skipped", true);
        return;
    }

    struct v4l2_control ctrl = {};
    ctrl.id = V4L2_CID_BRIGHTNESS;

    int rc = cam.getControl(&ctrl);
    TEST("getControl(BRIGHTNESS) returns 0", rc == 0);
    printf("    Current brightness: %d\n", ctrl.value);

    rc = cam.setControl(&ctrl);
    TEST("setControl(BRIGHTNESS, same value) returns 0", rc == 0);

    struct v4l2_control readback = { .id = V4L2_CID_BRIGHTNESS };
    cam.getControl(&readback);
    TEST("brightness unchanged after round-trip", readback.value == ctrl.value);

    TEST("getControl(nullptr) returns -EINVAL", cam.getControl(nullptr)  == -EINVAL);
    TEST("setControl(nullptr) returns -EINVAL", cam.setControl(nullptr)  == -EINVAL);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    const std::string device = (argc > 1) ? argv[1] : DEFAULT_DEVICE;

    printf("V4L2CameraBase hardware tester\n");
    printf("Device: %s\n", device.c_str());
    printf("(override: ./v4l2camerabase_tester /dev/videoN)\n");

    // --- Tests that don't need hardware ---
    test_empty_path();
    test_stream_closed();

    // --- Verify the device is accessible before running hardware tests ---
    {
        V4L2CameraBase probe;
        if (probe.camInit(device) != 0) {
            printf("\nERROR: Cannot open %s — is the device present and the "
                   "media pipeline configured?\n", device.c_str());
            printf("  On i.MX, run media-ctl to link the pipeline first.\n");
            printf("\nResult: %d passed, %d failed\n", g_passed, g_failed);
            return g_failed + 1;
        }
    }

    // --- Hardware tests (each creates its own V4L2CameraBase instance) ---
    test_double_open(device);
    test_open_close(device);
    test_capabilities(device);
    test_get_format(device);
    test_get_format_null(device);
    test_set_format(device);
    test_set_format_null(device);
    test_format_list(device);
    test_stream(device);
    test_double_stream(device);
    test_query_control(device);
    test_get_set_control(device);

    printf("\n=== SUMMARY ===\n");
    printf("Passed: %d\n", g_passed);
    printf("Failed: %d\n", g_failed);

    return g_failed;
}