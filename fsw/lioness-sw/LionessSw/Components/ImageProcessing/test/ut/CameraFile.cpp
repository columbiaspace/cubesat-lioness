#include "ImageProcessingTester.hpp"

#include <png.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <unistd.h>

using namespace LionessSw;

// PNG conversion and file saving belong to this test caller, not ImageProcessing.
TEST(CameraFile, RoundTrip) {
    const char* input = std::getenv("LIONESS_IMAGE_INPUT");
    const char* output = std::getenv("LIONESS_IMAGE_OUTPUT");
    if (input == nullptr) GTEST_SKIP() << "Set LIONESS_IMAGE_INPUT to a camera PNG";
    ASSERT_NE(output, nullptr) << "Set LIONESS_IMAGE_OUTPUT to an existing output directory";

    png_image image = {};
    image.version = PNG_IMAGE_VERSION;
    ASSERT_NE(png_image_begin_read_from_file(&image, input), 0) << image.message;
    image.format = PNG_FORMAT_RGB;
    const U32 width = image.width;
    const U32 height = image.height;
    const U64 length = static_cast<U64>(width) * height * 3;
    if (width == 0 || height == 0 || length > 16 * 1024 * 1024) {
        png_image_free(&image);
        FAIL() << "Use a nonempty image no larger than 16 MiB of RGB pixels";
    }
    std::vector<U8> original(static_cast<size_t>(length));
    const bool read = png_image_finish_read(&image, nullptr, original.data(), 0, nullptr) != 0;
    png_image_free(&image);
    ASSERT_TRUE(read) << "Could not decode input PNG";

    std::string pattern = std::string(output) + "/run-XXXXXX";
    std::vector<char> name(pattern.begin(), pattern.end());
    name.push_back('\0');
    ASSERT_NE(::mkdtemp(name.data()), nullptr) << "Output directory must exist and be writable";
    const std::string folder(name.data());

    Components::ImageProcessingTester test;
    test.setImage(width, height, original);
    ASSERT_TRUE(test.configure(10000, static_cast<U32>(length)));
    std::printf("Input decoded: %u x %u RGB8, %llu bytes\n", width, height,
                static_cast<unsigned long long>(length));
    auto destination = test.buffer();
    ASSERT_EQ(test.capture(101), ImageStatus::ACCEPTED);
    std::printf("capture[0]: ACCEPTED; dispatching worker handler\n");
    test.step();
    ASSERT_EQ(test.results.size(), 1u);
    auto& result = test.results[0];
    ASSERT_EQ(result.get_status(), ImageStatus::OK);
    ASSERT_EQ(result.get_bytesWritten(), length);
    ASSERT_EQ(test.cameraReturns, 1u);
    const std::vector<U8> returned(destination.getData(), destination.getData() + result.get_bytesWritten());
    ASSERT_EQ(returned, original) << "Caller buffer differs from input pixels";
    std::printf("imageReady[0]: OK; caller bytes match; camera lease released\n");

    const std::string processed = folder + "/processed.png";
    image = {};
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;
    image.format = PNG_FORMAT_RGB;
    const bool written = png_image_write_to_file(&image, processed.c_str(), 0, returned.data(), 0, nullptr) != 0;
    png_image_free(&image);
    ASSERT_TRUE(written) << "Test caller could not save PNG";
    image = {};
    image.version = PNG_IMAGE_VERSION;
    ASSERT_NE(png_image_begin_read_from_file(&image, processed.c_str()), 0);
    image.format = PNG_FORMAT_RGB;
    const bool sameSize = image.width == width && image.height == height;
    if (!sameSize) {
        png_image_free(&image);
        FAIL() << "Saved PNG dimensions differ";
    }
    std::vector<U8> decoded(original.size());
    const bool decodedOk = png_image_finish_read(&image, nullptr, decoded.data(), 0, nullptr) != 0;
    png_image_free(&image);
    ASSERT_TRUE(decodedOk);
    ASSERT_EQ(decoded, original) << "PNG round trip changed pixels";

    std::ofstream bytes(folder + "/buffer.bin", std::ios::binary);
    bytes.write(reinterpret_cast<const char*>(returned.data()), returned.size());
    bytes.close();
    ASSERT_TRUE(bytes.good());
    std::ofstream metadata(folder + "/frame.json");
    metadata << "{\n  \"width\": " << width << ",\n  \"height\": " << height
             << ",\n  \"format\": \"RGB8\",\n  \"offset\": 0,\n  \"stride\": " << width * 3
             << ",\n  \"bytesWritten\": " << length << ",\n  \"requestId\": 101,\n"
             << "  \"png\": \"processed.png\"\n}\n";
    metadata.close();
    ASSERT_TRUE(metadata.good());

    ASSERT_TRUE(test.component.cleanup());
    std::printf("PNG decoded: PASS, 0 differing pixels\n");
    std::printf("Caller saved PNG and buffer dump; no buffer-return call needed\n");
    std::printf("Artifacts: %s\n", folder.c_str());
}
