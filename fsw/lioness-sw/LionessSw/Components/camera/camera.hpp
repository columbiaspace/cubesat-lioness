// ======================================================================
// \title  camera.hpp
// \author jchen25
// \brief  hpp file for camera component implementation class
// ======================================================================

#ifndef Camera_camera_HPP
#define Camera_camera_HPP

#include "LionessSw/Components/camera/cameraComponentAc.hpp"

namespace Camera {

class camera final : public cameraComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct camera object
    camera(const char* const compName  //!< The component name
    );

    //! Destroy camera object
    ~camera();
};

}  // namespace Camera

#endif
