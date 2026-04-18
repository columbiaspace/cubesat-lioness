// ======================================================================
// \title  FlatSatTopologyDefs.hpp
// \brief required header file containing the required definitions for the topology autocoder
//
// ======================================================================
#ifndef FLATSAT_FLATSATTOPOLOGYDEFS_HPP
#define FLATSAT_FLATSATTOPOLOGYDEFS_HPP

// Subtopology PingEntries includes
#include "Svc/Subtopologies/CdhCore/PingEntries.hpp"
#include "Svc/Subtopologies/ComCcsds/PingEntries.hpp"
#include "Svc/Subtopologies/DataProducts/PingEntries.hpp"
#include "Svc/Subtopologies/FileHandling/PingEntries.hpp"
#include "LionessSw/ComCcsdsRoarFM/PingEntries.hpp"

// SubtopologyTopologyDefs includes
#include "Svc/Subtopologies/CdhCore/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/ComCcsds/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/DataProducts/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/FileHandling/SubtopologyTopologyDefs.hpp"
#include "LionessSw/ComCcsdsRoarFM/SubtopologyTopologyDefs.hpp"

//ComCcsds Enum Includes
#include "Svc/Subtopologies/ComCcsds/Ports_ComPacketQueueEnumAc.hpp"
#include "Svc/Subtopologies/ComCcsds/Ports_ComBufferQueueEnumAc.hpp"

// Include autocoded FPP constants
#include "LionessSw/FlatSat/Top/FppConstantsAc.hpp"

/**
 * \brief required ping constants
 *
 * The topology autocoder requires a WARN and FATAL constant definition for each component that supports the health-ping
 * interface. These are expressed as enum constants placed in a namespace named for the component instance. These
 * are all placed in the PingEntries namespace.
 *
 * Each constant specifies how many missed pings are allowed before a WARNING_HI/FATAL event is triggered. In the
 * following example, the health component will emit a WARNING_HI event if the component instance cmdDisp does not
 * respond for 3 pings and will FATAL if responses are not received after a total of 5 pings.
 *
 * ```c++
 * namespace PingEntries {
 * namespace cmdDisp {
 *     enum { WARN = 3, FATAL = 5 };
 * }
 * }
 * ```
 */
namespace PingEntries {
    namespace FlatSat_rateGroup1 {enum { WARN = 3, FATAL = 5 };}
    namespace FlatSat_rateGroup2 {enum { WARN = 3, FATAL = 5 };}
    namespace FlatSat_rateGroup3 {enum { WARN = 3, FATAL = 5 };}
    namespace FlatSat_cmdSeq {enum { WARN = 3, FATAL = 5 };}
}  // namespace PingEntries

// Definitions are placed within the same namespace as the FPP module that contains the topology.
namespace FlatSat {

/**
 * \brief required type definition to carry state
 *
 * The topology autocoder requires an object that carries state with the name `FlatSat::TopologyState`. Only the type
 * definition is required by the autocoder and the contents of this object are otherwise opaque to the autocoder. The
 * contents are entirely up to the definition of the project. This deployment uses subtopologies.
 */
struct TopologyState {
    //! UART device path for the primary ComCcsds stack (e.g. "/dev/ttyUSB0")
    const char* uartDevice;
    //! UART device path for the ComCcsdsRoarFM stack (stands in for the
    //! future Roar FM radio, e.g. "/dev/ttyUSB1")
    const char* roarfmUartDevice;
    //! Baud rate used for both UART devices above
    U32 baudRate;
    CdhCore::SubtopologyState cdhCore;                 //!< Subtopology state for CdhCore
    ComCcsds::SubtopologyState comCcsds;               //!< Subtopology state for ComCcsds
    ComCcsdsRoarFM::SubtopologyState comCcsdsRoarFM;   //!< Subtopology state for ComCcsdsRoarFM
    DataProducts::SubtopologyState dataProducts;       //!< Subtopology state for DataProducts
    FileHandling::SubtopologyState fileHandling;       //!< Subtopology state for FileHandling
};

namespace PingEntries = ::PingEntries;
}  // namespace FlatSat

#endif
