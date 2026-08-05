module FprimeGds {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------
  module Default {
    constant QUEUE_SIZE = 10
    constant STACK_SIZE = 64 * 1024
  }

  @ Time source using std::chrono
  instance chronoTime: Svc.ChronoTime base id 0x10010000

  @ TCP server for communication with GDS
  instance comDriver: Drv.TcpServer base id 0x10011000

  @ Instance to bridge F Prime communication to the CFS bus
  instance cfsBridge: FPrimeCfs.CfsBridge base id 0x10012000 \
    queue size Default.QUEUE_SIZE

  @ Instance to strip cFS telemetry secondary headers from downlinked space packets
  instance tlmStripper: FPrimeCfs.CfsTlmStripper base id 0x10013000

  topology GdsBridge {
    import ComCcsdsNoRouter.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance comDriver
    instance cfsBridge
    instance tlmStripper

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------
    time connections instance chronoTime


  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections CfsBridge {
      # Uplink: TC frames carry complete space packets; F Prime command packets are wrapped as
      # valid cFS command packets (secondary header + checksum) before transmission on the SB
      ComCcsdsNoRouter.tcDeframer.dataOut -> cfsBridge.dataIn
      cfsBridge.dataReturnOut -> ComCcsdsNoRouter.tcDeframer.dataReturnIn

      # Downlink: complete space packets from the cFS software bus have their cFS
      # telemetry secondary headers stripped, then are wrapped in TM frames
      cfsBridge.dataOut -> tlmStripper.dataIn
      tlmStripper.dataReturnOut -> cfsBridge.dataReturnIn
      tlmStripper.dataOut -> ComCcsdsNoRouter.framer.dataIn
      ComCcsdsNoRouter.framer.dataReturnOut -> tlmStripper.dataReturnIn
      tlmStripper.bufferAllocate -> ComCcsdsNoRouter.commsBufferManager.bufferGetCallee
      tlmStripper.bufferDeallocate -> ComCcsdsNoRouter.commsBufferManager.bufferSendIn
      ComCcsdsNoRouter.framer.comStatusOut -> cfsBridge.comStatusIn
    }

    connections Communications {
      # ComDriver buffer allocations
      comDriver.allocate      -> ComCcsdsNoRouter.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> ComCcsdsNoRouter.commsBufferManager.bufferSendIn
      comDriver.ready         -> ComCcsdsNoRouter.comStub.drvConnected
      
      # ComDriver <-> ComStub (Uplink)
      # TODO: connection **CONNECTION**
      comDriver.$recv                    ->  ComCcsdsNoRouter.comStub.drvReceiveIn
      ComCcsdsNoRouter.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn

      ComCcsdsNoRouter.comStub.drvSendOut -> comDriver.$send
      
      # ComStub <-> ComDriver (Downlink)
#                               -> comDriver.$send
#      comDriver.ready          -> FprimeGds.comStub.drvConnected
    }



  }

}
