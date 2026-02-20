// ======================================================================
// \title  ASIDetector.cpp
// \author jchen25
// \brief  cpp file for ASIDetector component implementation class
// ======================================================================

#include "LionessSw/Components/ASIDetector/ASIDetector.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

ASIDetector ::ASIDetector(const char* const compName) : ASIDetectorComponentBase(compName) {}

ASIDetector ::~ASIDetector() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void ASIDetector ::TODO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
