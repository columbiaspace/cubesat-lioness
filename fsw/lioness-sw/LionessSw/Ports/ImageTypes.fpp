module LionessSw {
    enum ImageStatus {
        OK, ACCEPTED, INVALID_REQUEST, QUEUE_FULL, UNAVAILABLE,
        DUPLICATE_REQUEST, TIMEOUT, BAD_FRAME, BUFFER_TOO_SMALL,
        CAPTURE_ERROR, RELEASE_ERROR, INVALID_BUFFER, BUFFER_IN_USE,
        UNSUPPORTED_ENCODING, FAULT
    }

    enum PixelFormat { UNKNOWN, GRAY8, RGB8, YUV420, BAYER10_PACKED, BAYER16 }
    enum ImageEncoding { RAW, JPEG, LOSSLESS }
    enum ImageState { IDLE, CAPTURING, PROCESSING, FAULT }

    struct ImagePlane {
        offset: U32
        stride: U32
        rowBytes: U32
        rows: U32
    }

    struct FrameInfo {
        width: U32
        height: U32
        $format: PixelFormat
        bitDepth: U8
        planeCount: U8
        planes: [3] ImagePlane
        bytesUsed: U32
        timestamp: Fw.Time
    }

    struct CaptureRequest {
        cameraId: U32
        requestId: U32
        encoding: ImageEncoding
        destination: Fw.Buffer
    }

    struct CameraFrame {
        buffer: Fw.Buffer
        info: FrameInfo
        lease: U32
    }

    struct ImageResult {
        cameraId: U32
        requestId: U32
        status: ImageStatus
        encoding: ImageEncoding
        bytesWritten: U32
        info: FrameInfo
    }

    struct ProcessingStatus {
        $state: ImageState
        currentRequest: U32
        ageMs: U64
        queueDepth: U32
        accepted: U32
        rejected: U32
        completed: U32
        capturesFailed: U32
        lastError: ImageStatus
    }
}
