// ======================================================================
// \title  ModeManager.cpp
// \author skn
// \brief  cpp file for ModeManager component implementation class
// ======================================================================

#include "LionessSw/Components/ModeManager/ModeManager.hpp"

using namespace LionessSw;

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

ModeManager ::ModeManager(const char* const compName) : ModeManagerComponentBase(compName) {}

ModeManager ::~ModeManager() {}


// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void ModeManager ::getMode_handler(FwIndexType portNum, LionessSw::MODE& mode) {
    mode = this->currMode;
}

void ModeManager ::schedIn_handler(FwIndexType portNum, U32 context) {
    if(this->isConnected_getBattery_OutputPort(0)) {
        this->getBattery_out(0, this->currBattery);
    }
    if(this->isConnected_getAcceleration_OutputPort(0)) {
        this->getAcceleration_out(0, this->currAcceleration);
    }

    this->tlmWrite_BatteryLevel(this->currBattery);
    this->tlmWrite_Acceleration(this->currAcceleration);

    if(this->currBattery < this->SAFE_BATTERY_THRESHOLD && this->currMode != MODE::SAFE) {
        this->log_WARNING_HI_BatteryLow(this->currBattery, this->SAFE_BATTERY_THRESHOLD);
        this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::SAFE, Fw::LogStringArg("battery low"));
        this->currMode = MODE::SAFE;
        this->tlmWrite_CurrentMode(this->currMode);
        return;
    }

    if(this->currAcceleration > this->SAFE_ACCELERATION_THRESHOLD && this->currMode != MODE::SAFE) {
        this->log_WARNING_HI_HighSpinRate(this->currAcceleration, this->SAFE_ACCELERATION_THRESHOLD);
        this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::SAFE, Fw::LogStringArg("spin rate high"));
        this->currMode = MODE::SAFE;
        this->tlmWrite_CurrentMode(this->currMode);
    }
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void ModeManager ::ToStandby_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    switch(this->currMode) {
         case MODE::SAFE:
	 case MODE::EXPERIMENT:
	 case MODE::TRANSMIT:
            this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::STANDBY, Fw::LogStringArg("cmd"));
            this->currMode = MODE::STANDBY;
            this->tlmWrite_CurrentMode(this->currMode);
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
            break;

       default:
            this->log_WARNING_HI_InvalidModeChange(this->currMode, MODE::STANDBY, Fw::LogStringArg("cmd"));
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            break;
    }
}

void ModeManager ::ToExperiment_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    switch(this->currMode) {
        case MODE::STANDBY:
            if(this->currBattery > this->SAFE_BATTERY_THRESHOLD) {
                this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::EXPERIMENT, Fw::LogStringArg("cmd"));
                this->currMode = MODE::EXPERIMENT;
                this->tlmWrite_CurrentMode(this->currMode);
                this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
            }
            else {
                this->log_WARNING_HI_BatteryLow(this->currBattery, this->SAFE_BATTERY_THRESHOLD);
                this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            }
            break;

        default:
            this->log_WARNING_HI_InvalidModeChange(this->currMode, MODE::EXPERIMENT, Fw::LogStringArg("cmd"));
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            break;
    }
}

void ModeManager ::ToTransmit_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    switch(this->currMode) {
        case MODE::STANDBY:
            if(this->currBattery > this->SAFE_BATTERY_THRESHOLD) {
                this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::TRANSMIT, Fw::LogStringArg("cmd"));
                this->currMode = MODE::TRANSMIT;
                this->tlmWrite_CurrentMode(this->currMode);
                this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
            }
            else {
                this->log_WARNING_HI_InvalidModeChange(this->currMode, MODE::EXPERIMENT, Fw::LogStringArg("cmd"));
                this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            }
            break;

        default:
            this->log_WARNING_HI_InvalidModeChange(this->currMode, MODE::TRANSMIT, Fw::LogStringArg("cmd"));
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            break;
    }
}

void ModeManager ::ToDetumble_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    switch(this->currMode) {
        case MODE::STANDBY:
            this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::DETUMBLE, Fw::LogStringArg("cmd"));
            this->currMode = MODE::DETUMBLE;
            this->tlmWrite_CurrentMode(this->currMode);
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
            break;

        default:
            this->log_WARNING_HI_InvalidModeChange(this->currMode, MODE::DETUMBLE, Fw::LogStringArg("cmd"));
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            break;
    }
}

void ModeManager ::ToSafe_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    switch(this->currMode) {
        case MODE::STANDBY:
        case MODE::EXPERIMENT:
        case MODE::TRANSMIT:
        case MODE::DETUMBLE:
            this->log_ACTIVITY_HI_ModeChanged(this->currMode, MODE::SAFE, Fw::LogStringArg("cmd"));
            this->currMode = MODE::SAFE;
            this->tlmWrite_CurrentMode(this->currMode);
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
            break;

        default:
            this->log_WARNING_HI_InvalidModeChange(this->currMode, MODE::SAFE, Fw::LogStringArg("cmd"));
            this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
            break;
    }
}

}  // namespace Components
