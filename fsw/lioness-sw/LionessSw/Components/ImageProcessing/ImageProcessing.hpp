#ifndef Components_ImageProcessing_HPP
#define Components_ImageProcessing_HPP

#include "LionessSw/Components/ImageProcessing/ImageProcessingComponentAc.hpp"
#include "LionessSw/Ports/ProcessingStatusSerializableAc.hpp"
#include "FrameProcessor.hpp"
#include "CameraSource.hpp"
#include "Os/Mutex.hpp"

#include <array>
#include <chrono>

namespace Components {
class ImageProcessing final : public ImageProcessingComponentBase {
  public:
    explicit ImageProcessing(const char* name);

    // Configure before starting the task; clean up after stopping and joining it.
    bool configure(CameraSource& camera, U32 maxFrameBytes, U32 timeoutMs);
    LionessSw::ProcessingStatus getStatus();
    bool cleanup();

  private:
    static constexpr U32 QUEUE_CAPACITY = 2;
    using Clock = std::chrono::steady_clock;
    struct Job {
        LionessSw::CaptureRequest request;
        U32 client = 0;
        Clock::time_point submitted;
    };

    LionessSw::ImageStatus capture_handler(FwIndexType portNum, LionessSw::CaptureRequest& request) override;
    void run_internalInterfaceHandler() override;

    LionessSw::ImageStatus validate(const Job& job);
    LionessSw::ImageStatus conflicts(const Job& first, const Job& second) const;
    LionessSw::ImageResult process(const Job& job);
    U32 remainingMs(const Job& job) const;
    void publishStatus();

    Os::Mutex m_mutex;
    CameraSource* m_camera = nullptr;
    std::array<Job, QUEUE_CAPACITY> m_queue;
    Job m_current;
    U32 m_head = 0;
    U32 m_count = 0;
    U32 m_timeoutMs = 0;
    U32 m_maxFrameBytes = 0;
    bool m_ready = false;
    bool m_scheduled = false;
    bool m_active = false;
    bool m_usingBuffer = false;
    LionessSw::ProcessingStatus m_status;
    U64 m_lastDurationMs = 0;
};
}
#endif
