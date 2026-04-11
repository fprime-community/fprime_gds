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

  topology GdsBridge {
    import ComCcsdsNoRouter.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance comDriver
    instance cfsBridge

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------
    time connections instance chronoTime


  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections CfsBridge {
      ComCcsdsNoRouter.spacePacketDeframer.dataOut -> cfsBridge.dataIn
      cfsBridge.dataReturnOut -> ComCcsdsNoRouter.spacePacketDeframer.dataReturnIn

      cfsBridge.dataOut -> ComCcsdsNoRouter.spacePacketFramer.dataIn
      ComCcsdsNoRouter.spacePacketFramer.dataReturnOut -> cfsBridge.dataReturnIn
      ComCcsdsNoRouter.spacePacketFramer.comStatusOut -> cfsBridge.comStatusIn
    
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
