// ======================================================================
// \title  WatchdogEmulator.cpp
// \author wesleymaa
// \brief  cpp file for WatchdogEmulator component implementation class
// ======================================================================

#include "LionessSw/Components/WatchdogEmulator/WatchdogEmulator.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

WatchdogEmulator ::WatchdogEmulator(const char* const compName)
    : WatchdogEmulatorComponentBase(compName), m_countdownTicks(0), m_resetAsserted(false) {}

WatchdogEmulator ::~WatchdogEmulator() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void WatchdogEmulator ::kickIn_handler(FwIndexType portNum, U32 code) {
    static_cast<void>(portNum);
    static_cast<void>(code);

    this->m_countdownTicks = this->getTimeoutTicks();
    this->m_resetAsserted = false;
    this->tlmWrite_CountdownTicks(this->m_countdownTicks);
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

    this->tlmWrite_CountdownTicks(this->m_countdownTicks);
}

void WatchdogEmulator ::SET_COUNTDOWN_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 ticks) {
    this->m_countdownTicks = ticks;
    this->m_resetAsserted = false;
    this->tlmWrite_CountdownTicks(this->m_countdownTicks);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ----------------------------------------------------------------------
// Private helpers
// ----------------------------------------------------------------------

// Returns the configured TIMEOUT_TICKS, falling back to the FPP-declared
// default if PrmDb has not yet served a valid value (e.g. before the
// first loadParameters() completes).
U32 WatchdogEmulator ::getTimeoutTicks() {
    Fw::ParamValid valid;
    const U32 timeout = this->paramGet_TIMEOUT_TICKS(valid);
    if (valid == Fw::ParamValid::VALID || valid == Fw::ParamValid::DEFAULT) {
        return timeout;
    }
    return 50U;
}

}  // namespace Components
