#ifndef Components_FrameProcessor_HPP
#define Components_FrameProcessor_HPP

#include "Fw/Buffer/Buffer.hpp"
#include "LionessSw/Ports/FrameInfoSerializableAc.hpp"
#include "LionessSw/Ports/ImageStatusEnumAc.hpp"

namespace Components {
namespace FrameProcessor {

// Check the layout before accessing camera memory or passing it to an encoder.
LionessSw::ImageStatus validate(const Fw::Buffer& source, const LionessSw::FrameInfo& info);
bool overlaps(const Fw::Buffer& first, const Fw::Buffer& second);
LionessSw::ImageStatus copy(const Fw::Buffer& source, const LionessSw::FrameInfo& info,
                           Fw::Buffer& destination);

}
}
#endif
