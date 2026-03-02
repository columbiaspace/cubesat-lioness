// ======================================================================
// \title  ModeManager.cpp
// \author moisesm
// \brief  cpp file for ModeManager component implementation class
// ======================================================================

#include "ModeManager/ModeManager.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

ModeManager ::ModeManager(const char* const compName) : ModeManagerComponentBase(compName) {}

ModeManager ::~ModeManager() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void ModeManager ::TODO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
