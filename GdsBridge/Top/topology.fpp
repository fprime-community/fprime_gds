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
  instance cfsBridge: FPrimeCfs.CfsBridge base id 0x10012000

  topology GdsBridge {
    import FprimeGds.ComCccsdsSubtopology

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
      FprimeGds.tcDeframer.dataOut -> cfsBridge.dataIn
      cfsBridge.dataReturnOut -> FprimeGds.tcDeframer.dataReturnIn
    }

    connections Communications {
      # ComDriver buffer allocations
      comDriver.allocate      -> FprimeGds.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> FprimeGds.commsBufferManager.bufferSendIn
      
      # ComDriver <-> ComStub (Uplink)
      # TODO: connection **CONNECTION**
      comDriver.$recv                    ->  FprimeGds.comStub.drvReceiveIn
      FprimeGds.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn
      
      # ComStub <-> ComDriver (Downlink)
#                               -> comDriver.$send
#      comDriver.ready          -> FprimeGds.comStub.drvConnected
    }



  }

}
