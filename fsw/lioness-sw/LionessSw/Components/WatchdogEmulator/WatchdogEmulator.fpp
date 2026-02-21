module LionessSw {
    @ Watchdog emulator for host-based watchdog testing
    passive component WatchdogEmulator {
        @ Kick input from the watchdog component under test.
        sync input port kickIn: Svc.WatchDog

        @ Scheduler tick used to count down timeout ticks.
        sync input port schedIn: Svc.Sched

        @ Reset signal asserted when countdown expires.
        output port resetOut: Fw.Signal

        @ Manually set the watchdog countdown in scheduler ticks.
        sync command SET_COUNTDOWN(
            ticks: U32 @< Countdown value in ticks.
        )

        @ Countdown reached zero and reset was asserted.
        event ResetAsserted \
            severity activity high \
            format "Watchdog reset asserted after countdown expired"

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Enables command handling
        import Fw.Command

        @ Enables event handling
        import Fw.Event

    }
}
