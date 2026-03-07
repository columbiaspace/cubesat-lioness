// ======================================================================
// \title  Camera.cpp
// \author gracexu
// \brief  cpp file for Camera component implementation class
// ======================================================================

#include "LionessSw/Components/Camera/Camera.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Camera ::Camera(const char* const compName) : CameraComponentBase(compName) {}

Camera ::~Camera() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Camera ::TAKE_IMAGE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void Camera ::SET_CONTINUOUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, bool continuous) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
