module Components {
    @ Receives raw bytes from an external board through a byte stream driver (e.g. Drv.LinuxUartDriver)
    passive component UartReceiver {

        @ Driver ready signal, received data, and buffer return to the driver
        import Drv.PassiveByteStreamDriverClientReadyRecv

        @ Rate group tick: publishes telemetry and refills the hex dump budget
        sync input port schedIn: Svc.Sched

        @ Total bytes received
        telemetry BytesReceived: U64 id 0

        @ Number of non-empty reads delivered by the driver
        telemetry ChunksReceived: U32 id 1

        @ Number of failed reads reported by the driver
        telemetry RecvErrors: U32 id 2

        event UartReady() \
            severity activity high id 0 format "UART driver ready"

        event DataReceived(
            $size: U32 @< Bytes in this chunk
            data: string size 100 @< Hex of the first bytes of the chunk
        ) severity activity low id 1 format "RX {} bytes: {}"

        event RecvError(
            status: Drv.ByteStreamStatus
        ) severity warning low id 2 format "UART receive failed: {}" throttle 10

        ###############################################################################
        # Standard AC Ports: Required for Channels and Events                         #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Enables event handling
        import Fw.Event

        @ Enables telemetry channels handling
        import Fw.Channel

    }
}
