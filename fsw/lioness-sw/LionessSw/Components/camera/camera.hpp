// ======================================================================
// \title  Camera.hpp
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
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command TAKE_IMAGE
    //!
    //! Type in "snap" to capture an image
    void TAKE_IMAGE_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                               U32 cmdSeq            //!< The command sequence number
                               ) override;

    //! Handler implementation for command SET_CONTINUOUS
    void SET_CONTINUOUS_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                   U32 cmdSeq,           //!< The command sequence number
                                   bool continuous) override;
};

}  // namespace Components

#endif
