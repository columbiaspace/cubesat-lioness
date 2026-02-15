module LionessSw {
    @ Watchdog emulator for host-based watchdog testing
    passive component WatchdogEmulator {
        @ Kick input from the watchdog component under test.
        sync input port kickIn: Svc.WatchDog

        @ Scheduler tick used to count down timeout ticks.
        sync input port schedIn: Svc.Sched

        @ Reset signal asserted when countdown expires.
        output port resetOut: Fw.Signal

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

    }
}
