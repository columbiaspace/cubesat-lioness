// ======================================================================
// \title  FlatSatTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <LionessSw/FlatSat/Top/FlatSatTopologyAc.hpp>
// Note: Uncomment when using Svc:TlmPacketizer
//#include <LionessSw/FlatSat/Top/FlatSatPacketsAc.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>

#include <cstdio>

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
    // Receive-buffer allocation size for the UART drivers
    UART_ALLOCATION_SIZE = 1024,
};

// Map a numeric baud rate coming in from the command line onto the
// LinuxUartDriver::UartBaudRate enum. The set of values handled here matches
// the enum declared in Drv/LinuxUartDriver/LinuxUartDriver.hpp.
static Drv::LinuxUartDriver::UartBaudRate mapBaudRate(U32 rate) {
    switch (rate) {
        case 9600:    return Drv::LinuxUartDriver::BAUD_9600;
        case 19200:   return Drv::LinuxUartDriver::BAUD_19200;
        case 38400:   return Drv::LinuxUartDriver::BAUD_38400;
        case 57600:   return Drv::LinuxUartDriver::BAUD_57600;
        case 115200:  return Drv::LinuxUartDriver::BAUD_115K;
        case 230400:  return Drv::LinuxUartDriver::BAUD_230K;
#ifdef TGT_OS_TYPE_LINUX
        case 460800:  return Drv::LinuxUartDriver::BAUD_460K;
        case 921600:  return Drv::LinuxUartDriver::BAUD_921K;
        case 1000000: return Drv::LinuxUartDriver::BAUD_1000K;
#endif
        default:
            (void)printf("Unsupported baud %u, falling back to 115200\n", static_cast<unsigned>(rate));
            return Drv::LinuxUartDriver::BAUD_115K;
    }
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
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);

    // Open and start the two UART-backed com stacks. Each is optional so the
    // deployment can still run (e.g. for local testing) when a given device
    // path is not available on the host.
    const Drv::LinuxUartDriver::UartBaudRate baud = mapBaudRate(state.baudRate);

    if (state.uartDevice != nullptr) {
        const bool opened = comDriver.open(state.uartDevice,
                                           baud,
                                           Drv::LinuxUartDriver::NO_FLOW,
                                           Drv::LinuxUartDriver::PARITY_NONE,
                                           UART_ALLOCATION_SIZE);
        if (opened) {
            comDriver.start(COMM_PRIORITY, Default::STACK_SIZE);
        } else {
            (void)printf("Failed to open primary UART device: %s\n", state.uartDevice);
        }
    }

    if (state.roarfmUartDevice != nullptr) {
        const bool opened = roarfmDriver.open(state.roarfmUartDevice,
                                              baud,
                                              Drv::LinuxUartDriver::NO_FLOW,
                                              Drv::LinuxUartDriver::PARITY_NONE,
                                              UART_ALLOCATION_SIZE);
        if (opened) {
            roarfmDriver.start(COMM_PRIORITY, Default::STACK_SIZE);
        } else {
            (void)printf("Failed to open RoarFM UART device: %s\n", state.roarfmUartDevice);
        }
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

    // Stop and join the UART read tasks before tearing down the rest of the topology.
    comDriver.quitReadThread();
    (void)comDriver.join();
    roarfmDriver.quitReadThread();
    (void)roarfmDriver.join();

    // Resource deallocation
    cmdSeq.deallocateBuffer(mallocator);

    tearDownComponents(state);
}
};  // namespace FlatSat
