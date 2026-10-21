// ======================================================================
// \title  FlatSatTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <FlatSat/Top/FlatSatTopologyAc.hpp>
// Note: Uncomment when using Svc:TlmPacketizer
//#include <FlatSat/Top/FlatSatPacketsAc.hpp>

// Necessary project-specified types
#include <Fw/Logger/Logger.hpp>
#include <Fw/Types/MallocAllocator.hpp>

// Public functions for use in main program are namespaced with deployment module FlatSat
// This is also the namespace where the topology components are instantiated by FPP.
namespace FlatSat {

// Instantiate a malloc allocator for cmdSeq buffer allocation
Fw::MallocAllocator mallocator;

// The reference topology divides the incoming clock signal (1Hz) into sub-signals: 1Hz, 1/2Hz, and 1/4Hz with 0 offset
Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {2, 0}, {4, 0}}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
U32 rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
U32 rateGroup2Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
U32 rateGroup3Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

enum TopologyConstants {
    COMM_PRIORITY = 34,
    EXT_UART_PRIORITY = 33,
    EXT_UART_BUFFER_MGR_ID = 300,     //!< Must differ from the other buffer managers' IDs
    EXT_UART_BUFFER_SIZE = 256,       //!< Max bytes per driver read
    EXT_UART_BUFFER_COUNT = 10,
};

// Set once the external UART read thread is running, so teardown only stops what was started
bool extUartStarted = false;

/**
 * \brief open the external board UART if a device was given on the command line
 */
bool openExternalUart(const TopologyState& state) {
    if (state.uartDevice == nullptr) {
        return false;
    }
    Drv::LinuxUartDriver::UartBaudRate baud;
    switch (state.uartBaud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
        case 230400:
        case 460800:
        case 921600:
            // UartBaudRate enumerators are equal to their baud rate
            baud = static_cast<Drv::LinuxUartDriver::UartBaudRate>(state.uartBaud);
            break;
        default:
            Fw::Logger::log("[ERROR] Unsupported UART baud rate %" PRIu32 "\n", state.uartBaud);
            return false;
    }
    // On failure the driver emits an OpenError event with the reason
    return extUart.open(state.uartDevice, baud, Drv::LinuxUartDriver::NO_FLOW, Drv::LinuxUartDriver::PARITY_NONE,
                        EXT_UART_BUFFER_SIZE);
}

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology() {
    // Rate group driver needs a divisor list
    rateGroupDriver.configure(rateGroupDivisorsSet);

    // Rate groups require context arrays.
    rateGroup1.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));
    rateGroup2.configure(rateGroup2Context, FW_NUM_ARRAY_ELEMENTS(rateGroup2Context));
    rateGroup3.configure(rateGroup3Context, FW_NUM_ARRAY_ELEMENTS(rateGroup3Context));

    // Command sequencer needs to allocate memory to hold contents of command sequences
    cmdSeq.allocateBuffer(0, mallocator, 5 * 1024);

    // Receive buffers for the external board UART
    Svc::BufferManager::BufferBins extUartBins{};
    extUartBins.bins[0].bufferSize = EXT_UART_BUFFER_SIZE;
    extUartBins.bins[0].numBuffers = EXT_UART_BUFFER_COUNT;
    extUartBufferManager.setup(EXT_UART_BUFFER_MGR_ID, 0, mallocator, extUartBins);
}

void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    if (state.hostname != nullptr && state.port != 0) {
        comDriver.configure(state.hostname, state.port);
    }
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    const bool extUartOpen = openExternalUart(state);
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);
    // Initialize socket communication if and only if there is a valid specification
    if (state.hostname != nullptr && state.port != 0) {
        Os::TaskString name("ReceiveTask");
        // Uplink is configured for receive so a socket task is started
        comDriver.start(name, COMM_PRIORITY, Default::STACK_SIZE);
    }
    // Read thread for the external board UART
    if (extUartOpen) {
        extUart.start(EXT_UART_PRIORITY, Default::STACK_SIZE);
        extUartStarted = true;
    }
}

void startRateGroups(const Fw::TimeInterval& interval) {
    // The timer component drives the fundamental tick rate of the system.
    // Svc::RateGroupDriver will divide this down to the slower rate groups.
    // This call will block until the stopRateGroups() call is made.
    // For this Linux demo, that call is made from a signal handler.
    timer.startTimer(interval);
}

void stopRateGroups() {
    timer.quit();
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);

    // Other task clean-up.
    comDriver.stop();
    (void)comDriver.join();
    if (extUartStarted) {
        extUart.quitReadThread();
        (void)extUart.join();
    }

    // Resource deallocation
    cmdSeq.deallocateBuffer(mallocator);
    extUartBufferManager.cleanup();

    tearDownComponents(state);
}
};  // namespace FlatSat
