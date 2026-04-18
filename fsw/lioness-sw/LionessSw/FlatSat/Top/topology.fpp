module FlatSat {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

  enum Ports_RateGroups {
    rateGroup1
    rateGroup2
    rateGroup3
  }

  topology FlatSat {

  # ----------------------------------------------------------------------
  # Subtopology imports
  # ----------------------------------------------------------------------
    import CdhCore.Subtopology
    import ComCcsds.Subtopology
    import ComCcsdsRoarFM.Subtopology
    import DataProducts.Subtopology
    import FileHandling.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance rateGroup1
    instance rateGroup2
    instance rateGroup3
    instance rateGroupDriver
    instance systemResources
    instance timer
    instance comDriver
    instance roarfmDriver
    instance comSplitterEvents
    instance comSplitterTelemetry
    instance cmdSeq
    instance watchdog
    instance watchdogGpio
    instance modeManager

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------

    command connections instance CdhCore.cmdDisp
    event connections instance CdhCore.events
    telemetry connections instance CdhCore.tlmSend
    text event connections instance CdhCore.textLogger
    health connections instance CdhCore.$health
    param connections instance FileHandling.prmDb
    time connections instance chronoTime

  # ----------------------------------------------------------------------
  # Telemetry packets (only used when TlmPacketizer is used)
  # ----------------------------------------------------------------------

    # include "FlatSatPackets.fppi"

  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections ComCcsds_CdhCore {
      # Core events and telemetry fan-out to both com stacks via ComSplitters
      CdhCore.events.PktSend  -> comSplitterEvents.comIn
      comSplitterEvents.comOut -> ComCcsds.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.EVENTS]
      comSplitterEvents.comOut -> ComCcsdsRoarFM.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.EVENTS]

      CdhCore.tlmSend.PktSend -> comSplitterTelemetry.comIn
      comSplitterTelemetry.comOut -> ComCcsds.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.TELEMETRY]
      comSplitterTelemetry.comOut -> ComCcsdsRoarFM.comQueue.comPacketQueueIn[ComCcsds.Ports_ComPacketQueue.TELEMETRY]

      # Uplinked commands from either stack's router are routed to the Command Dispatcher.
      # Both connections share the cmdDisp's seqCmdBuff/seqCmdStatus pair (FPP auto-assigns
      # distinct port indices per connection).
      ComCcsds.fprimeRouter.commandOut       -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus           -> ComCcsds.fprimeRouter.cmdResponseIn

      ComCcsdsRoarFM.fprimeRouter.commandOut -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus           -> ComCcsdsRoarFM.fprimeRouter.cmdResponseIn
    }

    connections ComCcsds_FileHandling {
      # File downlink/uplink currently only run over the primary (ComCcsds) stack.
      # The RoarFM stack mirrors events/telemetry for situational awareness during
      # bring-up but is not yet used for file transfer.
      FileHandling.fileDownlink.bufferSendOut -> ComCcsds.comQueue.bufferQueueIn[ComCcsds.Ports_ComBufferQueue.FILE]
      ComCcsds.comQueue.bufferReturnOut[ComCcsds.Ports_ComBufferQueue.FILE] -> FileHandling.fileDownlink.bufferReturn

      # Router to File Uplink
      ComCcsds.fprimeRouter.fileOut -> FileHandling.fileUplink.bufferSendIn
      FileHandling.fileUplink.bufferSendOut -> ComCcsds.fprimeRouter.fileBufferReturnIn
    }

    connections Communications {
      # Primary com stack: ComDriver (UART) <-> ComCcsds.comStub
      comDriver.allocate      -> ComCcsds.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> ComCcsds.commsBufferManager.bufferSendIn

      # Uplink
      comDriver.$recv                      -> ComCcsds.comStub.drvReceiveIn
      ComCcsds.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn

      # Downlink
      ComCcsds.comStub.drvSendOut -> comDriver.$send
      comDriver.ready             -> ComCcsds.comStub.drvConnected
    }

    connections CommunicationsRoarFM {
      # Secondary com stack: RoarFM UART driver <-> ComCcsdsRoarFM.comStub
      # (UART stands in for the future Roar FM radio during bring-up)
      roarfmDriver.allocate   -> ComCcsdsRoarFM.commsBufferManager.bufferGetCallee
      roarfmDriver.deallocate -> ComCcsdsRoarFM.commsBufferManager.bufferSendIn

      # Uplink
      roarfmDriver.$recv                         -> ComCcsdsRoarFM.comStub.drvReceiveIn
      ComCcsdsRoarFM.comStub.drvReceiveReturnOut -> roarfmDriver.recvReturnIn

      # Downlink
      ComCcsdsRoarFM.comStub.drvSendOut -> roarfmDriver.$send
      roarfmDriver.ready                -> ComCcsdsRoarFM.comStub.drvConnected
    }

    connections FileHandling_DataProducts {
      # Data Products to File Downlink
      DataProducts.dpCat.fileOut -> FileHandling.fileDownlink.SendFile
      FileHandling.fileDownlink.FileComplete -> DataProducts.dpCat.fileDone
    }

    connections RateGroups {
      # timer to drive rate group
      timer.CycleOut -> rateGroupDriver.CycleIn

      # Rate group 1
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1.CycleIn
      rateGroup1.RateGroupMemberOut[0] -> CdhCore.tlmSend.Run
      rateGroup1.RateGroupMemberOut[1] -> FileHandling.fileDownlink.Run
      rateGroup1.RateGroupMemberOut[2] -> systemResources.run
      rateGroup1.RateGroupMemberOut[3] -> ComCcsds.comQueue.run
      rateGroup1.RateGroupMemberOut[4] -> ComCcsds.aggregator.timeout
      rateGroup1.RateGroupMemberOut[5] -> watchdog.run
      rateGroup1.RateGroupMemberOut[6] -> modeManager.schedIn
      rateGroup1.RateGroupMemberOut[7] -> ComCcsdsRoarFM.comQueue.run
      rateGroup1.RateGroupMemberOut[8] -> ComCcsdsRoarFM.aggregator.timeout

      # Rate group 2
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup2] -> rateGroup2.CycleIn
      rateGroup2.RateGroupMemberOut[0] -> cmdSeq.schedIn

      # Rate group 3
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup3] -> rateGroup3.CycleIn
      rateGroup3.RateGroupMemberOut[0] -> CdhCore.$health.Run
      rateGroup3.RateGroupMemberOut[1] -> ComCcsds.commsBufferManager.schedIn
      rateGroup3.RateGroupMemberOut[2] -> DataProducts.dpBufferManager.schedIn
      rateGroup3.RateGroupMemberOut[3] -> DataProducts.dpWriter.schedIn
      rateGroup3.RateGroupMemberOut[4] -> DataProducts.dpMgr.schedIn
      rateGroup3.RateGroupMemberOut[5] -> ComCcsdsRoarFM.commsBufferManager.schedIn
    }

    connections CdhCore_cmdSeq {
      # Command Sequencer
      cmdSeq.comCmdOut -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus -> cmdSeq.cmdResponseIn
    }

    connections FlatSat {
      watchdog.gpioSet -> watchdogGpio.gpioWrite
    }

  }

}
