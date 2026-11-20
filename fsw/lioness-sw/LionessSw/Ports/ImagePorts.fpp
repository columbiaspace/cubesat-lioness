module LionessSw {
    port ImageCapture(ref $request: CaptureRequest) -> ImageStatus
    port ImageReady(ref result: ImageResult)
}
