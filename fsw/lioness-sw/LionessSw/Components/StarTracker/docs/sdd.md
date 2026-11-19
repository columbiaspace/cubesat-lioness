# StarTracker software design

StarTracker turns a grayscale image of stars into an estimate of the camera's orientation. The attitude determination and control system (ADCS) uses that estimate with the camera mounting angle to determine spacecraft attitude.

This document describes the planned integration. The [F Prime component](../StarTracker.fpp) is currently a scaffold, and the working algorithm is in the [standalone prototype](../../../../../../test/star-tracker/README.md).

## Component flow

1. **Validate the frame.** Check the buffer, image dimensions, byte count, and tracker readiness before reading pixels.
2. **Find star centers.** Group neighboring bright pixels, reject small and edge-touching regions, and use pixel brightness to find each center to a fraction of a pixel. Start with the prototype's threshold and five-pixel minimum for comparison with its sample result.
3. **Match stars.** Convert each center to a direction using the camera focal length. Pyramid compares the angles between observed directions with angles in the catalog and looks for a unique pattern of at least four stars.
4. **Solve attitude.** QUEST fits a rotation between the catalog and observed directions. Check that the quaternion is finite, has unit length, and agrees with the matched directions.
5. **Report the result.** Send the frame ID, capture time, match count, and solve status. The consumer uses the quaternion only on success.

## F Prime interface

| Interface | Planned behavior |
| --- | --- |
| `imageIn` | Receives a borrowed `Fw.Buffer`, width, height, frame ID, and capture time. Accepts packed 8-bit grayscale with one byte per pixel and no row padding. |
| `attitudeOut` | Sends one result per frame when connected. Status indicates success, invalid input, tracker not ready, too few candidates, no match, or an invalid attitude. |
| Parameters | Focal length in pixels and match tolerance in degrees. Use the prototype's remaining search settings for the first comparison. |
| Command | `RESET_COUNTERS` clears solve and failure counts. |
| Events and telemetry | An event reports catalog initialization failure. Telemetry reports successful and failed frame counts. |

StarTracker is passive. It processes the frame and sends the result before returning to the caller. The caller owns the image and keeps it valid for the duration of the call. The consumer uses the result's capture time to associate it with the original observation.

The quaternion uses `(w, x, y, z)` and rotates catalog directions into camera coordinates. Verify that direction with a known rotation. An identity quaternion can be a valid result, so the solver must report failure explicitly.

## Plan for `StarTracker.cpp`

| Planned function | Responsibility |
| --- | --- |
| `initializeCatalog()` | During startup, validate the supplied catalog bytes and pair index. Keep the data for subsequent frames and mark the tracker ready after a successful load. |
| `imageIn_handler()` | Validate the frame and settings, call centroiding, Pyramid, and QUEST in order, and pass the outcome to `publishResult()`. Return a failure result as soon as a stage cannot proceed. |
| `publishResult()` | Record the outcome in telemetry and send it through `attitudeOut` if connected. Keep the original frame ID and capture time on both paths. |
| `RESET_COUNTERS_cmdHandler()` | Clear the counters and acknowledge the command. |

Declare the ports and result type in [StarTracker.fpp](../StarTracker.fpp) and keep the catalog state and counters in [StarTracker.hpp](../StarTracker.hpp). Reuse [star_id.cpp](../../../../../../test/star-tracker/star_id.cpp) and [star_id.hpp](../../../../../../test/star-tracker/star_id.hpp). Extract the pixel-only centroid routine from [star_tracker_test.cpp](../../../../../../test/star-tracker/star_tracker_test.cpp) for shared use. Keep image file decoding in the prototype and register the shared source in the existing [component CMake file](../CMakeLists.txt).

## Verification and remaining decisions

- Run the included `image.png` and `database.dat` through the component and compare with the prototype. The sample baseline is 23 identified stars.
- Use a synthetic image with a known orientation to check the quaternion. Verify that malformed frames, blank images, unmatched patterns, and invalid solves report failure.
- Confirm the camera format and capture timestamp with the camera team, and confirm the mounting rotation with ADCS. Before flight, replace recursive region traversal, bound memory and processing time on the target, calibrate the camera, and test difficult scenes such as Earth in view.
