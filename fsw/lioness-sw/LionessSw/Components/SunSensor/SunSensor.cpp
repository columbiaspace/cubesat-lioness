// ======================================================================
// \title  SunSensor.cpp
// \author tk3144
// \brief  cpp file for SunSensor component implementation class
// ======================================================================

#include "LionessSw/Components/SunSensor/SunSensor.hpp"

namespace Components {

  SunSensor::SunSensor(const char* const compName) : 
    SunSensorComponentBase(compName) 
  { }

  SunSensor::~SunSensor() { }

  void SunSensor::run_handler(
      const FwIndexType portNum,
      U32 context
  ) {
    // Rate Group 1 = 1s, 2 = 2s, 3 = 4s
    // Since our rate groups are in intervals of 1, 2, 4 seconds respectively,
    // and we want a ping every 10 seconds, we use a counter to tally for Rate Group 2
    
    // static to preserve value of tick_counter when the method finishes
    static U32 tick_counter = 0;
    tick_counter++;

    // array values
    m_diode_readings[0] = 0.0f; // PX
    m_diode_readings[1] = 0.0f; // MX
    m_diode_readings[2] = 0.0f; // PY
    m_diode_readings[3] = 0.0f; // MY
    m_diode_readings[4] = 0.0f; // PZ
    m_diode_readings[5] = 0.0f; // MZ

    // Increment Diode Values Periodically
    m_diode_readings[0] = m_diode_readings[0] + ((F32)tick_counter)/10;

    // Diode telemetry to see raw values in GDS
    this->tlmWrite_DIODE_PX(m_diode_readings[0]);
    this->tlmWrite_DIODE_MX(m_diode_readings[1]);
    this->tlmWrite_DIODE_PY(m_diode_readings[2]);
    this->tlmWrite_DIODE_MY(m_diode_readings[3]);
    this->tlmWrite_DIODE_PZ(m_diode_readings[4]);
    this->tlmWrite_DIODE_MZ(m_diode_readings[5]);

    if (tick_counter >= 5) {
      this->log_ACTIVITY_LO_SunSensorTick();
      this->calculateSunVector();
      tick_counter = 0;  // reset 
    }
  }

  void SunSensor::calculateSunVector() {
    /*
    Calculating Sun Vectors:
    For each axis, 

    Sx = (diodePX - diodeMX) / (diodePX + diodeMX)
    
    Note: P = Plus, M = Minus
    Note: Resulting vector S = [sx, sy, sz]

    We need to avoid dividing by 0. Let epsilon be the error bound

    We complete with normalizing the calculated vectors (using pythagorean theorem)
    Magnitude = sqrt(sx^2 + sy^2 + sz^2)
    S_unit = S / Magnitude for unit x,y,z
    */
    const F32 epsilon = 0.001f;

    // Hard-coded math (likely theres a cleaner/easier way to do this, but don't want to mess with libraries atm)
    // calculate sums (Denominator)
    F32 sum_x = m_diode_readings[0] + m_diode_readings[1];
    F32 sum_y = m_diode_readings[2] + m_diode_readings[3];
    F32 sum_z = m_diode_readings[4] + m_diode_readings[5];

    
    // calculate differences (Numerator)
    F32 diff_x = m_diode_readings[0] - m_diode_readings[1];
    F32 diff_y = m_diode_readings[2] - m_diode_readings[3];
    F32 diff_z = m_diode_readings[4] - m_diode_readings[5];

    // Calculate Sun Vectors. If s < epsilon, set to 0
    F32 sx; // = (sum_x > epsilon) ? (diff_x / sum_x) : 0.0f;
    F32 sy; // = (sum_y > epsilon) ? (diff_y / sum_y) : 0.0f;
    F32 sz; // = (sum_z > epsilon) ? (diff_z / sum_z) : 0.0f;

    // Calculate length of vector
    F32 magnitude_squared = (diff_x * diff_x) + (diff_y * diff_y) + (diff_z * diff_z);

    // Scale to Unit Vector (magnitude = 1.0, direction only)
    if (magnitude_squared > epsilon) {
        F32 magnitude = sqrtf(magnitude_squared);
        sx = diff_x / magnitude;
        sy = diff_y / magnitude;
        sz = diff_z / magnitude;
    } else {
        // clear the vector
        sx = 0.0f;
        sy = 0.0f;
        sz = 0.0f;
    }

    // Log the calculated vector 
    this->log_ACTIVITY_HI_SunVectorCalculated(sx, sy, sz);
    
    // Write the vector telemetry 
    this->tlmWrite_Sun_Vec_X(sx);
    this->tlmWrite_Sun_Vec_Y(sy);
    this->tlmWrite_Sun_Vec_Z(sz);
  }


}



/*
// Proxy telemetry channels test (in tick_counter)
// Every 10 seconds, the location is updated under its respective Channel in g

in SunSensor.fpp:
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


Here, in run_handler():
this->tlmWrite_DIODE_PX(1.0f);   // this face is in direct sunlight
this->tlmWrite_DIODE_MX(0.0f);   // Opposite face is in total shadow
this->tlmWrite_DIODE_PY(0.5f);   // partial sunlight (45 degree angle)
this->tlmWrite_DIODE_MY(0.0f);
this->tlmWrite_DIODE_PZ(0.0f);   // top/bottom in shadow
this->tlmWrite_DIODE_MZ(0.0f);
*/

/*
Tested Edge Cases
All edge cases have been satisfied appropriately
Note: If not explicitly valued, assume the photodiode values not mentioned are 0.

1. Single Axis for Direct Sun
  px = 1
  Expected Output: sx = 1

2. 45 degree angle 
  px = 2, py = 2
  Expected Output: sx = 0.707, sy = 0.707

3. Epsilon edge case if very little Sun
  px = 0.0001
  Expected Output: sx = 0

4. Negative values
  mx = 5
  Exepcted output: sx = -1

5. Opposite Cancelling
  px = mx = 10
  Expected Output: sx = 0

6. All "P" photodiodes with values
  px = py = pz = 10
  Expected Output: sx = sy = sz = 0.577

7. The difference of very small values (of opposite-sided photodiodes)
  px = 0.001001, mx = 0.001
  Expected Output: sx = 0

8. Asymmetry between one axis (positive val) and another (negative val)
  px = 10, my = -2
  Expected Output: sx = 0.981, sy = 0.196


Note: (Case 5) If values on two (opposite-sided) photodiodes are ever close enough to being the same value and the other photodiodes are are omitted, as this code stands, the calculations will assume there is no Sun.
              We may need to consider a "baseline" reflection expected from Earth (or possibly other satelites if somehow, but unlikely, brighter)
              OR
              A hardware filter over the photodiodes can be proposed to help discern this difference.

*/