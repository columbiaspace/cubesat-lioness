#include "ImageProcessing.hpp"

#include <cstdint>
#include <limits>

namespace Components {
using namespace LionessSw;

ImageProcessing::ImageProcessing(const char* name) : ImageProcessingComponentBase(name) {}

bool ImageProcessing::configure(CameraSource& camera, U32 maxFrameBytes, U32 timeoutMs) {
    Os::ScopeLock lock(m_mutex);
    if (maxFrameBytes == 0 || timeoutMs == 0 || m_ready) return false;
    m_camera = &camera;
    m_maxFrameBytes = maxFrameBytes;
    m_timeoutMs = timeoutMs;
    m_status = ProcessingStatus();
    m_status.set_state(ImageState::IDLE);
    m_lastDurationMs = 0;
    m_ready = true;
    return true;
}

bool ImageProcessing::cleanup() {
    Os::ScopeLock lock(m_mutex);
    if (!m_ready || m_scheduled) return false;
    m_ready = false;
    m_camera = nullptr;
    return true;
}

ImageStatus ImageProcessing::conflicts(const Job& first, const Job& second) const {
    if (first.client == second.client && first.request.get_requestId() == second.request.get_requestId()) {
        return ImageStatus::DUPLICATE_REQUEST;
    }
    return FrameProcessor::overlaps(first.request.get_destination(), second.request.get_destination())
               ? ImageStatus(ImageStatus::BUFFER_IN_USE) : ImageStatus(ImageStatus::ACCEPTED);
}

// Called under the admission lock; no camera access or pixel writes here.
ImageStatus ImageProcessing::validate(const Job& job) {
    const U32 camera = job.request.get_cameraId();
    if (camera >= CameraSource::CAMERA_COUNT) return ImageStatus::INVALID_REQUEST;
    if (!m_ready || !isConnected_imageReady_OutputPort(job.client)) {
        return ImageStatus::UNAVAILABLE;
    }
    if (m_status.get_state() == ImageState::FAULT) return ImageStatus::FAULT;
    if (job.request.get_encoding() != ImageEncoding::RAW) return ImageStatus::UNSUPPORTED_ENCODING;
    const auto& buffer = job.request.get_destination();
    const auto address = reinterpret_cast<std::uintptr_t>(buffer.getData());
    if (!buffer.isValid() || buffer.getSize() - 1 > std::numeric_limits<std::uintptr_t>::max() - address) {
        return ImageStatus::INVALID_BUFFER;
    }
    if (m_usingBuffer) {
        const auto conflict = conflicts(job, m_current);
        if (conflict != ImageStatus::ACCEPTED) return conflict;
    }
    for (U32 i = 0; i < m_count; ++i) {
        const auto conflict = conflicts(job, m_queue[(m_head + i) % QUEUE_CAPACITY]);
        if (conflict != ImageStatus::ACCEPTED) return conflict;
    }
    return m_count == QUEUE_CAPACITY ? ImageStatus(ImageStatus::QUEUE_FULL) : ImageStatus(ImageStatus::ACCEPTED);
}

ImageStatus ImageProcessing::capture_handler(FwIndexType portNum, CaptureRequest& request) {
    Job job;
    job.client = static_cast<U32>(portNum);
    job.request = request;
    ImageStatus admission;
    bool wake = false;
    bool reportRejection = false;
    {
        Os::ScopeLock lock(m_mutex);
        admission = validate(job);
        if (admission == ImageStatus::ACCEPTED) {
            job.submitted = Clock::now();
            m_queue[(m_head + m_count) % QUEUE_CAPACITY] = job;
            ++m_count;
            m_status.set_accepted(m_status.get_accepted() + 1);
            wake = !m_scheduled;
            m_scheduled = true;
        } else {
            reportRejection = m_status.get_rejected() < 10;
            m_status.set_rejected(m_status.get_rejected() + 1);
            m_status.set_lastError(admission);
        }
    }
    if (wake) run_internalInterfaceInvoke();
    if (reportRejection) log_WARNING_LO_RequestRejected(job.request.get_requestId(), admission);
    return admission;
}

U32 ImageProcessing::remainingMs(const Job& job) const {
    const auto age = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - job.submitted).count();
    return static_cast<U64>(age) >= m_timeoutMs ? 0 : m_timeoutMs - static_cast<U32>(age);
}

// Copy while the camera lease is valid, then release it before notifying the caller.
ImageResult ImageProcessing::process(const Job& job) {
    ImageResult result;
    result.set_cameraId(job.request.get_cameraId());
    result.set_requestId(job.request.get_requestId());
    result.set_encoding(ImageEncoding::RAW);
    {
        Os::ScopeLock lock(m_mutex);
        if (m_status.get_state() == ImageState::FAULT) {
            result.set_status(ImageStatus::FAULT);
            return result;
        }
    }
    const U32 remaining = remainingMs(job);
    if (remaining == 0) {
        result.set_status(ImageStatus::TIMEOUT);
        return result;
    }

    CameraFrame frame;
    auto outcome = m_camera->getFrame(job.request.get_cameraId(), remaining, frame);
    if (outcome == ImageStatus::OK) {
        {
            Os::ScopeLock lock(m_mutex);
            m_status.set_state(ImageState::PROCESSING);
        }
        auto destination = job.request.get_destination();
        if (destination.getSize() > m_maxFrameBytes) destination.setSize(m_maxFrameBytes);
        result.set_info(frame.get_info());
        outcome = remainingMs(job) == 0 ? ImageStatus(ImageStatus::TIMEOUT)
                                       : FrameProcessor::copy(frame.get_buffer(), frame.get_info(), destination);
        if (m_camera->releaseFrame(job.request.get_cameraId(), frame.get_lease()) != ImageStatus::OK) {
            outcome = ImageStatus::RELEASE_ERROR;
        }
    }
    if (outcome == ImageStatus::FAULT || outcome == ImageStatus::RELEASE_ERROR) {
        Os::ScopeLock lock(m_mutex);
        m_status.set_state(ImageState::FAULT);
    }
    if (outcome == ImageStatus::OK && remainingMs(job) == 0) outcome = ImageStatus::TIMEOUT;
    result.set_status(outcome);
    if (outcome == ImageStatus::OK) {
        result.set_bytesWritten(result.get_info().get_bytesUsed());
    } else {
        result.get_info().set_bytesUsed(0);
    }
    return result;
}

void ImageProcessing::run_internalInterfaceHandler() {
    Job job;
    {
        Os::ScopeLock lock(m_mutex);
        FW_ASSERT(m_count > 0);
        job = m_queue[m_head];
        m_head = (m_head + 1) % QUEUE_CAPACITY;
        --m_count;
        m_current = job;
        m_active = true;
        m_usingBuffer = true;
        m_status.set_currentRequest(job.request.get_requestId());
        if (m_status.get_state() != ImageState::FAULT) m_status.set_state(ImageState::CAPTURING);
    }

    publishStatus();
    auto result = process(job);
    {
        Os::ScopeLock lock(m_mutex);
        m_status.set_completed(m_status.get_completed() + 1);
        if (result.get_status() != ImageStatus::OK) m_status.set_capturesFailed(m_status.get_capturesFailed() + 1);
        m_status.set_lastError(result.get_status());
        m_lastDurationMs = static_cast<U64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - job.submitted).count());
        m_usingBuffer = false;
    }
    if (result.get_status() == ImageStatus::OK) {
        log_ACTIVITY_LO_ImageReady(job.request.get_requestId(), result.get_bytesWritten());
    } else {
        log_WARNING_HI_CaptureFailed(job.request.get_requestId(), result.get_status());
    }
    if (result.get_status() == ImageStatus::RELEASE_ERROR || result.get_status() == ImageStatus::FAULT) {
        log_WARNING_HI_ComponentFault(job.request.get_requestId());
    }
    imageReady_out(job.client, result);

    bool wake;
    {
        Os::ScopeLock lock(m_mutex);
        m_active = false;
        m_status.set_currentRequest(0);
        if (m_status.get_state() != ImageState::FAULT) m_status.set_state(ImageState::IDLE);
        wake = m_count != 0;
        m_scheduled = wake;
    }
    publishStatus();
    if (wake) run_internalInterfaceInvoke();
}

ProcessingStatus ImageProcessing::getStatus() {
    Os::ScopeLock lock(m_mutex);
    auto snapshot = m_status;
    snapshot.set_queueDepth(m_count);
    if (m_active || m_count != 0) {
        const auto& oldest = m_active ? m_current : m_queue[m_head];
        snapshot.set_ageMs(static_cast<U64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - oldest.submitted).count()));
        snapshot.set_currentRequest(oldest.request.get_requestId());
    }
    return snapshot;
}

// Only the worker publishes telemetry; no scheduler/status input ports are needed.
void ImageProcessing::publishStatus() {
    const auto snapshot = getStatus();
    U64 duration;
    {
        Os::ScopeLock lock(m_mutex);
        duration = m_lastDurationMs;
    }
    tlmWrite_RequestsAccepted(snapshot.get_accepted());
    tlmWrite_RequestsRejected(snapshot.get_rejected());
    tlmWrite_RequestsCompleted(snapshot.get_completed());
    tlmWrite_QueueDepth(snapshot.get_queueDepth());
    tlmWrite_WorkerState(snapshot.get_state());
    tlmWrite_CurrentRequest(snapshot.get_currentRequest());
    tlmWrite_CapturesFailed(snapshot.get_capturesFailed());
    tlmWrite_LastDurationMs(duration);
    tlmWrite_LastError(snapshot.get_lastError());
}
}
