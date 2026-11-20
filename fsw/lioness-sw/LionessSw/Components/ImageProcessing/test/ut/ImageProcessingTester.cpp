#include "ImageProcessingTester.hpp"

#include <algorithm>
#include <chrono>

namespace Components {
using namespace LionessSw;

ImageProcessingTester::ImageProcessingTester() : ImageProcessingGTestBase("Tester", 100), component("images") {
    for (auto& output : buffers) output.resize(64, 165);
    initComponents();
    connectPorts();
    frameInfo.set_width(2);
    frameInfo.set_height(2);
    frameInfo.set_format(PixelFormat::RGB8);
    frameInfo.set_bitDepth(8);
    frameInfo.set_planeCount(1);
    frameInfo.get_planes()[0] = ImagePlane(0, 8, 6, 2);
    frameInfo.set_bytesUsed(16);
    frameInfo.set_timestamp(Fw::Time(123, 456));
}

ImageProcessingTester::~ImageProcessingTester() {
    while (snapshot().get_queueDepth() != 0) step();
    component.cleanup();
    component.deinit();
}

bool ImageProcessingTester::configure(U32 timeoutMs, U32 maxFrameBytes) {
    return component.configure(*this, maxFrameBytes, timeoutMs);
}

Fw::Buffer ImageProcessingTester::buffer(U32 index) {
    return Fw::Buffer(buffers[index].data(), buffers[index].size(), index);
}

void ImageProcessingTester::setImage(U32 width, U32 height, const std::vector<U8>& pixels) {
    m_image = pixels;
    for (auto& output : buffers) output.resize(pixels.size());
    frameInfo.set_width(width);
    frameInfo.set_height(height);
    frameInfo.get_planes()[0] = ImagePlane(0, width * 3, width * 3, height);
    frameInfo.set_bytesUsed(static_cast<U32>(pixels.size()));
}

ImageStatus ImageProcessingTester::capture(U32 id, U32 bufferIndex, U32 client, U32 camera) {
    CaptureRequest request(camera, id, ImageEncoding::RAW, buffer(bufferIndex));
    return submit(request, client);
}

ImageStatus ImageProcessingTester::submit(CaptureRequest& request, U32 client) {
    return invoke_to_capture(client, request);
}

ProcessingStatus ImageProcessingTester::snapshot() {
    return component.getStatus();
}

void ImageProcessingTester::step() { dispatchOne(component); }

ImageStatus ImageProcessingTester::getFrame(U32 cameraId, U32 timeoutMs,
                                                             CameraFrame& frame) {
    ++captures;
    cameraThread = std::this_thread::get_id();
    EXPECT_GT(timeoutMs, 0u);
    EXPECT_LT(cameraId, static_cast<U32>(CameraSource::CAMERA_COUNT));
    {
        std::unique_lock<std::mutex> lock(m_gate);
        m_cameraEntered = true;
        m_condition.notify_all();
        if (blockCamera) m_condition.wait(lock, [this] { return m_continue; });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(captureDelayMs));
    if (cameraStatus != ImageStatus::OK) return cameraStatus;
    source = {{255, 0, 0, 0, 255, 0, 165, 165, 0, 0, 255, 255, 255, 255, 165, 165}};
    if (m_image.empty()) {
        frame.set_buffer(Fw::Buffer(source.data(), source.size()));
    } else {
        m_cameraImage = m_image;
        frame.set_buffer(Fw::Buffer(m_cameraImage.data(), m_cameraImage.size()));
    }
    frame.set_info(frameInfo);
    frame.set_lease(7);
    return ImageStatus::OK;
}

ImageStatus ImageProcessingTester::releaseFrame(U32 cameraId, U32 lease) {
    EXPECT_EQ(lease, 7u);
    ++cameraReturns;
    source.fill(238);
    std::fill(m_cameraImage.begin(), m_cameraImage.end(), 238);
    return releaseStatus;
}

void ImageProcessingTester::from_imageReady_handler(FwIndexType portNum, ImageResult& result) {
    std::lock_guard<std::mutex> lock(m_gate);
    results.push_back(result);
    clients.push_back(static_cast<U32>(portNum));
    if (submitFromCallback && results.size() == 1) {
        EXPECT_EQ(capture(99, static_cast<U32>(portNum), static_cast<U32>(portNum)), ImageStatus::ACCEPTED);
    }
    m_condition.notify_all();
}

void ImageProcessingTester::waitForCamera() {
    std::unique_lock<std::mutex> lock(m_gate);
    m_condition.wait(lock, [this] { return m_cameraEntered; });
}

void ImageProcessingTester::unblockCamera() {
    std::lock_guard<std::mutex> lock(m_gate);
    m_continue = true;
    m_condition.notify_all();
}

}
