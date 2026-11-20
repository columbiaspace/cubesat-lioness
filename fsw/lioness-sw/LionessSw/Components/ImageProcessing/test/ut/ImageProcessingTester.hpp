#ifndef Components_ImageProcessingTester_HPP
#define Components_ImageProcessingTester_HPP

#include "ImageProcessingGTestBase.hpp"
#include "LionessSw/Components/ImageProcessing/ImageProcessing.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace Components {
class ImageProcessingTester : public ImageProcessingGTestBase, public CameraSource {
  public:
    enum { TEST_INSTANCE_ID = 0, TEST_INSTANCE_QUEUE_DEPTH = 2 };
    ImageProcessingTester();
    ~ImageProcessingTester();

    bool configure(U32 timeoutMs = 1000, U32 maxFrameBytes = 64);
    Fw::Buffer buffer(U32 index = 0);
    LionessSw::ImageStatus capture(U32 id, U32 bufferIndex = 0, U32 client = 0, U32 camera = 0);
    LionessSw::ImageStatus submit(LionessSw::CaptureRequest& request, U32 client = 0);
    LionessSw::ProcessingStatus snapshot();
    void step();
    void waitForCamera();
    void unblockCamera();
    void setImage(U32 width, U32 height, const std::vector<U8>& pixels);

    ImageProcessing component;
    LionessSw::FrameInfo frameInfo;
    LionessSw::ImageStatus cameraStatus = LionessSw::ImageStatus::OK;
    LionessSw::ImageStatus releaseStatus = LionessSw::ImageStatus::OK;
    std::vector<LionessSw::ImageResult> results;
    std::vector<U32> clients;
    std::array<std::vector<U8>, 4> buffers;
    std::array<U8, 16> source;
    U32 captures = 0;
    U32 cameraReturns = 0;
    U32 captureDelayMs = 0;
    bool blockCamera = false;
    bool submitFromCallback = false;
    std::thread::id cameraThread;

  private:
    void connectPorts();
    void initComponents();
    LionessSw::ImageStatus getFrame(U32 cameraId, U32 timeoutMs,
                                                     LionessSw::CameraFrame& frame) override;
    LionessSw::ImageStatus releaseFrame(U32 cameraId, U32 lease) override;
    void from_imageReady_handler(FwIndexType portNum, LionessSw::ImageResult& result) override;

    std::vector<U8> m_image;
    std::vector<U8> m_cameraImage;
    std::mutex m_gate;
    std::condition_variable m_condition;
    bool m_cameraEntered = false;
    bool m_continue = false;
};
}
#endif
