module Components {
    @ Watchdog emulator for host-based watchdog testing
    passive component WatchdogEmulator {
        @ Kick input from the watchdog component under test.
        guarded input port kickIn: Svc.WatchDog

        @ Scheduler tick used to count down timeout ticks.
        guarded input port schedIn: Svc.Sched

        @ Reset signal asserted when countdown expires.
        output port resetOut: Fw.Signal

        @ Manually set the watchdog countdown in scheduler ticks.
        guarded command SET_COUNTDOWN(
            ticks: U32 @< Countdown value in ticks.
        )

        @ Default countdown reload value in scheduler ticks.
        param TIMEOUT_TICKS: U32 default 50

        @ Current countdown value in scheduler ticks.
        telemetry CountdownTicks: U32

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

        @ Enables telemetry channels handling
        import Fw.Channel

        @ Port to return the value of a parameter
        param get port prmGetOut

        @ Port to set the value of a parameter
        param set port prmSetOut

    }
}
