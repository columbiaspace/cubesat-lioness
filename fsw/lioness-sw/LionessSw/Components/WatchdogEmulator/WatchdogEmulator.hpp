// ======================================================================
// \title  WatchdogEmulator.hpp
// \author wesleymaa
// \brief  hpp file for WatchdogEmulator component implementation class
// ======================================================================

#ifndef LionessSw_WatchdogEmulator_HPP
#define LionessSw_WatchdogEmulator_HPP

#include "LionessSw/Components/WatchdogEmulator/WatchdogEmulatorComponentAc.hpp"

namespace LionessSw {

class WatchdogEmulator final : public WatchdogEmulatorComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct WatchdogEmulator object
    WatchdogEmulator(const char* const compName  //!< The component name
    );

    //! Destroy WatchdogEmulator object
    ~WatchdogEmulator();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for kickIn
    //!
    //! Kick input from the watchdog component under test.
    void kickIn_handler(FwIndexType portNum,  //!< The port number
                        U32 code              //!< Watchdog stroke code
                        ) override;

    //! Handler implementation for schedIn
    //!
    //! Scheduler tick used to count down timeout ticks.
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;

    //! Handler implementation for command SET_COUNTDOWN
    //!
    //! Manually set the watchdog countdown in scheduler ticks.
    void SET_COUNTDOWN_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                  U32 cmdSeq,           //!< The command sequence number
                                  U32 ticks             //!< Countdown value in ticks
                                  ) override;

    static constexpr U32 TIMEOUT_TICKS = 50;
    U32 m_countdownTicks;
    bool m_resetAsserted;
};

}  // namespace LionessSw

#endif
