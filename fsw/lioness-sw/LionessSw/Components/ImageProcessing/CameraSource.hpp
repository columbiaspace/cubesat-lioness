#ifndef Components_CameraSource_HPP
#define Components_CameraSource_HPP

#include "LionessSw/Ports/CameraFrameSerializableAc.hpp"
#include "LionessSw/Ports/ImageStatusEnumAc.hpp"

namespace Components {
// Internal camera backend. A successful capture stays valid until releaseFrame.
class CameraSource {
  public:
    static constexpr U32 CAMERA_COUNT = 2;
    virtual ~CameraSource() = default;
    virtual LionessSw::ImageStatus getFrame(U32 cameraId, U32 timeoutMs, LionessSw::CameraFrame& frame) = 0;
    virtual LionessSw::ImageStatus releaseFrame(U32 cameraId, U32 lease) = 0;
};
}
#endif
