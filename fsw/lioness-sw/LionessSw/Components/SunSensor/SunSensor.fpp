module Components {
  @ Sun Sensor component for attitude determination
  passive component SunSensor {

    @ Run port receiving the periodic signal from Rate Group
    sync input port run: [1] Svc.Sched

    # --- Events ---
    event SunSensorTick() \
      severity activity low \
      format "Sun Sensor Rate Group tick received"

    # --- Standard F Prime Ports ---
    time get port timeCaller
    event port eventOut
    text event port textEventOut
    telemetry port tlmOut
  }
}