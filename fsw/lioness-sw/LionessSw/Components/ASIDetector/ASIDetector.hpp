// ======================================================================
// \title  ASIDetector.hpp
// \author jchen25
// \brief  hpp file for ASIDetector component implementation class
// ======================================================================

#ifndef Components_ASIDetector_HPP
#define Components_ASIDetector_HPP

#include "LionessSw/Components/ASIDetector/ASIDetectorComponentAc.hpp"

namespace Components {

class ASIDetector final : public ASIDetectorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct ASIDetector object
    ASIDetector(const char* const compName  //!< The component name
    );

    //! Destroy ASIDetector object
    ~ASIDetector();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command TODO
    //!
    //! TODO
    void TODO_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                         U32 cmdSeq            //!< The command sequence number
                         ) override;
};

}  // namespace Components

#endif
