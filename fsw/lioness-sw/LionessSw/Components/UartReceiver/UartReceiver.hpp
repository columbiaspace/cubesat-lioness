// ======================================================================
// \title  UartReceiver.hpp
// \brief  hpp file for UartReceiver component implementation class
// ======================================================================

#ifndef Components_UartReceiver_HPP
#define Components_UartReceiver_HPP

#include <atomic>

#include "LionessSw/Components/UartReceiver/UartReceiverComponentAc.hpp"

namespace Components {

class UartReceiver final : public UartReceiverComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct UartReceiver object
    UartReceiver(const char* const compName  //!< The component name
    );

    //! Destroy UartReceiver object
    ~UartReceiver();

  private:
    //! Max bytes of each chunk shown in a DataReceived event
    static constexpr FwSizeType HEX_DUMP_BYTES = 32;
    //! Max DataReceived events per schedIn tick, so a fast stream can't flood the event log
    static constexpr U32 HEX_DUMPS_PER_TICK = 10;

    //! Handle bytes received from the external board
    void processBytes(const U8* data, FwSizeType size);

  private:
    // Written from the driver's read thread, read from the rate group thread
    std::atomic<U64> m_bytesReceived{0};
    std::atomic<U32> m_chunksReceived{0};
    std::atomic<U32> m_recvErrors{0};
    std::atomic<U32> m_hexDumpBudget{HEX_DUMPS_PER_TICK};

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for byteStreamDriverReady
    void byteStreamDriverReady_handler(FwIndexType portNum  //!< The port number
                                       ) override;

    //! Handler implementation for fromByteStreamDriver
    void fromByteStreamDriver_handler(FwIndexType portNum,                //!< The port number
                                      Fw::Buffer& buffer,                 //!< The received data
                                      const Drv::ByteStreamStatus& status  //!< The receive status
                                      ) override;

    //! Handler implementation for schedIn
    void schedIn_handler(FwIndexType portNum,  //!< The port number
                         U32 context           //!< The call order
                         ) override;
};

}  // namespace Components

#endif
