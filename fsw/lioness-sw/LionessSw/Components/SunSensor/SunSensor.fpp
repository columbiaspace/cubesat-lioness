module Components {
  @ Sun Sensor component for attitude determination
  passive component SunSensor {

    @ Run port receiving the periodic signal from Rate Group
    sync input port run: [1] Svc.Sched

    # --- Telemetry ---
    @ Light intensity on the +X face
    telemetry DIODE_PX: F32 id 0 format "{.2f}"
    @ Light intensity on the -X face
    telemetry DIODE_MX: F32 id 1 format "{.2f}"
    
    @ Light intensity on the +Y face
    telemetry DIODE_PY: F32 id 2 format "{.2f}"
    @ Light intensity on the -Y face
    telemetry DIODE_MY: F32 id 3 format "{.2f}"
    
    @ Light intensity on the +Z face
    telemetry DIODE_PZ: F32 id 4 format "{.2f}"
    @ Light intensity on the -Z face
    telemetry DIODE_MZ: F32 id 5 format "{.2f}"

    @ Calculated Sun Vector X-component
    telemetry Sun_Vec_X: F32 id 6

    @ Calculated Sun Vector Y-component
    telemetry Sun_Vec_Y: F32 id 7

    @ Calculated Sun Vector Z-component
    telemetry Sun_Vec_Z: F32 id 8




    # --- Events ---
    event SunSensorTick() \
      severity activity low \
      format "Sun Sensor Rate Group tick received"

    @ Sun Direction Vector Computation
    event SunVectorCalculated(x: F32, y: F32, z: F32) \
      severity activity high \
      format "Calculated Sun Vector: x={.2f}, y={.2f}, z={.2f}"

    # --- Standard F Prime Ports ---
    time get port timeCaller
    event port eventOut
    text event port textEventOut
    telemetry port tlmOut
  }
}