// ======================================================================
// \title  SubtopologyTopologyDefs.hpp
// \brief  Required topology-level definitions for the ComCcsdsRoarFM subtopology
//
// Provides the ComCcsdsRoarFM::SubtopologyState type referenced by the
// deployment's TopologyState. The RoarFM stack does not currently require
// any runtime state (it is wired up behind a UART driver for bring-up),
// so the struct is intentionally empty - it exists only to match the
// pattern used by the other CCSDS subtopologies.
// ======================================================================
#ifndef COMCCSDSROARFM_SUBTOPOLOGY_DEFS_HPP
#define COMCCSDSROARFM_SUBTOPOLOGY_DEFS_HPP

#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/BufferManager/BufferManager.hpp>
#include <Svc/FrameAccumulator/FrameDetector/CcsdsTcFrameDetector.hpp>

#include "Svc/Subtopologies/ComCcsds/ComCcsdsConfig/ComCcsdsSubtopologyConfig.hpp"
#include "Svc/Subtopologies/ComCcsds/ComCcsdsConfig/FppConstantsAc.hpp"

namespace ComCcsdsRoarFM {
struct SubtopologyState {
    // Empty - no external state needed for ComCcsdsRoarFM subtopology
};

struct TopologyState {
    SubtopologyState comCcsdsRoarFM;
};
}  // namespace ComCcsdsRoarFM

#endif
