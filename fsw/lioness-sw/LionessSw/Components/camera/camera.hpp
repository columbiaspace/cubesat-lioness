// ======================================================================
// \title  camera.hpp
// \author gracexu
// \brief  hpp file for Camera component implementation class
// ======================================================================

#ifndef Components_Camera_HPP
#define Components_Camera_HPP

#include "LionessSw/Components/camera/CameraComponentAc.hpp"

namespace Components {

class Camera final : public CameraComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct Camera object
    Camera(const char* const compName  //!< The component name
    );

    //! Destroy Camera object
    ~Camera();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for takePicture
    //!
    //! Port for star tracker component to take image
    Fw::String takePicture_handler(FwIndexType portNum  //!< The port number
                                   ) override;
};

}  // namespace Components

#endif
