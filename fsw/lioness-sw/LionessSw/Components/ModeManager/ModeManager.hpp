// ======================================================================
// \title  ModeManager.hpp
// \author moisesm
// \brief  hpp file for ModeManager component implementation class
// ======================================================================

#ifndef Components_ModeManager_HPP
#define Components_ModeManager_HPP

#include "ModeManager/ModeManagerComponentAc.hpp"

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
