// ======================================================================
// \title  Burnwire.cpp
// \author valedictorian101
// \brief  cpp file for Burnwire component implementation class
// ======================================================================

#include "LionessSw/Components/Burnwire/Burnwire.hpp"

namespace LionessSw {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Burnwire ::Burnwire(const char* const compName) : BurnwireComponentBase(compName) {}

Burnwire ::~Burnwire() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void Burnwire ::burnStart_handler(FwIndexType portNum) {
    // TODO
}

void Burnwire ::burnStop_handler(FwIndexType portNum) {
    // TODO
}

void Burnwire ::schedIn_handler(FwIndexType portNum, U32 context) {
    // TODO
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Burnwire ::START_BURNWIRE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void Burnwire ::STOP_BURNWIRE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace LionessSw
