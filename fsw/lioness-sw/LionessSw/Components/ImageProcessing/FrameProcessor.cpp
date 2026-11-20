#include "FrameProcessor.hpp"

#include <cstdint>
#include <cstring>

namespace Components {
namespace FrameProcessor {
using namespace LionessSw;

ImageStatus validate(const Fw::Buffer& source, const FrameInfo& info) {
    const U32 width = info.get_width();
    const U32 height = info.get_height();
    const U32 length = info.get_bytesUsed();
    if (!source.isValid() || width == 0 || height == 0 || length == 0 || length > source.getSize()) {
        return ImageStatus::BAD_FRAME;
    }

    U64 rowBytes = width;
    U32 planes = 1;
    switch (info.get_format()) {
        case PixelFormat::GRAY8:
            if (info.get_bitDepth() != 8)
                return ImageStatus::BAD_FRAME;
            break;
        case PixelFormat::RGB8:
            if (info.get_bitDepth() != 8)
                return ImageStatus::BAD_FRAME;
            rowBytes *= 3;
            break;
        case PixelFormat::YUV420:
            if (info.get_bitDepth() != 8 || width % 2 != 0 || height % 2 != 0)
                return ImageStatus::BAD_FRAME;
            planes = 3;
            break;
        case PixelFormat::BAYER10_PACKED:
            if (info.get_bitDepth() != 10 || width % 4 != 0)
                return ImageStatus::BAD_FRAME;
            rowBytes = static_cast<U64>(width / 4) * 5;
            break;
        case PixelFormat::BAYER16:
            if (info.get_bitDepth() < 9 || info.get_bitDepth() > 16)
                return ImageStatus::BAD_FRAME;
            rowBytes *= 2;
            break;
        default:
            return ImageStatus::BAD_FRAME;
    }
    if (info.get_planeCount() != planes)
        return ImageStatus::BAD_FRAME;

    U64 starts[3] = {};
    U64 ends[3] = {};
    for (U32 i = 0; i < planes; ++i) {
        const auto& plane = info.get_planes()[i];
        const U64 expectedBytes = i == 0 ? rowBytes : width / 2;
        const U32 expectedRows = i == 0 ? height : height / 2;
        if (plane.get_rowBytes() != expectedBytes || plane.get_rows() != expectedRows ||
            plane.get_stride() < expectedBytes) {
            return ImageStatus::BAD_FRAME;
        }
        starts[i] = plane.get_offset();
        ends[i] = starts[i] + static_cast<U64>(expectedRows - 1) * plane.get_stride() + expectedBytes;
        if (ends[i] > length)
            return ImageStatus::BAD_FRAME;
        for (U32 j = 0; j < i; ++j) {
            if (starts[i] < ends[j] && starts[j] < ends[i])
                return ImageStatus::BAD_FRAME;
        }
    }
    return ImageStatus::OK;
}

bool overlaps(const Fw::Buffer& first, const Fw::Buffer& second) {
    if (!first.isValid() || !second.isValid()) return false;
    const auto a = reinterpret_cast<std::uintptr_t>(first.getData());
    const auto b = reinterpret_cast<std::uintptr_t>(second.getData());
    return a <= b ? b - a < first.getSize() : a - b < second.getSize();
}

ImageStatus copy(const Fw::Buffer& source, const FrameInfo& info, Fw::Buffer& destination) {
    const auto status = validate(source, info);
    if (status != ImageStatus::OK)
        return status;
    const auto length = info.get_bytesUsed();
    if (!destination.isValid() || destination.getSize() < length)
        return ImageStatus::BUFFER_TOO_SMALL;

    Fw::Buffer input(source.getData(), length);
    Fw::Buffer output(destination.getData(), length);
    if (overlaps(input, output)) {
        return ImageStatus::BAD_FRAME;
    }
    std::memcpy(destination.getData(), source.getData(), length);
    return ImageStatus::OK;
}
}  // namespace FrameProcessor
}  // namespace Components
