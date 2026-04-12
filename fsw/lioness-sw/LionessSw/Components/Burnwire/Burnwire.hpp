// ======================================================================
// \title  Burnwire.hpp
// \author valedictorian101
// \brief  hpp file for Burnwire component implementation class
// ======================================================================

#ifndef LionessSw_Burnwire_HPP
#define LionessSw_Burnwire_HPP

#include "LionessSw/Components/Burnwire/BurnwireComponentAc.hpp"

namespace LionessSw {

class Burnwire final : public BurnwireComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct Burnwire object
    Burnwire(const char* const compName  //!< The component name
    );

    //! Destroy Burnwire object
    ~Burnwire();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for burnStart
    //!
    //! Port getting start signal
    void burnStart_handler(FwIndexType portNum  //!< The port number
                           ) override;

    //! Handler implementation for burnStop
    //!
    //! Port getting stop signal
    void burnStop_handler(FwIndexType portNum  //!< The port number
                          ) override;

    //! Handler implementation for schedIn
    //!
    //! Input Port to get the rate group
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command START_BURNWIRE
    //!
    //! START_BURNWIRE turns on the burnwire
    void START_BURNWIRE_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                   U32 cmdSeq            //!< The command sequence number
                                   ) override;

    //! Handler implementation for command STOP_BURNWIRE
    //!
    //! STOP_BURNWIRE turns on the burnwire
    void STOP_BURNWIRE_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                  U32 cmdSeq            //!< The command sequence number
                                  ) override;
};

}  // namespace LionessSw

#endif
