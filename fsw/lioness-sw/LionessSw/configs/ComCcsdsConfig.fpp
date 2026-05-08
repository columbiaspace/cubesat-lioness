####
# ComCcsdsConfig.fpp
#
# Project-level override of the fprime core ComCcsds subtopology configuration
# (lib/fprime/Svc/Subtopologies/ComCcsds/ComCcsdsConfig/ComCcsdsConfig.fpp).
#
# Preserves all original constants (required by the core ComCcsds subtopology)
# and adds BASE_ID_ROARFM so a second, project-level ComCcsdsRoarFM subtopology
# can share the same configuration module without colliding with the core
# subtopology's base IDs.
####
module ComCcsdsConfig {
    # Base ID for the (core) ComCcsds Subtopology
    constant BASE_ID = 0x02000000
    # Base ID for the project-level ComCcsdsRoarFM Subtopology
    constant BASE_ID_ROARFM = 0x03000000

    module QueueSizes {
        constant comQueue    = 50
        constant aggregator  = 10
    }

    module StackSizes {
        constant comQueue   = 64 * 1024
        constant aggregator = 64 * 1024
    }

    module Priorities {
        constant aggregator = 30
        constant comQueue   = 29
    }

    # Queue configuration constants
    module QueueDepths {
        constant events      = 200
        constant tlm         = 500
        constant file        = 100
    }

    module QueuePriorities {
        constant events      = 0
        constant tlm         = 2
        constant file        = 1
    }

    # Buffer management constants
    module BuffMgr {
        constant frameAccumulatorSize  = 2048
        constant commsBuffSize         = 2048
        constant commsFileBuffSize     = 3000
        constant commsBuffCount        = 20
        constant commsFileBuffCount    = 30
        # BufferManager instance ID for the core ComCcsds stack
        constant commsBuffMgrId        = 200
        # BufferManager instance ID for the ComCcsdsRoarFM stack
        constant commsBuffMgrIdRoarFM  = 201
    }
}
