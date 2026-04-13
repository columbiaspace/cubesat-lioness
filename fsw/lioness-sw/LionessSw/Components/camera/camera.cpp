// ======================================================================
// \title  camera.cpp
// \author gracexu
// \brief  cpp file for Camera component implementation class
// ======================================================================


#include "LionessSw/Components/camera/camera.hpp"
namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Camera ::Camera(const char* const compName) : CameraComponentBase(compName) {}

Camera ::~Camera() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

Fw::String Camera ::takePicture_handler(FwIndexType portNum) {
    // TODO return
}

}  // namespace Components
