// ======================================================================
// \title  fsw_watchdog.cpp
// \author lkrah
// \brief  cpp file for fsw_watchdog component implementation class
// ======================================================================

#include "LionessSw/Components/fsw_watchdog/fsw_watchdog.hpp"

namespace FSW_Watchdog {

  // ----------------------------------------------------------------------
  // Component construction and destruction
  // ----------------------------------------------------------------------

  fsw_watchdog::fsw_watchdog(const char* const compName):fsw_watchdogComponentBase(compName) {}

  fsw_watchdog::~fsw_watchdog() {}

  // ----------------------------------------------------------------------
  // Handler implementations for typed input ports
  // ----------------------------------------------------------------------

  void fsw_watchdog ::
    run_handler(
        FwIndexType portNum,
        U32 context
    ) {
        if(this->m_run) {
            this->m_state = !this->m_state; // Simple bool toggle
            this->m_transitions++;
            this->tlmWrite_WatchdogTransitions(this->m_transitions);

            this->gpioSet_out(0, this->m_state ? Fw::Logic::HIGH : Fw::Logic::LOW);
        }
    }

  void fsw_watchdog ::
    start_handler(FwIndexType portNum)
  {
    this->m_run = 0;

    // Write initial telemetry value to ensure it's available immediately
    this->tlmWrite_WatchdogTransitions(this->m_transitions);

    // Report watchdog started
    this->log_ACTIVITY_HI_WatchdogStart();
  }

  void fsw_watchdog ::
    stop_handler(FwIndexType portNum)
  {
    this->m_run = false;

    // Report watchdog stopped
    this->log_ACTIVITY_HI_WatchdogStop();
  }

  // ----------------------------------------------------------------------
  // Handler implementations for commands
  // ----------------------------------------------------------------------

  void fsw_watchdog ::
    start_watchdog_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
    )
  {
    this->start_handler(0);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

  void fsw_watchdog ::
    stop_watchdog_cmdHandler(
        FwOpcodeType opCode,
        U32 cmdSeq
    )
  {
    this->prepareForReboot_out(0);
    this->stop_handler(0);

    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
  }

}
