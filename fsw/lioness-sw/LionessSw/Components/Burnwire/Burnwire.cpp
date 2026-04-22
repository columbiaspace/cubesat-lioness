// ======================================================================
// \title  Burnwire.cpp
// \author valedictorian101
// \brief  cpp file for Burnwire component implementation class
// ======================================================================

#include "LionessSw/Components/Burnwire/Burnwire.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Burnwire ::Burnwire(const char* const compName) : BurnwireComponentBase(compName) {
    this->m_timer = 0;
    this->m_state = Fw::On::OFF;
}

Burnwire ::~Burnwire() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void Burnwire::schedIn_handler(FwIndexType portNum, U32 context) {

    if (this->m_state == Fw::On::ON) {

        this->m_timer++;

        // First tick → turn ON
        if (this->m_timer == 1) {
            this->gpioSet_out(0, Fw::Logic::HIGH);
            

            // 🔥 Event: Burn started
            this->log_ACTIVITY_HI_SetBurnwireState(Fw::On::ON);
        }

        // Timeout reached → turn OFF
        if (this->m_timer >= this->m_timeout) {

            this->gpioSet_out(0, Fw::Logic::LOW);

            // Convert ticks → seconds for logging
            U32 seconds = this->m_timeout;

            // 🔥 Event: how long we burned
            this->log_ACTIVITY_LO_BurnwireEndCount(seconds);

            this->m_state = Fw::On::OFF;
            this->m_timer = 0;

            // 🔥 Event: Burn stopped
            this->log_ACTIVITY_HI_SetBurnwireState(Fw::On::OFF);
        }
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Burnwire::START_BURNWIRE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 durationS) {
    Fw::ParamValid valid;
    U32 maxTime = this->paramGet_SAFETY_TIMER(valid);

    if (valid != Fw::ParamValid::VALID) {
        maxTime = 30;
    }

    // Clamp in seconds
    if (durationS > maxTime) {
        durationS = maxTime;
    }

    // Convert seconds → ticks
    this->m_timeout = durationS;

    this->m_timer = 0;
    this->m_state = Fw::On::ON;

    // 🔥 Event: commanded duration
    this->log_ACTIVITY_HI_SafetyTimerState(durationS);

    // ACK command
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}
