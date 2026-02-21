// ======================================================================
// \title  WatchdogEmulator.cpp
// \author wesleymaa
// \brief  cpp file for WatchdogEmulator component implementation class
// ======================================================================

#include "LionessSw/Components/WatchdogEmulator/WatchdogEmulator.hpp"

namespace LionessSw {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

WatchdogEmulator ::WatchdogEmulator(const char* const compName)
    : WatchdogEmulatorComponentBase(compName), m_countdownTicks(TIMEOUT_TICKS), m_resetAsserted(false) {}

WatchdogEmulator ::~WatchdogEmulator() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void WatchdogEmulator ::kickIn_handler(FwIndexType portNum, U32 code) {
    static_cast<void>(portNum);
    static_cast<void>(code);

    this->m_countdownTicks = TIMEOUT_TICKS;
    this->m_resetAsserted = false;
}

void WatchdogEmulator ::schedIn_handler(FwIndexType portNum, U32 context) {
    static_cast<void>(portNum);
    static_cast<void>(context);

    if (this->m_countdownTicks > 0) {
        this->m_countdownTicks--;
    }

    if ((this->m_countdownTicks == 0U) && (not this->m_resetAsserted)) {
        this->resetOut_out(0);
        this->log_ACTIVITY_HI_ResetAsserted();
        this->m_resetAsserted = true;
    }
}

void WatchdogEmulator ::SET_COUNTDOWN_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 ticks) {
    this->m_countdownTicks = ticks;
    this->m_resetAsserted = false;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace LionessSw
