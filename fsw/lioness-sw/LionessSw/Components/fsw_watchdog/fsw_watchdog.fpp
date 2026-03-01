module FSW_Watchdog {
    @ Watchdog circuit for fsw
    passive component fsw_watchdog {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port
        @ TODO

        @ Command for starting watchdog
        sync command start_watchdog()

        @ Command for stopping watchdog
        sync command stop_watchdog()

        telemetry WatchdogTransitions: U32

        event WatchdogStart() \
            severity activity high \
            format "Watchdog started"

        event WatchdogStop() \
            severity activity high \
            format "Watchdog stopped"


        @ Port receiving calls from the rate group
        sync input port run: Svc.Sched

        @ Port sending calls to the GPIO driver
        output port gpioSet: Drv.GpioWrite

        @ Port to start the watchdog
        sync input port start: Fw.Signal

        @ Port to stop the watchdog
        sync input port stop: Fw.Signal

        @ Port to signal a clean reboot
        output port prepareForReboot: Fw.Signal


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

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}