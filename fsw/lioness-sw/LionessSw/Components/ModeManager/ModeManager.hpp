// ======================================================================
// \title  ModeManager.hpp
// \author skn
// \brief  hpp file for ModeManager component implementation class
// ======================================================================

#ifndef Components_ModeManager_HPP
#define Components_ModeManager_HPP

#include "LionessSw/Components/ModeManager/ModeManagerComponentAc.hpp"

namespace Components {

class ModeManager final : public ModeManagerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct ModeManager object
    ModeManager(const char* const compName  //!< The component name
    );

    //! Destroy ModeManager object
    ~ModeManager();

  //Sensor retrieval functions (dummy functions and values for now)
  private:
    static constexpr F32 SAFE_BATTERY_THRESHOLD = 80.0f;
    static constexpr F32 SAFE_ACCELERATION_THRESHOLD = 10.0f;

  private:
    LionessSw::MODE  currMode = LionessSw::MODE::SAFE;
    F32 currBattery = 100.0f; //dummy functions
    F32 currAcceleration = 0.0f; //dummy functions

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for getMode
    void getMode_handler(FwIndexType portNum,  //!< The port number
                                 LionessSw::MODE& mode
                                ) override;

    //! Handler implementation for schedIn
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command ToStandby
    void ToStandby_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                              U32 cmdSeq            //!< The command sequence number
                              ) override;

    //! Handler implementation for command ToExperiment
    void ToExperiment_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                 U32 cmdSeq            //!< The command sequence number
                                 ) override;

    //! Handler implementation for command ToTransmit
    void ToTransmit_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                               U32 cmdSeq            //!< The command sequence number
                               ) override;

    //! Handler implementation for command ToDetumble
    void ToDetumble_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                               U32 cmdSeq            //!< The command sequence number
                               ) override;

    //! Handler implementation for command ToSafe
    void ToSafe_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                           U32 cmdSeq            //!< The command sequence number
                           ) override;
};

}  // namespace Components

#endif
