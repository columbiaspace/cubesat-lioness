// ======================================================================
// \title  SunSensor.hpp
// \author tk3144
// \brief  hpp file for SunSensor component implementation class
// ======================================================================

#ifndef Components_SunSensor_HPP
#define Components_SunSensor_HPP

#include <Fw/FPrimeBasicTypes.hpp>
#include "LionessSw/Components/SunSensor/SunSensorComponentAc.hpp"

namespace Components {

  class SunSensor final : public SunSensorComponentBase {
    public:
      SunSensor(const char* const compName);
      ~SunSensor();

    private:
      // Matching the SystemResources signature exactly
      void run_handler(
          const FwIndexType portNum,
          U32 context
      );
  };

}

#endif