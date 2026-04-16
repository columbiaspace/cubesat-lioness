// ======================================================================
// \title  EPS_Sim.hpp
// \author xiwei
// \brief  hpp file for EPS_Sim component implementation class
// ======================================================================

#ifndef Components_EPS_Sim_HPP
#define Components_EPS_Sim_HPP

#include "LionessSw/Components/EPS_Sim/EPS_SimComponentAc.hpp"

namespace Components {

class EPS_Sim final : public EPS_SimComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct EPS_Sim object
    EPS_Sim(const char* const compName, F32 delta);

    //! Destroy EPS_Sim object
    ~EPS_Sim();

  private:
    Components::OpMode _opMode;
    F32 _delta;
    F32 _batteryLevel;
    
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for getBattery
    void getBattery_handler(FwIndexType portNum, F32 &batteryLevel) override;

    //! Handler implementation for run
    void run_handler(FwIndexType portNum, U32 context) override;

    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command SET_EPS_SIM_OPMODE
    //!
    //! Command for starting simulation
    void SET_EPS_SIM_OPMODE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Components::OpMode opMode) override;

    //! Handler implementation for command SET_EPS_SIM_BATTERY_LEVEL
    //!
    //! Command for changing the Simulation Mode
    void SET_EPS_SIM_BATTERY_LEVEL_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, F32 batteryLevel) override;
};

}  // namespace Components

#endif
