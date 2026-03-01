// ======================================================================
// \title  fsw_watchdog.hpp
// \author lkrah
// \brief  hpp file for fsw_watchdog component implementation class
// ======================================================================

#ifndef FSW_Watchdog_fsw_watchdog_HPP
#define FSW_Watchdog_fsw_watchdog_HPP

#include "LionessSw/Components/fsw_watchdog/fsw_watchdogComponentAc.hpp"

namespace FSW_Watchdog {

  class fsw_watchdog final :
    public fsw_watchdogComponentBase
  {

    public:

      // ----------------------------------------------------------------------
      // Component construction and destruction
      // ----------------------------------------------------------------------

      //! Construct fsw_watchdog object
      fsw_watchdog(
          const char* const compName //!< The component name
      );

      //! Destroy fsw_watchdog object
      ~fsw_watchdog();

    private:

      // ----------------------------------------------------------------------
      // Handler implementations for typed input ports
      // ----------------------------------------------------------------------

      //! Handler implementation for run
      //!
      //! Port receiving calls from the rate group
      void run_handler(
          FwIndexType portNum, //!< The port number
          U32 context //!< The call order
      ) override;

      //! Handler implementation for start
      //!
      //! Port to start the watchdog
      void start_handler(
          FwIndexType portNum //!< The port number
      ) override;

      //! Handler implementation for stop
      //!
      //! Port to stop the watchdog
      void stop_handler(
          FwIndexType portNum //!< The port number
      ) override;

    private:

      // ----------------------------------------------------------------------
      // Handler implementations for commands
      // ----------------------------------------------------------------------

      //! Handler implementation for command start_watchdog
      //!
      //! TODO
      //! Command for starting watchdog
      void start_watchdog_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq //!< The command sequence number
      ) override;

      //! Handler implementation for command stop_watchdog
      //!
      //! Command for stopping watchdog
      void stop_watchdog_cmdHandler(
          FwOpcodeType opCode, //!< The opcode
          U32 cmdSeq //!< The command sequence number
      ) override;

    std::atomic_bool m_run{true};  //! Boolean to determine if watchdog is running
    bool m_state = false;
    U32 m_transitions = 0; //! Keeps track of how many transitions
  };

}

#endif
