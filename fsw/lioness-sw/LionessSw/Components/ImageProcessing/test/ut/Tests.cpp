#include "ImageProcessingTester.hpp"
#include "Os/Os.hpp"

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdint>
#include <limits>

using namespace LionessSw;
using Components::ImageProcessingTester;

static std::vector<U8> expectedPixels() {
    return {255, 0, 0, 0, 255, 0, 165, 165, 0, 0, 255, 255, 255, 255, 165, 165};
}

TEST(ImageProcessing, WritesCallerBufferAndReportsLength) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    const auto descriptor = test.buffer();
    ASSERT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    EXPECT_TRUE(test.results.empty());
    EXPECT_EQ(test.captures, 0u);
    EXPECT_FALSE(test.component.cleanup());
    test.step();
    ASSERT_EQ(test.results.size(), 1u);
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::OK);
    EXPECT_EQ(test.results[0].get_bytesWritten(), 16u);
    EXPECT_EQ(test.results[0].get_encoding(), ImageEncoding::RAW);
    EXPECT_EQ(test.results[0].get_info(), test.frameInfo);
    EXPECT_EQ(test.buffer(), descriptor);
    const auto expected = expectedPixels();
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), test.buffers[0].begin()));
    EXPECT_TRUE(std::all_of(test.buffers[0].begin() + 16, test.buffers[0].end(), [](U8 b) { return b == 165; }));
    EXPECT_EQ(test.cameraReturns, 1u);
    EXPECT_EQ(test.source[0], 238u);
    EXPECT_EQ(test.snapshot().get_completed(), 1u);
}

TEST(ImageProcessing, InvalidRequestsDoNotTouchCallerMemory) {
    ImageProcessingTester test;
    const auto original = test.buffers[0];
    EXPECT_EQ(test.capture(1), ImageStatus::UNAVAILABLE);
    EXPECT_FALSE(test.configure(0));
    EXPECT_FALSE(test.configure(1000, 0));
    ASSERT_TRUE(test.configure());
    EXPECT_FALSE(test.configure());
    EXPECT_EQ(test.capture(2, 0, 0, 10), ImageStatus::INVALID_REQUEST);
    CaptureRequest request(0, 3, ImageEncoding::RAW, Fw::Buffer());
    EXPECT_EQ(test.submit(request), ImageStatus::INVALID_BUFFER);
    request.set_destination(Fw::Buffer(test.buffers[0].data(), 0));
    EXPECT_EQ(test.submit(request), ImageStatus::INVALID_BUFFER);
    request.set_destination(Fw::Buffer(reinterpret_cast<U8*>(std::numeric_limits<std::uintptr_t>::max() - 1), 16));
    EXPECT_EQ(test.submit(request), ImageStatus::INVALID_BUFFER);
    EXPECT_EQ(test.buffers[0], original);
    EXPECT_EQ(test.captures, 0u);
    EXPECT_TRUE(test.results.empty());
}

TEST(ImageProcessing, FutureEncodingsRejectBeforeCapture) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    const auto original = test.buffers[0];
    CaptureRequest request(0, 1, ImageEncoding::JPEG, test.buffer());
    EXPECT_EQ(test.submit(request), ImageStatus::UNSUPPORTED_ENCODING);
    request.set_encoding(ImageEncoding::LOSSLESS);
    EXPECT_EQ(test.submit(request), ImageStatus::UNSUPPORTED_ENCODING);
    EXPECT_EQ(test.buffers[0], original);
    EXPECT_EQ(test.captures, 0u);
    EXPECT_TRUE(test.results.empty());
}

TEST(ImageProcessing, QueueRejectionLeavesDestinationUntouched) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    const auto untouched = test.buffers[2];
    EXPECT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    EXPECT_EQ(test.capture(2, 1), ImageStatus::ACCEPTED);
    EXPECT_EQ(test.capture(3, 2), ImageStatus::QUEUE_FULL);
    EXPECT_EQ(test.buffers[2], untouched);
    test.step();
    test.step();
    ASSERT_EQ(test.results.size(), 2u);
    EXPECT_EQ(test.snapshot().get_rejected(), 1u);
    EXPECT_EQ(test.capture(3, 2), ImageStatus::ACCEPTED);
    test.step();
    EXPECT_EQ(test.results.back().get_requestId(), 3u);
}

TEST(ImageProcessing, DuplicateIdsAndOverlappingDestinationsReject) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    ASSERT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    EXPECT_EQ(test.capture(1, 1), ImageStatus::DUPLICATE_REQUEST);
    EXPECT_EQ(test.capture(2), ImageStatus::BUFFER_IN_USE);
    CaptureRequest overlap(0, 3, ImageEncoding::RAW, Fw::Buffer(test.buffers[0].data() + 8, 16));
    EXPECT_EQ(test.submit(overlap, 1), ImageStatus::BUFFER_IN_USE);
    test.step();
    EXPECT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    test.step();
    EXPECT_EQ(test.results.size(), 2u);
}

TEST(ImageProcessing, DifferentClientsCanUseSameRequestId) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    EXPECT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    EXPECT_EQ(test.capture(1, 1, 1, 1), ImageStatus::ACCEPTED);
    test.step();
    test.step();
    ASSERT_EQ(test.results.size(), 2u);
    EXPECT_EQ(test.clients, (std::vector<U32>{0, 1}));
    EXPECT_EQ(test.results[1].get_cameraId(), 1u);
}

TEST(ImageProcessing, RequestDescriptorIsCopiedAtAdmission) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    CaptureRequest request(0, 1, ImageEncoding::RAW, test.buffer());
    ASSERT_EQ(test.submit(request), ImageStatus::ACCEPTED);
    request.set_requestId(99);
    request.set_destination(test.buffer(1));
    test.step();
    EXPECT_EQ(test.results[0].get_requestId(), 1u);
    EXPECT_EQ(test.buffers[0][0], 255u);
    EXPECT_EQ(test.buffers[1][0], 165u);
}

TEST(ImageProcessing, CallerCanFreeBufferOnCompletion) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    ASSERT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    test.step();
    test.buffers[0].clear();
    test.buffers[0].shrink_to_fit();
    ASSERT_EQ(test.capture(2, 1), ImageStatus::ACCEPTED);
    test.step();
    EXPECT_EQ(test.results.back().get_status(), ImageStatus::OK);
    EXPECT_TRUE(test.component.cleanup());
}

TEST(ImageProcessing, DestinationCapacityAndFrameLimitAreEnforced) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure(1000, 8));
    const auto original = test.buffers[0];
    ASSERT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    test.step();
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::BUFFER_TOO_SMALL);
    EXPECT_EQ(test.results[0].get_bytesWritten(), 0u);
    EXPECT_EQ(test.buffers[0], original);
    EXPECT_EQ(test.cameraReturns, 1u);
    ASSERT_TRUE(test.component.cleanup());
    ASSERT_TRUE(test.configure());
    CaptureRequest small(0, 2, ImageEncoding::RAW, Fw::Buffer(test.buffers[0].data(), 4));
    ASSERT_EQ(test.submit(small), ImageStatus::ACCEPTED);
    test.step();
    EXPECT_EQ(test.results[1].get_status(), ImageStatus::BUFFER_TOO_SMALL);
    EXPECT_EQ(test.buffers[0], original);
}

TEST(ImageProcessing, CameraFailureDoesNotBlockNextRequest) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    const auto original = test.buffers[0];
    test.capture(1);
    test.capture(2, 1);
    test.cameraStatus = ImageStatus::TIMEOUT;
    test.step();
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::TIMEOUT);
    EXPECT_EQ(test.results[0].get_bytesWritten(), 0u);
    EXPECT_EQ(test.buffers[0], original);
    EXPECT_EQ(test.cameraReturns, 0u);
    test.cameraStatus = ImageStatus::OK;
    test.step();
    EXPECT_EQ(test.results[1].get_status(), ImageStatus::OK);
    EXPECT_EQ(test.snapshot().get_capturesFailed(), 1u);
    EXPECT_EQ(test.capture(3), ImageStatus::ACCEPTED);
    test.step();
    EXPECT_EQ(test.results.back().get_status(), ImageStatus::OK);
}

TEST(ImageProcessing, MalformedFrameIsReleasedWithoutWriting) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    const auto original = test.buffers[0];
    test.frameInfo.set_bytesUsed(100);
    test.capture(1);
    test.step();
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::BAD_FRAME);
    EXPECT_EQ(test.results[0].get_info().get_bytesUsed(), 0u);
    EXPECT_EQ(test.buffers[0], original);
    EXPECT_EQ(test.cameraReturns, 1u);
    test.frameInfo.set_bytesUsed(16);
    test.capture(2);
    test.step();
    EXPECT_EQ(test.results[1].get_status(), ImageStatus::OK);
}

TEST(ImageProcessing, QueuedDeadlineExpiresBeforeCameraCall) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure(1));
    const auto original = test.buffers[0];
    test.capture(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    test.step();
    EXPECT_EQ(test.captures, 0u);
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::TIMEOUT);
    EXPECT_EQ(test.buffers[0], original);
}

TEST(ImageProcessing, LateFrameIsReleased) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure(5));
    const auto original = test.buffers[0];
    test.captureDelayMs = 20;
    test.capture(1);
    test.step();
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::TIMEOUT);
    EXPECT_EQ(test.results[0].get_bytesWritten(), 0u);
    EXPECT_EQ(test.buffers[0], original);
    EXPECT_EQ(test.cameraReturns, 1u);
}

TEST(ImageProcessing, ReleaseFailureLatchesFault) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    test.releaseStatus = ImageStatus::CAPTURE_ERROR;
    test.capture(1);
    test.capture(2, 1);
    test.step();
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::RELEASE_ERROR);
    EXPECT_EQ(test.results[0].get_bytesWritten(), 0u);
    EXPECT_EQ(test.snapshot().get_state(), ImageState::FAULT);
    EXPECT_EQ(test.capture(3, 2), ImageStatus::FAULT);
    test.step();
    EXPECT_EQ(test.results[1].get_status(), ImageStatus::FAULT);
    EXPECT_EQ(test.captures, 1u);
}

TEST(ImageProcessing, StatusAndAdmissionRemainAvailableDuringCapture) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure(5000));
    test.blockCamera = true;
    test.capture(1);
    std::thread worker([&] { test.step(); });
    test.waitForCamera();
    EXPECT_EQ(test.snapshot().get_state(), ImageState::CAPTURING);
    EXPECT_EQ(test.snapshot().get_currentRequest(), 1u);
    EXPECT_EQ(test.capture(2), ImageStatus::BUFFER_IN_USE);
    EXPECT_EQ(test.capture(2, 1, 1), ImageStatus::ACCEPTED);
    EXPECT_EQ(test.capture(3, 2), ImageStatus::ACCEPTED);
    EXPECT_EQ(test.snapshot().get_queueDepth(), 2u);
    EXPECT_EQ(test.capture(4, 3), ImageStatus::QUEUE_FULL);
    EXPECT_FALSE(test.component.cleanup());
    test.unblockCamera();
    worker.join();
    test.step();
    test.step();
    EXPECT_EQ(test.results.size(), 3u);
}

TEST(ImageProcessing, ConcurrentAdmissionAndCallbackReuse) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    ImageStatus first, second;
    std::thread a([&] { first = test.capture(1); });
    std::thread b([&] { second = test.capture(2, 1, 1); });
    a.join();
    b.join();
    EXPECT_EQ(first, ImageStatus::ACCEPTED);
    EXPECT_EQ(second, ImageStatus::ACCEPTED);
    test.submitFromCallback = true;
    test.step();
    test.step();
    test.step();
    EXPECT_EQ(test.results.size(), 3u);
    EXPECT_EQ(test.results.back().get_requestId(), 99u);
    EXPECT_EQ(test.snapshot().get_queueDepth(), 0u);
}

TEST(ImageProcessing, WorkerDispatchRunsOffCallerThread) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure(5000));
    ASSERT_EQ(test.capture(1), ImageStatus::ACCEPTED);
    std::thread worker([&] { test.step(); });
    worker.join();
    EXPECT_NE(test.cameraThread, std::this_thread::get_id());
    ASSERT_EQ(test.results.size(), 1u);
    EXPECT_EQ(test.results[0].get_status(), ImageStatus::OK);
}

TEST(ImageProcessing, RepeatedCapturesReuseCallerBuffer) {
    ImageProcessingTester test;
    ASSERT_TRUE(test.configure());
    for (U32 i = 0; i < 50; ++i) {
        ASSERT_EQ(test.capture(i), ImageStatus::ACCEPTED);
        test.step();
        ASSERT_EQ(test.results.back().get_status(), ImageStatus::OK);
    }
    EXPECT_EQ(test.snapshot().get_accepted(), 50u);
    EXPECT_EQ(test.snapshot().get_completed(), 50u);
    EXPECT_TRUE(test.component.cleanup());
    EXPECT_EQ(test.capture(51), ImageStatus::UNAVAILABLE);
}

TEST(FrameProcessor, BoundsAndOutputCapacity) {
    ImageProcessingTester test;
    std::array<U8, 16> source = {}, output = {};
    Fw::Buffer input(source.data(), source.size());
    Fw::Buffer destination(output.data(), output.size());
    EXPECT_EQ(Components::FrameProcessor::copy(input, test.frameInfo, destination), ImageStatus::OK);
    destination.setSize(4);
    EXPECT_EQ(Components::FrameProcessor::copy(input, test.frameInfo, destination), ImageStatus::BUFFER_TOO_SMALL);
    EXPECT_EQ(Components::FrameProcessor::copy(input, test.frameInfo, input), ImageStatus::BAD_FRAME);
    test.frameInfo.get_planes()[0].set_offset(UINT_MAX);
    EXPECT_EQ(Components::FrameProcessor::validate(input, test.frameInfo), ImageStatus::BAD_FRAME);
    test.frameInfo.get_planes()[0] = ImagePlane(0, UINT_MAX, 6, 2);
    EXPECT_EQ(Components::FrameProcessor::validate(input, test.frameInfo), ImageStatus::BAD_FRAME);
}

TEST(FrameProcessor, NativeFormatsAndPlaneOverlap) {
    std::array<U8, 16> pixels = {};
    Fw::Buffer source(pixels.data(), pixels.size());
    FrameInfo info;
    info.set_width(4);
    info.set_height(2);
    info.set_format(PixelFormat::YUV420);
    info.set_bitDepth(8);
    info.set_planeCount(3);
    info.set_bytesUsed(12);
    info.get_planes()[0] = ImagePlane(0, 4, 4, 2);
    info.get_planes()[1] = ImagePlane(8, 2, 2, 1);
    info.get_planes()[2] = ImagePlane(10, 2, 2, 1);
    EXPECT_EQ(Components::FrameProcessor::validate(source, info), ImageStatus::OK);
    info.get_planes()[2].set_offset(9);
    EXPECT_EQ(Components::FrameProcessor::validate(source, info), ImageStatus::BAD_FRAME);
    info.set_format(PixelFormat::BAYER10_PACKED);
    info.set_bitDepth(10);
    info.set_planeCount(1);
    info.set_bytesUsed(10);
    info.get_planes()[0] = ImagePlane(0, 5, 5, 2);
    EXPECT_EQ(Components::FrameProcessor::validate(source, info), ImageStatus::OK);
    info.set_format(PixelFormat::BAYER16);
    info.set_bytesUsed(16);
    info.get_planes()[0] = ImagePlane(0, 8, 8, 2);
    EXPECT_EQ(Components::FrameProcessor::validate(source, info), ImageStatus::OK);
    info.set_format(PixelFormat::UNKNOWN);
    EXPECT_EQ(Components::FrameProcessor::validate(source, info), ImageStatus::BAD_FRAME);
}

int main(int argc, char** argv) {
    Os::init();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
