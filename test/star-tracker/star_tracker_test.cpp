#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct RawFrame {
    std::vector<uint8_t> data; // Y plane, then U plane, then V plane
    int width = 0;
    int height = 0;
};

RawFrame load_yuv420_from_png(const std::string &inputFile) {
    int width, height, channels;
    unsigned char *rgb = stbi_load(inputFile.c_str(), &width, &height, &channels, 3);
    if (!rgb) {
        std::cerr << "[ERROR] Failed to load PNG: " << inputFile << "\n";
        std::exit(1);
    }

    std::cout << "[INFO] Loaded PNG: " << width << "x" << height << "\n";

    const int ySize = width * height;
    const int uvWidth = (width + 1) / 2;
    const int uvHeight = (height + 1) / 2;
    const int uvSize = uvWidth * uvHeight;

    RawFrame frame;
    frame.width = width;
    frame.height = height;
    frame.data.resize(ySize + 2 * uvSize);

    uint8_t *yPlane = frame.data.data();
    uint8_t *uPlane = yPlane + ySize;
    uint8_t *vPlane = uPlane + uvSize;

    // Convert RGB to Y plane
    for (int i = 0; i < ySize; i++) {
        const uint8_t r = rgb[i * 3 + 0];
        const uint8_t g = rgb[i * 3 + 1];
        const uint8_t b = rgb[i * 3 + 2];
        const int y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
        yPlane[i] = static_cast<uint8_t>(std::clamp(y, 0, 255));
    }

    // Convert RGB to subsampled U and V planes (2x2 averaging)
    for (int j = 0; j < uvHeight; j++) {
        for (int i = 0; i < uvWidth; i++) {
            int rSum = 0, gSum = 0, bSum = 0;
            int count = 0;
            for (int dy = 0; dy < 2 && j * 2 + dy < height; dy++) {
                for (int dx = 0; dx < 2 && i * 2 + dx < width; dx++) {
                    int px = (j * 2 + dy) * width + (i * 2 + dx);
                    rSum += rgb[px * 3 + 0];
                    gSum += rgb[px * 3 + 1];
                    bSum += rgb[px * 3 + 2];
                    count++;
                }
            }
            const int r = rSum / count;
            const int g = gSum / count;
            const int b = bSum / count;

            const int u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            const int v = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

            uPlane[j * uvWidth + i] = static_cast<uint8_t>(std::clamp(u, 0, 255));
            vPlane[j * uvWidth + i] = static_cast<uint8_t>(std::clamp(v, 0, 255));
        }
    }

    stbi_image_free(rgb);
    return frame;
}

RawFrame load_yuv420(const std::string &inputFile, int width, int height) {
    std::ifstream ifs(inputFile, std::ios::binary);
    if (!ifs) {
        std::cerr << "[ERROR] Failed to open raw file: " << inputFile << "\n";
        std::exit(1);
    }

    RawFrame frame;
    frame.width = width;
    frame.height = height;

    int ySize = width * height;
    int uvSize = ((width + 1) / 2) * ((height + 1) / 2);
    frame.data.resize(ySize + 2 * uvSize);

    ifs.read(reinterpret_cast<char *>(frame.data.data()),
             static_cast<std::streamsize>(frame.data.size()));
    if (!ifs) {
        std::cerr << "[ERROR] Failed to read " << frame.data.size()
                  << " bytes from: " << inputFile << "\n";
        std::exit(1);
    }
    std::cout << "[INFO] Loaded raw YUV420: " << width << "x" << height << "\n";
    return frame;
}

int main(int argc, char *argv[]) {
    std::string inputFile = "image.png";

    if (argc >= 2) {
        inputFile = argv[1];
    }

    RawFrame frame;
    if (inputFile.ends_with(".png")) {
        frame = load_yuv420_from_png(inputFile);
    } else if (inputFile.ends_with(".raw")) {
        int width = 1920, height = 1080;
        if (argc >= 3) width = std::stoi(argv[2]);
        if (argc >= 4) height = std::stoi(argv[3]);
        frame = load_yuv420(inputFile, width, height);
    } else {
        std::cerr << "[ERROR] Unsupported file type: " << inputFile << "\n";
        std::exit(1);
    }

    std::cout << "[INFO] YUV420 frame ready: " << frame.width << "x" << frame.height
              << " (" << frame.data.size() << " bytes)\n";
}
