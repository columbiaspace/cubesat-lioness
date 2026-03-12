module Components {
    @ Logic that switches the satellite in and out of different modes
    active component ModeManager {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

        async command ToStandby()
        async command ToExperiment()
        async command ToTransmit()
        async command ToDetumble()
        async command ToSafe()

        telemetry CurrentMode: LionessSw.MODE   
        telemetry BatteryLevel: F32
        telemetry Acceleration: F32

        event ModeChanged(
            prevmode: LionessSw.MODE, newmode: LionessSw.MODE, reason: string size 20
        ) severity activity high id 0 format "Mode changed from {} to {} due to {}"

        event InvalidModeChange(
            prevmode: LionessSw.MODE, newmode: LionessSw.MODE, attemptedBy: string size 20
        ) severity activity high id 1 format "Invalid Mode Change {} to {} attempted by {}"
        event BatteryLow(
            currLevel: F32, safeLevel: F32
        ) severity activity high id 2 format "Warning: Battery Level {}% < {}%, transitioning to SAFE"
        event HighSpinRate(
            currAcceleration: F32, safeAcceleration: F32
        ) severity activity high id 3 format "Warning: Spin Rate {} > {}, transitioning to DETUMBLE"

        sync input port getMode: LionessSw.GetMode
        sync input port schedIn: Svc.Sched
        output port getBattery: LionessSw.GetBattery
        output port getAcceleration: LionessSw.GetAcceleration

        ##############################################################################
        #### Uncomment the following examples to start customizing your component ####
        ##############################################################################

        # @ Example async command
        # async command COMMAND_NAME(param_name: U32)

        # @ Example telemetry counter
        # telemetry ExampleCounter: U64

        # @ Example event
        # event ExampleStateEvent(example_state: Fw.On) severity activity high id 0 format "State set to {}"

        # @ Example port: receiving calls from the rate group
        # sync input port run: Svc.Sched

        # @ Example parameter
        # param PARAMETER_NAME: U32

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