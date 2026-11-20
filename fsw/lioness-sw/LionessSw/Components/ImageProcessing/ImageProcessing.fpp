module Components {
    active component ImageProcessing {
        sync input port capture: [2] LionessSw.ImageCapture
        output port imageReady: [2] LionessSw.ImageReady
        match capture with imageReady
        internal port run

        event RequestRejected(requestId: U32, reason: LionessSw.ImageStatus) \
            severity warning low format "Image request {} rejected: {}"
        event CaptureFailed(requestId: U32, reason: LionessSw.ImageStatus) \
            severity warning high format "Image request {} failed: {}"
        event ImageReady(requestId: U32, bytes: U32) \
            severity activity low format "Image request {} ready: {} bytes"
        event ComponentFault(requestId: U32) \
            severity warning high format "Image request {} requires camera recovery"

        telemetry RequestsAccepted: U32
        telemetry RequestsRejected: U32
        telemetry QueueDepth: U32
        telemetry RequestsCompleted: U32
        telemetry WorkerState: LionessSw.ImageState
        telemetry CurrentRequest: U32
        telemetry CapturesFailed: U32
        telemetry LastDurationMs: U64
        telemetry LastError: LionessSw.ImageStatus

        time get port timeCaller
        import Fw.Event
        import Fw.Channel
    }
}
