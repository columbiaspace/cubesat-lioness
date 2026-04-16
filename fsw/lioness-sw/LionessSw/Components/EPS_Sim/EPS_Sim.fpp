module Components {
    port BatteryLevelGet(ref batteryLevel: F32)

    enum OpMode {
      DRAINING = -1,
      OFF = 0,
      CHARGING = 1
    }
}

module Components {
    @ Basic simulator for EPS to emulate satellite power
    passive component EPS_Sim {
        sync input port run: Svc.Sched

        @ Command for starting simulation
        sync command SET_EPS_SIM_OPMODE(opMode: OpMode)

        @ Command for changing the Simulation Mode
        sync command SET_EPS_SIM_BATTERY_LEVEL(batteryLevel: F32)

        event EPS_SimSetOpMode(opMode: OpMode) \
            severity activity high \
            format "EPS Simulation Operation Mode set to {}"

        sync input port getBattery: BatteryLevelGet

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