/**
 * capture.cpp — Standalone MIPI CSI camera capture using the libcamera C++ API.
 *
 * Usage: ./capture [output_file] [width] [height]
 *   Defaults: frame.raw  1920  1080
 *
 * Captures a single still frame in YUV420 format and writes the raw bytes
 * to disk.  Prints the ffmpeg command needed to convert to PNG.
 */

#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#include <libcamera/libcamera.h>

using namespace libcamera;

static bool requestDone = false;

static void requestCompleted(Request *request)
{
    if (request->status() == Request::RequestComplete) {
        std::cout << "[INFO] Request completed successfully.\n";
    } else {
        std::cerr << "[ERROR] Request completed with errors.\n";
    }
    requestDone = true;
}

int main(int argc, char *argv[])
{
    // ── CLI arguments ────────────────────────────────────────────────
    std::string outputFile = "frame.raw";
    unsigned int width     = 1920;
    unsigned int height    = 1080;

    if (argc >= 2) outputFile = argv[1];
    if (argc >= 3) width  = static_cast<unsigned int>(std::stoul(argv[2]));
    if (argc >= 4) height = static_cast<unsigned int>(std::stoul(argv[3]));

    std::cout << "[INFO] Output: " << outputFile
              << "  Resolution: " << width << "x" << height << "\n";

    // ── Start CameraManager ──────────────────────────────────────────
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    int ret = cm->start();
    if (ret) {
        std::cerr << "[ERROR] Failed to start CameraManager (ret=" << ret << ").\n";
        return 1;
    }

    // ── List cameras ─────────────────────────────────────────────────
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        std::cerr << "[ERROR] No cameras detected.\n";
        cm->stop();
        return 1;
    }

    std::cout << "[INFO] Detected " << cameras.size() << " camera(s):\n";
    for (size_t i = 0; i < cameras.size(); ++i) {
        std::cout << "       [" << i << "] " << cameras[i]->id() << "\n";
    }

    // ── Acquire first camera ─────────────────────────────────────────
    std::shared_ptr<Camera> camera = cameras[0];
    ret = camera->acquire();
    if (ret) {
        std::cerr << "[ERROR] Failed to acquire camera (ret=" << ret << ").\n";
        cm->stop();
        return 1;
    }

    // ── Configure StillCapture stream ────────────────────────────────
    std::unique_ptr<CameraConfiguration> config =
        camera->generateConfiguration({StreamRole::StillCapture});
    if (!config) {
        std::cerr << "[ERROR] Failed to generate camera configuration.\n";
        camera->release();
        cm->stop();
        return 1;
    }

    StreamConfiguration &streamCfg = config->at(0);
    streamCfg.pixelFormat = formats::YUV420;
    streamCfg.size        = Size(width, height);

    CameraConfiguration::Status status = config->validate();
    if (status == CameraConfiguration::Invalid) {
        std::cerr << "[ERROR] Camera configuration is invalid.\n";
        camera->release();
        cm->stop();
        return 1;
    }
    if (status == CameraConfiguration::Adjusted) {
        std::cout << "[WARN] Configuration adjusted by the driver.\n";
        std::cout << "       Actual: " << streamCfg.pixelFormat << " "
                  << streamCfg.size.toString() << "\n";
    }

    ret = camera->configure(config.get());
    if (ret) {
        std::cerr << "[ERROR] Failed to configure camera (ret=" << ret << ").\n";
        camera->release();
        cm->stop();
        return 1;
    }

    std::cout << "[INFO] Stream configured: " << streamCfg.pixelFormat << " "
              << streamCfg.size.toString()
              << " stride=" << streamCfg.stride << "\n";

    // ── Allocate frame buffers ───────────────────────────────────────
    FrameBufferAllocator allocator(camera);
    Stream *stream = streamCfg.stream();

    ret = allocator.allocate(stream);
    if (ret < 0) {
        std::cerr << "[ERROR] Failed to allocate buffers (ret=" << ret << ").\n";
        camera->release();
        cm->stop();
        return 1;
    }
    std::cout << "[INFO] Allocated " << allocator.buffers(stream).size()
              << " buffer(s).\n";

    // ── Create and queue a single request ────────────────────────────
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers =
        allocator.buffers(stream);

    std::unique_ptr<Request> request = camera->createRequest();
    if (!request) {
        std::cerr << "[ERROR] Failed to create request.\n";
        allocator.free(stream);
        camera->release();
        cm->stop();
        return 1;
    }

    ret = request->addBuffer(stream, buffers[0].get());
    if (ret) {
        std::cerr << "[ERROR] Failed to add buffer to request (ret=" << ret << ").\n";
        allocator.free(stream);
        camera->release();
        cm->stop();
        return 1;
    }

    // ── Connect signal & start camera ────────────────────────────────
    camera->requestCompleted.connect(requestCompleted);

    ret = camera->start();
    if (ret) {
        std::cerr << "[ERROR] Failed to start camera (ret=" << ret << ").\n";
        allocator.free(stream);
        camera->release();
        cm->stop();
        return 1;
    }

    ret = camera->queueRequest(request.get());
    if (ret) {
        std::cerr << "[ERROR] Failed to queue request (ret=" << ret << ").\n";
        camera->stop();
        allocator.free(stream);
        camera->release();
        cm->stop();
        return 1;
    }

    // ── Event loop — pump until the request completes ────────────────
    std::cout << "[INFO] Waiting for capture...\n";
    while (!requestDone) {
        cm->processEvents();
    }

    // ── Read completed frame via mmap ────────────────────────────────
    FrameBuffer *fb = buffers[0].get();
    const FrameMetadata &meta = fb->metadata();

    std::cout << "[INFO] Frame captured — " << meta.planes().size()
              << " plane(s), sequence=" << meta.sequence
              << ", status=" << static_cast<int>(meta.status) << "\n";

    std::vector<uint8_t> frameData;

    for (size_t i = 0; i < fb->planes().size(); ++i) {
        const FrameBuffer::Plane &plane = fb->planes()[i];
        size_t planeLength = meta.planes()[i].bytesused;

        void *mapped = mmap(nullptr, planeLength, PROT_READ, MAP_SHARED,
                            plane.fd.get(), plane.offset);
        if (mapped == MAP_FAILED) {
            std::cerr << "[ERROR] mmap failed for plane " << i
                      << " (fd=" << plane.fd.get()
                      << ", len=" << planeLength << ").\n";
            camera->stop();
            allocator.free(stream);
            camera->release();
            cm->stop();
            return 1;
        }

        const uint8_t *src = static_cast<const uint8_t *>(mapped);
        frameData.insert(frameData.end(), src, src + planeLength);
        munmap(mapped, planeLength);
    }

    // ── Write to output file ─────────────────────────────────────────
    std::ofstream ofs(outputFile, std::ios::binary);
    if (!ofs) {
        std::cerr << "[ERROR] Cannot open output file: " << outputFile << "\n";
        camera->stop();
        allocator.free(stream);
        camera->release();
        cm->stop();
        return 1;
    }

    ofs.write(reinterpret_cast<const char *>(frameData.data()),
              static_cast<std::streamsize>(frameData.size()));
    ofs.close();

    std::cout << "[INFO] Wrote " << frameData.size() << " bytes to "
              << outputFile << "\n";

    // ── Print ffmpeg conversion command ──────────────────────────────
    unsigned int actualW = streamCfg.size.width;
    unsigned int actualH = streamCfg.size.height;

    std::cout << "\n[INFO] Convert to PNG with:\n"
              << "  ffmpeg -f rawvideo -pixel_format yuv420p"
              << " -video_size " << actualW << "x" << actualH
              << " -i " << outputFile
              << " -frames:v 1 output.png\n\n";

    // ── Clean up ─────────────────────────────────────────────────────
    camera->stop();
    allocator.free(stream);
    camera->release();
    cm->stop();

    std::cout << "[INFO] Done.\n";
    return 0;
}
