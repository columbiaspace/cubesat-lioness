// ======================================================================
// \title  EPS_Sim.cpp
// \author xiwei
// \brief  cpp file for EPS_Sim component implementation class
// ======================================================================

#include "LionessSw/Components/EPS_Sim/EPS_Sim.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

EPS_Sim ::EPS_Sim(const char* const compName, F32 delta) : EPS_SimComponentBase(compName) {
    this->_delta = delta;
    _opMode = Components::OpMode::OFF;
    _batteryLevel = 100;
}

EPS_Sim ::~EPS_Sim() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void EPS_Sim ::getBattery_handler(FwIndexType portNum, F32 &batteryLevel) {
    batteryLevel = _batteryLevel;
}

void EPS_Sim ::run_handler(FwIndexType portNum, U32 context) {
    if (_opMode != Components::OpMode::OFF) {
        _batteryLevel += _opMode * _delta; // _opMode is negative while in draining mode, and positive while in charging mode

        if (_batteryLevel < 0) _batteryLevel = 0; // These lines just cap the battery level at realistic levels
        else if (_batteryLevel > 100) _batteryLevel = 100;
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void EPS_Sim ::SET_EPS_SIM_OPMODE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Components::OpMode opMode) {
    _opMode = opMode;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void EPS_Sim ::SET_EPS_SIM_BATTERY_LEVEL_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, F32 batteryLevel) {
    _batteryLevel = batteryLevel;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
