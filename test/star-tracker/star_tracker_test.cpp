#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#include "star_id.hpp"

struct RawFrame {
    std::vector<uint8_t> data; // Y plane, then U plane, then V plane
    uint16_t width = 0;
    uint16_t height = 0;
};

struct CentroidParams {
    double yCoordMagSum;
    double xCoordMagSum;
    uint32_t magSum;
    uint16_t xMin;
    uint16_t xMax;
    uint16_t yMin;
    uint16_t yMax;
    uint8_t cutoff;
    bool isValid;
    std::unordered_set<uint32_t> checkedIndices;
};

static RawFrame load_yuv420_from_png(const std::string &inputFile) {
    int width, height, channels;
    uint8_t *rgb = stbi_load(inputFile.c_str(), &width, &height, &channels, 3);
    if (!rgb) {
        std::cerr << "[ERROR] Failed to load PNG: " << inputFile << "\n";
        std::exit(1);
    }

    std::cout << "[INFO] Loaded PNG: " << width << "x" << height << "\n";

    if (width > UINT16_MAX || height > UINT16_MAX) {
        std::cerr << "[ERROR] Image too large\n";
        std::exit(1);
    }

    const uint32_t ySize = width * height;
    const uint32_t uvWidth = (width + 1) / 2;
    const uint32_t uvHeight = (height + 1) / 2;
    const uint32_t uvSize = uvWidth * uvHeight;

    RawFrame frame;
    frame.width = static_cast<uint16_t>(width);
    frame.height = static_cast<uint16_t>(height);
    frame.data.resize(ySize + 2 * uvSize);

    uint8_t *yPlane = frame.data.data();
    uint8_t *uPlane = yPlane + ySize;
    uint8_t *vPlane = uPlane + uvSize;

    // Convert RGB to Y plane (BT.709 luminosity)
    for (uint32_t i = 0; i < ySize; i++) {
        const uint8_t r = rgb[i * 3 + 0];
        const uint8_t g = rgb[i * 3 + 1];
        const uint8_t b = rgb[i * 3 + 2];
        yPlane[i] = static_cast<uint8_t>(std::round(r * 0.21 + g * 0.71 + b * 0.07));
    }

    // Convert RGB to subsampled U and V planes (2x2 averaging)
    for (uint32_t j = 0; j < uvHeight; j++) {
        for (uint32_t i = 0; i < uvWidth; i++) {
            uint16_t rSum = 0, gSum = 0, bSum = 0;
            uint8_t count = 0;
            for (uint8_t dy = 0; dy < 2 && j * 2 + dy < height; dy++) {
                for (uint8_t dx = 0; dx < 2 && i * 2 + dx < width; dx++) {
                    uint32_t px = (j * 2 + dy) * width + (i * 2 + dx);
                    rSum += rgb[px * 3 + 0];
                    gSum += rgb[px * 3 + 1];
                    bSum += rgb[px * 3 + 2];
                    count++;
                }
            }
            const uint8_t r = rSum / count;
            const uint8_t g = gSum / count;
            const uint8_t b = bSum / count;

            const int32_t u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            const int32_t v = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

            uPlane[j * uvWidth + i] = static_cast<uint8_t>(std::clamp(u, 0, 255));
            vPlane[j * uvWidth + i] = static_cast<uint8_t>(std::clamp(v, 0, 255));
        }
    }

    stbi_image_free(rgb);
    return frame;
}

static RawFrame load_yuv420(const std::string &inputFile, const uint16_t width, const uint16_t height) {
    std::ifstream ifs(inputFile, std::ios::binary);
    if (!ifs) {
        std::cerr << "[ERROR] Failed to open raw file: " << inputFile << "\n";
        std::exit(1);
    }

    RawFrame frame;
    frame.width = width;
    frame.height = height;

    const uint32_t ySize = width * height;
    const uint32_t uvSize = ((width + 1) / 2) * ((height + 1) / 2);
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

// a simple, but well tested thresholding algorithm that works well with star images
static uint8_t centroids_basic_threshold(const uint8_t *image, const uint16_t imageWidth, const uint16_t imageHeight) {
    uint64_t totalMag = 0;
    double stdDev = 0;
    uint32_t totalPixels = imageHeight * imageWidth;
    for (uint32_t i = 0; i < totalPixels; i++) {
        totalMag += image[i];
    }
    double mean = static_cast<double>(totalMag) / totalPixels;
    for (uint32_t i = 0; i < totalPixels; i++) {
        stdDev += std::pow(image[i] - mean, 2);
    }
    stdDev = std::sqrt(stdDev / totalPixels);
    return static_cast<uint8_t>(std::clamp(mean + (stdDev * 5), 0.0, 255.0));
}

static void centroids_cog_helper(CentroidParams *p, const uint32_t i, const uint8_t *image, const uint16_t imageWidth, const uint16_t imageHeight) {
    if (i < static_cast<uint32_t>(imageWidth) * imageHeight && image[i] >= p->cutoff && !p->checkedIndices.contains(i)) {
        //check if pixel is on the edge of the image, if it is, we dont want to centroid this star
        if (i % imageWidth == 0 || i % imageWidth == imageWidth - 1 || i / imageWidth == 0 || i / imageWidth == imageHeight - 1) {
            p->isValid = false;
        }
        p->checkedIndices.insert(i);
        if (i % imageWidth > p->xMax) {
            p->xMax = i % imageWidth;
        } else if (i % imageWidth < p->xMin) {
            p->xMin = i % imageWidth;
        }
        if (i / imageWidth > p->yMax) {
            p->yMax = i / imageWidth;
        } else if (i / imageWidth < p->yMin) {
            p->yMin = i / imageWidth;
        }
        p->magSum += image[i];
        p->xCoordMagSum += (i % imageWidth) * image[i];
        p->yCoordMagSum += (i / imageWidth) * image[i]; // NOLINT(*-integer-division)
        if (i % imageWidth != imageWidth - 1) {
            centroids_cog_helper(p, i + 1, image, imageWidth, imageHeight);
        }
        if (i % imageWidth != 0) {
            centroids_cog_helper(p, i - 1, image, imageWidth, imageHeight);
        }
        centroids_cog_helper(p, i + imageWidth, image, imageWidth, imageHeight);
        centroids_cog_helper(p, i - imageWidth, image, imageWidth, imageHeight);
    }
}

static Stars calculate_centroids_cog(const uint8_t *image, const uint16_t imageWidth, const uint16_t imageHeight) {
    CentroidParams p;
    Stars result;

    p.cutoff = centroids_basic_threshold(image, imageWidth, imageHeight);
    for (uint32_t i = 0; i < static_cast<uint32_t>(imageHeight) * imageWidth; i++) {
        if (image[i] >= p.cutoff && !p.checkedIndices.contains(i)) {

            //iterate over pixels that are part of the star
            uint16_t xDiameter = 0; //radius of current star
            uint16_t yDiameter = 0;
            p.yCoordMagSum = 0; //y coordinate of current star
            p.xCoordMagSum = 0; //x coordinate of current star
            p.magSum = 0; //sum of magnitudes of current star

            p.xMax = i % imageWidth;
            p.xMin = i % imageWidth;
            p.yMax = i / imageWidth;
            p.yMin = i / imageWidth;
            p.isValid = true;

            const uint32_t sizeBefore = p.checkedIndices.size();

            centroids_cog_helper(&p, i, image, imageWidth, imageHeight);
            xDiameter = (p.xMax - p.xMin) + 1;
            yDiameter = (p.yMax - p.yMin) + 1;

            //use the sums to finish CoG equation and add stars to the result
            double xCoord = p.xCoordMagSum / (p.magSum * 1.0);
            double yCoord = p.yCoordMagSum / (p.magSum * 1.0);

            if (p.isValid) {
                result.emplace_back(xCoord + 0.5, yCoord + 0.5, xDiameter / 2.0, yDiameter / 2.0, p.checkedIndices.size() - sizeBefore);
            }
        }
    }
    return result;
}

int main(const int argc, const char *argv[]) {
    std::string inputFile = "image.png";
    std::string databaseFile = "database.dat";
    double focalLengthMm = 49.0;
    double pixelSizeUm = 22.2;
    double toleranceDeg = 0.05;
    uint32_t numFalseStars = 1000;
    double maxMismatchProbability = 0.0001;
    uint64_t cutoff = 1000;
    int width = 1920;
    int height = 1080;

    if (argc >= 2 && strcmp(argv[1], "-h") == 0) {
        std::cout << "Usage: " << argv[0] << " [image.png|image.raw] [database.dat] [focal_length_mm] [pixel_size_um] [tolerance_deg] [num_false_stars max_mismatch_probability cutoff] [width height]\n";
        return EXIT_SUCCESS;
    }
    if (argc >= 2) {
        inputFile = argv[1];
    }
    if (argc >= 3) {
        databaseFile = argv[2];
    }
    if (argc >= 5) {
        focalLengthMm = std::stod(argv[3]);
        pixelSizeUm = std::stod(argv[4]);
    }
    if (argc >= 6) {
        toleranceDeg = std::stod(argv[5]);
    }
    if (argc >= 9) {
        numFalseStars = std::stoul(argv[6]);
        maxMismatchProbability = std::stod(argv[7]);
        cutoff = std::stoull(argv[8]);
    }
    if (argc >= 11) {
        width = std::stoi(argv[9]);
        height = std::stoi(argv[10]);
    }

    RawFrame frame;
    if (inputFile.ends_with(".png")) {
        frame = load_yuv420_from_png(inputFile);
    } else if (inputFile.ends_with(".raw")) {
        if (width > UINT16_MAX || height > UINT16_MAX) {
            std::cerr << "[ERROR] Image too large\n";
            std::exit(1);
        }
        frame = load_yuv420(inputFile, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
    } else {
        std::cerr << "[ERROR] Unsupported file type: " << inputFile << "\n";
        std::exit(1);
    }

    std::cout << "[INFO] YUV420 frame ready: " << frame.width << "x" << frame.height
              << " (" << frame.data.size() << " bytes)\n";

    const Stars unfilteredStars = calculate_centroids_cog(frame.data.data(), frame.width, frame.height);

    std::cout << "[INFO] Unfiltered stars: " << unfilteredStars.size() << "\n";

    // Magnitude filter: discard stars with magnitude less than minMagnitude
    constexpr uint32_t minMagnitude = 5;
    Stars stars;
    for (const Star &star : unfilteredStars) {
        if (star.magnitude >= minMagnitude) {
            stars.push_back(star);
        }
    }

    std::cout << "[INFO] Stars: " << stars.size() << "\n";

    std::ifstream dbFile(databaseFile, std::ios::binary | std::ios::ate);
    if (!dbFile) {
        std::cerr << "[ERROR] Cannot open database: " << databaseFile << "\n";
        return EXIT_FAILURE;
    }
    std::streampos dbSize = dbFile.tellg();
    dbFile.seekg(0);
    std::vector<uint8_t> dbData(dbSize);
    dbFile.read(reinterpret_cast<char *>(dbData.data()), dbSize);
    std::cout << "[INFO] Database: " << dbSize << " bytes\n";

    Catalog catalog = star_tracker_load_catalog(dbData.data());
    std::cout << "[INFO] Catalog: " << catalog.size() << " stars\n";

    // Camera: focal length in pixels = focalLength_mm * 1000 / pixelSize_um
    double focalLengthPx = focalLengthMm * 1000.0 / pixelSizeUm;
    Camera camera(focalLengthPx, frame.width, frame.height);
    std::cout << "[INFO] Focal length: " << focalLengthPx << " px\n";

    // Star identification
    double toleranceRad = toleranceDeg * M_PI / 180.0;
    StarIdentifiers identified = star_tracker_pyramid_star_id(dbData.data(), stars, catalog, camera,
                                                toleranceRad, numFalseStars, maxMismatchProbability, cutoff);
    std::cout << "[INFO] Identified: " << identified.size() << " stars\n";

    // Attitude estimation
    if (identified.size() >= 2) {
        Quaternion q = star_tracker_quest_attitude(camera, stars, catalog, identified);
        EulerAngles ea = q.ToSpherical();
        std::cout << "[INFO] Attitude:\n"
                  << "  RA:   " << RadToDeg(ea.ra)   << " deg\n"
                  << "  Dec:  " << RadToDeg(ea.de)   << " deg\n"
                  << "  Roll: " << RadToDeg(ea.roll) << " deg\n";
    } else {
        std::cerr << "[WARN] Not enough identified stars for attitude.\n";
    }

    return EXIT_SUCCESS;
}
