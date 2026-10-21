// ======================================================================
// \title  UartReceiver.cpp
// \brief  cpp file for UartReceiver component implementation class
// ======================================================================

#include "LionessSw/Components/UartReceiver/UartReceiver.hpp"

#include <cstdio>

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

UartReceiver ::UartReceiver(const char* const compName) : UartReceiverComponentBase(compName) {}

UartReceiver ::~UartReceiver() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void UartReceiver ::byteStreamDriverReady_handler(FwIndexType portNum) {
    this->log_ACTIVITY_HI_UartReady();
}

void UartReceiver ::fromByteStreamDriver_handler(FwIndexType portNum,
                                                 Fw::Buffer& buffer,
                                                 const Drv::ByteStreamStatus& status) {
    // Called on the driver's read thread. The driver delivers whatever bytes arrived since the last read, so one
    // message from the board may be split across calls or several may arrive in one.
    if (status == Drv::ByteStreamStatus::OP_OK) {
        if (buffer.getSize() > 0) {
            this->m_bytesReceived += buffer.getSize();
            this->m_chunksReceived++;
            this->processBytes(buffer.getData(), buffer.getSize());
        }
    } else {
        this->m_recvErrors++;
        this->log_WARNING_LO_RecvError(status);
    }
    // Every buffer must go back to the driver, including failed reads, or its buffer pool runs dry
    this->fromByteStreamDriverReturn_out(0, buffer);
}

void UartReceiver ::schedIn_handler(FwIndexType portNum, U32 context) {
    this->tlmWrite_BytesReceived(this->m_bytesReceived);
    this->tlmWrite_ChunksReceived(this->m_chunksReceived);
    this->tlmWrite_RecvErrors(this->m_recvErrors);
    this->m_hexDumpBudget = HEX_DUMPS_PER_TICK;
}

// ----------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------

void UartReceiver ::processBytes(const U8* data, FwSizeType size) {
    // Replace or extend this with frame parsing for the specific board
    if (this->m_hexDumpBudget == 0) {
        return;
    }
    this->m_hexDumpBudget--;

    char hex[3 * HEX_DUMP_BYTES + 4] = {};
    const FwSizeType shown = (size < HEX_DUMP_BYTES) ? size : HEX_DUMP_BYTES;
    FwSizeType pos = 0;
    for (FwSizeType i = 0; i < shown; i++) {
        pos += static_cast<FwSizeType>(
            snprintf(&hex[pos], sizeof(hex) - pos, (i == 0) ? "%02X" : " %02X", data[i]));
    }
    if (shown < size) {
        (void)snprintf(&hex[pos], sizeof(hex) - pos, " ...");
    }
    Fw::LogStringArg hexArg(hex);
    this->log_ACTIVITY_LO_DataReceived(static_cast<U32>(size), hexArg);
}

}  // namespace Components
