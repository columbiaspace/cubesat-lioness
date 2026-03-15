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
      void calculateSunVector();     // compute 3D sun vector from current photodiode intensities
      F32 m_diode_readings[6];       // Index Mapping: 0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z

      // similar to SystemResources signature
      void run_handler(
          const FwIndexType portNum,
          U32 context
      );
  };

}

#endif