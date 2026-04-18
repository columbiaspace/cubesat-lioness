####
# ComCcsdsRoarFM/CMakeLists.txt:
#
# Project-level CCSDS communications subtopology paired with the Roar FM radio
# (currently stubbed out with a UART driver for bring-up). Mirrors the shape of
# the core Svc::Subtopologies::ComCcsds subtopology so that it can be imported
# side-by-side with it, enabling a split communications stack.
####
register_fprime_module(
    EXCLUDE_FROM_ALL
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/ComCcsdsRoarFM.fpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/SubtopologyTopologyDefs.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/PingEntries.hpp"
    DEPENDS
        Svc_Subtopologies_ComCcsds_ComCcsdsConfig
    INTERFACE
)
