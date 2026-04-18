// ======================================================================
// \title  Burnwire.hpp
// \author valedictorian101
// \brief  hpp file for Burnwire component implementation class
// ======================================================================

#ifndef Components_Burnwire_HPP
#define Components_Burnwire_HPP

#include "LionessSw/Components/Burnwire/BurnwireComponentAc.hpp"

namespace Components {

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
                                   U32 cmdSeq,           //!< The command sequence number
                                   U32 durationS) override;

    
    Fw::On m_state = Fw::On::OFF;   // ON/OFF burn state
    U32 m_safetyCounter = 0;       
    U32 m_timeout = 0;
    static constexpr U32 SCHED_HZ = 10;              
                                       
};

}  // namespace Components

#endif
