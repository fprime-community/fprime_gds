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


  topology GdsBridge {

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance comDriver

  # ----------------------------------------------------------------------
  # Pattern graph specifiers
  # ----------------------------------------------------------------------
    time connections instance chronoTime


  # ----------------------------------------------------------------------
  # Direct graph specifiers
  # ----------------------------------------------------------------------

    connections Communications {
      # ComDriver buffer allocations
      comDriver.allocate      -> ComCcsds.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> ComCcsds.commsBufferManager.bufferSendIn
      
      # ComDriver <-> ComStub (Uplink)
      # TODO: connection **CONNECTION**
#      comDriver.$recv                     -> 
#                                          -> comDriver.recvReturnIn
      
      # ComStub <-> ComDriver (Downlink)
#                               -> comDriver.$send
#      comDriver.ready          -> ComCcsds.comStub.drvConnected
    }



  }

}
