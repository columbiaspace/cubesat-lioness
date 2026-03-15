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

    if (tick_counter >= 5) {
      this->log_ACTIVITY_LO_SunSensorTick();

      // reset 
      tick_counter = 0;
    }


  }

}

