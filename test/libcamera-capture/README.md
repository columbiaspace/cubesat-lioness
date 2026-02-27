# libcamera Standalone Capture

> **Phase 2** of the F Prime camera integration — standalone testing of the
> MIPI CSI camera via the libcamera C++ API before wrapping in an F Prime
> component.

Captures a single still frame in YUV420 format from the first detected CSI
camera and writes the raw bytes to disk.

## Prerequisites

### Hardware
- Raspberry Pi 4/5 with a MIPI CSI camera module connected and enabled.
- Verify the camera is detected by the kernel:
  ```
  vcgencmd get_camera        # legacy
  libcamera-hello --list-cameras
  ```

### Smoke test
Run the stock Raspberry Pi camera tools first to confirm the driver stack is
working:
```bash
rpicam-hello -t 2000          # live preview for 2 s
rpicam-still -o test.jpg      # capture a JPEG
```
If these fail, fix hardware/driver issues before continuing.

### Packages
```bash
sudo apt install libcamera-dev cmake g++
```

## Build

```bash
cd test/libcamera-capture
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Run

```bash
# defaults: frame.raw, 1920x1080
./capture

# custom output and resolution
./capture photo.raw 1280 720
```

The program will:
1. Start the libcamera CameraManager
2. List all detected cameras
3. Acquire the first camera
4. Configure a StillCapture stream (YUV420)
5. Capture a single frame
6. Write the raw bytes to the output file
7. Print the ffmpeg conversion command

## Converting to PNG

After capture, convert the raw YUV420 file to a viewable image:

```bash
ffmpeg -f rawvideo -pixel_format yuv420p \
       -video_size 1920x1080 \
       -i frame.raw \
       -frames:v 1 output.png
```

Adjust `-video_size` to match the actual resolution printed by the program
(especially if the driver adjusted it).

## Troubleshooting

### No cameras detected
- Make sure the ribbon cable is seated properly on both ends.
- Check `libcamera-hello --list-cameras` — if it also shows nothing, the
  issue is at the driver/hardware level, not in this program.
- On Pi 5, ensure you are using the correct CSI connector.
- Confirm the camera overlay is enabled in `/boot/firmware/config.txt`
  (e.g., `dtoverlay=imx219` for Camera Module v2).

### Configuration adjusted by the driver
The program prints a `[WARN]` if the driver changed the requested format or
resolution.  This is normal when the sensor doesn't support the exact
parameters.  The actual values are printed; use those values with ffmpeg.

### mmap errors
- Ensure you are running on the Raspberry Pi (not cross-compiled without
  matching kernel headers).
- Some older libcamera versions may return different `fd`/`offset` layouts.
  Update libcamera: `sudo apt update && sudo apt upgrade libcamera-dev`.

### Garbled / green image
- Double-check the resolution and pixel format passed to ffmpeg match what
  the program printed.
- The stride (row alignment) may differ from `width`.  If the image looks
  skewed, try using the stride value printed by the program as the width
  in the ffmpeg command.
- Make sure you are using `yuv420p` (planar), not `nv12` or `nv21`.
