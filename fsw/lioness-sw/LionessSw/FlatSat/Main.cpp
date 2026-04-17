// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for CLI-based systems (Linux, macOS)
//
// ======================================================================
// Used to access topology functions
#include <LionessSw/FlatSat/Top/FlatSatTopology.hpp>
// OSAL initialization
#include <Os/Os.hpp>
// Used for signal handling shutdown
#include <signal.h>
// Used for command line argument processing
#include <getopt.h>
// Used for printf functions
#include <cstdlib>
#include <cstdio>

/**
 * \brief print command line help message
 *
 * This will print a command line help message including the available command line arguments.
 *
 * @param app: name of application
 */
void print_usage(const char* app) {
    (void)printf(
        "Usage: ./%s [options]\n"
        "  -u <path>   Primary  (ComCcsds)        UART device path (default: /dev/ttyUSB0)\n"
        "  -r <path>   Secondary (ComCcsdsRoarFM) UART device path (default: /dev/ttyUSB1)\n"
        "  -b <rate>   UART baud rate for both devices (default: 115200)\n"
        "  -h          Show this help message\n",
        app);
}

/**
 * \brief shutdown topology cycling on signal
 *
 * The reference topology allows for a simulated cycling of the rate groups. This simulated cycling needs to be stopped
 * in order for the program to shutdown. This is done via handling signals such that it is performed via Ctrl-C
 *
 * @param signum
 */
static void signalHandler(int signum) {
    FlatSat::stopRateGroups();
}

/**
 * \brief execute the program
 *
 * This F' program drives a FlatSat deployment with a split communications
 * stack: the core ComCcsds subtopology is fed by a primary UART driver and a
 * project-level ComCcsdsRoarFM subtopology is fed by a secondary UART driver
 * (the latter stands in for the future Roar FM radio during bring-up).
 *
 * Device paths for the two UART links and the shared baud rate are passed in
 * via command line flags; sensible defaults are used for values not supplied.
 *
 * @param argc: argument count supplied to program
 * @param argv: argument values supplied to program
 * @return: 0 on success, something else on failure
 */
int main(int argc, char* argv[]) {
    I32 option = 0;
    const char* uartDevice = "/dev/ttyUSB0";
    const char* roarfmUartDevice = "/dev/ttyUSB1";
    U32 baudRate = 115200;

    Os::init();

    while ((option = getopt(argc, argv, "hu:r:b:")) != -1) {
        switch (option) {
            case 'u':
                uartDevice = optarg;
                break;
            case 'r':
                roarfmUartDevice = optarg;
                break;
            case 'b':
                baudRate = static_cast<U32>(std::atoi(optarg));
                break;
            case 'h':
            case '?':
            default:
                print_usage(argv[0]);
                return (option == 'h') ? 0 : 1;
        }
    }

    // Object for communicating state to the topology
    FlatSat::TopologyState inputs;
    inputs.uartDevice = uartDevice;
    inputs.roarfmUartDevice = roarfmUartDevice;
    inputs.baudRate = baudRate;

    // Setup program shutdown via Ctrl-C
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    (void)printf("Primary UART:  %s @ %u baud\n", uartDevice, static_cast<unsigned>(baudRate));
    (void)printf("RoarFM UART:   %s @ %u baud\n", roarfmUartDevice, static_cast<unsigned>(baudRate));
    (void)printf("Hit Ctrl-C to quit\n");

    // Setup, cycle, and teardown topology
    FlatSat::setupTopology(inputs);
    FlatSat::startRateGroups(Fw::TimeInterval(1, 0));  // Program loop cycling rate groups at 1Hz
    FlatSat::teardownTopology(inputs);
    (void)printf("Exiting...\n");
    return 0;
}
