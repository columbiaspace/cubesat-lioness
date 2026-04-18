####
# ComCcsdsRoarFM.fpp
#
# Project-level CCSDS communications subtopology intended to host the
# (future) Roar FM custom radio. For now the radio is stubbed with a
# plain UART driver wired up by the deployment topology, but the
# subtopology itself is an intentional copy of the core ComCcsds stack
# so that it can coexist with the core ComCcsds subtopology and be
# split/fan-out via ComSplitter at the deployment level.
#
# Base IDs are sourced from the project-level ComCcsdsConfig override
# (see configs/ComCcsdsConfig.fpp) via BASE_ID_ROARFM.
####
module ComCcsdsRoarFM {

    # ----------------------------------------------------------------------
    # Active Components
    # ----------------------------------------------------------------------
    instance comQueue: Svc.ComQueue base id ComCcsdsConfig.BASE_ID_ROARFM + 0x00000 \
        queue size ComCcsdsConfig.QueueSizes.comQueue \
        stack size ComCcsdsConfig.StackSizes.comQueue \
        priority ComCcsdsConfig.Priorities.comQueue \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        using namespace ComCcsdsRoarFM;
        Svc::ComQueue::QueueConfigurationTable configurationTableRoarFM;

        // Events (highest-priority)
        configurationTableRoarFM.entries[ComCcsds::Ports_ComPacketQueue::EVENTS].depth = ComCcsdsConfig::QueueDepths::events;
        configurationTableRoarFM.entries[ComCcsds::Ports_ComPacketQueue::EVENTS].priority = ComCcsdsConfig::QueuePriorities::events;

        // Telemetry
        configurationTableRoarFM.entries[ComCcsds::Ports_ComPacketQueue::TELEMETRY].depth = ComCcsdsConfig::QueueDepths::tlm;
        configurationTableRoarFM.entries[ComCcsds::Ports_ComPacketQueue::TELEMETRY].priority = ComCcsdsConfig::QueuePriorities::tlm;

        // File Downlink Queue (buffer queue using NUM_CONSTANTS offset)
        configurationTableRoarFM.entries[ComCcsds::Ports_ComPacketQueue::NUM_CONSTANTS + ComCcsds::Ports_ComBufferQueue::FILE].depth = ComCcsdsConfig::QueueDepths::file;
        configurationTableRoarFM.entries[ComCcsds::Ports_ComPacketQueue::NUM_CONSTANTS + ComCcsds::Ports_ComBufferQueue::FILE].priority = ComCcsdsConfig::QueuePriorities::file;

        // Allocation identifier is 0 as the MallocAllocator discards it
        ComCcsdsRoarFM::comQueue.configure(configurationTableRoarFM, 0, ComCcsds::Allocation::memAllocator);
        """
        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCcsdsRoarFM::comQueue.cleanup();
        """
    }

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance frameAccumulator: Svc.FrameAccumulator base id ComCcsdsConfig.BASE_ID_ROARFM + 0x01000 \
    {

        phase Fpp.ToCpp.Phases.configObjects """
        Svc::FrameDetectors::CcsdsTcFrameDetector frameDetector;
        """
        phase Fpp.ToCpp.Phases.configComponents """
        ComCcsdsRoarFM::frameAccumulator.configure(
            ConfigObjects::ComCcsdsRoarFM_frameAccumulator::frameDetector,
            1,
            ComCcsds::Allocation::memAllocator,
            ComCcsdsConfig::BuffMgr::frameAccumulatorSize
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCcsdsRoarFM::frameAccumulator.cleanup();
        """
    }

    instance commsBufferManager: Svc.BufferManager base id ComCcsdsConfig.BASE_ID_ROARFM + 0x02000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::BufferManager::BufferBins bins;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        memset(&ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins, 0, sizeof(ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins));
        ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins.bins[0].bufferSize = ComCcsdsConfig::BuffMgr::commsBuffSize;
        ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins.bins[0].numBuffers = ComCcsdsConfig::BuffMgr::commsBuffCount;
        ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins.bins[1].bufferSize = ComCcsdsConfig::BuffMgr::commsFileBuffSize;
        ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins.bins[1].numBuffers = ComCcsdsConfig::BuffMgr::commsFileBuffCount;
        ComCcsdsRoarFM::commsBufferManager.setup(
            ComCcsdsConfig::BuffMgr::commsBuffMgrIdRoarFM,
            0,
            ComCcsds::Allocation::memAllocator,
            ConfigObjects::ComCcsdsRoarFM_commsBufferManager::bins
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCcsdsRoarFM::commsBufferManager.cleanup();
        """
    }

    instance fprimeRouter: Svc.FprimeRouter base id ComCcsdsConfig.BASE_ID_ROARFM + 0x03000

    instance tcDeframer: Svc.Ccsds.TcDeframer base id ComCcsdsConfig.BASE_ID_ROARFM + 0x04000

    instance spacePacketDeframer: Svc.Ccsds.SpacePacketDeframer base id ComCcsdsConfig.BASE_ID_ROARFM + 0x05000

    instance aggregator: Svc.ComAggregator base id ComCcsdsConfig.BASE_ID_ROARFM + 0x06000 \
        queue size ComCcsdsConfig.QueueSizes.aggregator \
        stack size ComCcsdsConfig.StackSizes.aggregator

    # NOTE: name 'framer' is used for the framer that connects to the Com Adapter Interface for better subtopology interoperability
    instance framer: Svc.Ccsds.TmFramer base id ComCcsdsConfig.BASE_ID_ROARFM + 0x07000

    instance spacePacketFramer: Svc.Ccsds.SpacePacketFramer base id ComCcsdsConfig.BASE_ID_ROARFM + 0x08000

    instance apidManager: Svc.Ccsds.ApidManager base id ComCcsdsConfig.BASE_ID_ROARFM + 0x09000

    instance comStub: Svc.ComStub base id ComCcsdsConfig.BASE_ID_ROARFM + 0x0A000

    topology FramingSubtopology {
        # Usage Note:
        #
        # When importing this subtopology, users shall establish 5 port connections with a component implementing
        # the Svc.Com (Svc/Interfaces/Com.fpp) interface. They are as follows:
        #
        # 1) Outputs:
        #     - ComCcsdsRoarFM.framer.dataOut                 -> [Svc.Com].dataIn
        #     - ComCcsdsRoarFM.frameAccumulator.dataReturnOut -> [Svc.Com].dataReturnIn
        # 2) Inputs:
        #     - [Svc.Com].dataReturnOut -> ComCcsdsRoarFM.framer.dataReturnIn
        #     - [Svc.Com].comStatusOut  -> ComCcsdsRoarFM.framer.comStatusIn
        #     - [Svc.Com].dataOut       -> ComCcsdsRoarFM.frameAccumulator.dataIn


        # Active Components
        instance comQueue

        # Passive Components
        instance commsBufferManager
        instance frameAccumulator
        instance fprimeRouter
        instance tcDeframer
        instance spacePacketDeframer
        instance framer
        instance spacePacketFramer
        instance apidManager
        instance aggregator

        connections Downlink {
            # ComQueue <-> SpacePacketFramer
            comQueue.dataOut                -> spacePacketFramer.dataIn
            spacePacketFramer.dataReturnOut -> comQueue.dataReturnIn
            # SpacePacketFramer buffer and APID management
            spacePacketFramer.bufferAllocate   -> commsBufferManager.bufferGetCallee
            spacePacketFramer.bufferDeallocate -> commsBufferManager.bufferSendIn
            spacePacketFramer.getApidSeqCount  -> apidManager.getApidSeqCountIn
            # SpacePacketFramer <-> TmFramer
            spacePacketFramer.dataOut -> aggregator.dataIn
            aggregator.dataOut        -> framer.dataIn

            framer.dataReturnOut      -> aggregator.dataReturnIn
            aggregator.dataReturnOut  -> spacePacketFramer.dataReturnIn

            # ComStatus
            framer.comStatusOut            -> aggregator.comStatusIn
            aggregator.comStatusOut        -> spacePacketFramer.comStatusIn
            spacePacketFramer.comStatusOut -> comQueue.comStatusIn
            # (Outgoing) Framer <-> ComInterface connections shall be established by the user
        }

        connections Uplink {
            # (Incoming) ComInterface <-> FrameAccumulator connections shall be established by the user
            # FrameAccumulator buffer allocations
            frameAccumulator.bufferDeallocate -> commsBufferManager.bufferSendIn
            frameAccumulator.bufferAllocate   -> commsBufferManager.bufferGetCallee
            # FrameAccumulator <-> TcDeframer
            frameAccumulator.dataOut -> tcDeframer.dataIn
            tcDeframer.dataReturnOut -> frameAccumulator.dataReturnIn
            # TcDeframer <-> SpacePacketDeframer
            tcDeframer.dataOut                -> spacePacketDeframer.dataIn
            spacePacketDeframer.dataReturnOut -> tcDeframer.dataReturnIn
            # SpacePacketDeframer APID validation
            spacePacketDeframer.validateApidSeqCount -> apidManager.validateApidSeqCountIn
            # SpacePacketDeframer <-> Router
            spacePacketDeframer.dataOut -> fprimeRouter.dataIn
            fprimeRouter.dataReturnOut  -> spacePacketDeframer.dataReturnIn
            # Router buffer allocations
            fprimeRouter.bufferAllocate   -> commsBufferManager.bufferGetCallee
            fprimeRouter.bufferDeallocate -> commsBufferManager.bufferSendIn
        }
    } # end FramingSubtopology

    # This subtopology uses FramingSubtopology with a ComStub component for Com Interface
    topology Subtopology {
        import FramingSubtopology

        instance comStub

        connections ComStub {
            # Framer <-> ComStub (Downlink)
            ComCcsdsRoarFM.framer.dataOut -> comStub.dataIn
            comStub.dataReturnOut         -> ComCcsdsRoarFM.framer.dataReturnIn
            comStub.comStatusOut          -> ComCcsdsRoarFM.framer.comStatusIn

            # ComStub <-> FrameAccumulator (Uplink)
            comStub.dataOut                             -> ComCcsdsRoarFM.frameAccumulator.dataIn
            ComCcsdsRoarFM.frameAccumulator.dataReturnOut -> comStub.dataReturnIn
        }
    } # end Subtopology

} # end ComCcsdsRoarFM
