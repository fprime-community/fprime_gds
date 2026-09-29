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
  instance tlmStripper: FprimeGds.CfsTlmStripper base id 0x10013000

  @ Timer polled from the application main loop, drives the downlink rate group
  instance pollingTimer: Svc.PollingTimer base id 0x10014000

  @ Rate group flushing partially filled downlink frames out of the aggregator
  instance rateGroup: Svc.PassiveRateGroup base id 0x10015000 \
  {
    phase Fpp.ToCpp.Phases.configObjects """
    Svc::PassiveRateGroup::ContextArray context;
    """

    phase Fpp.ToCpp.Phases.configComponents """
    FprimeGds::rateGroup.configure(ConfigObjects::FprimeGds_rateGroup::context);
    """
  }

  deployment topology GdsBridge {
    import ComCcsdsNoRouter.Subtopology

  # ----------------------------------------------------------------------
  # Instances used in the topology
  # ----------------------------------------------------------------------
    instance chronoTime
    instance comDriver
    instance cfsBridge
    instance tlmStripper
    instance pollingTimer
    instance rateGroup

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
      # telemetry secondary headers stripped in place, then are packed whole into
      # idle-filled TM data fields by the aggregator before being wrapped in TM frames
      cfsBridge.dataOut -> tlmStripper.dataIn
      tlmStripper.dataReturnOut -> cfsBridge.dataReturnIn
      tlmStripper.dataOut -> ComCcsdsNoRouter.aggregator.dataIn
      ComCcsdsNoRouter.aggregator.dataReturnOut -> tlmStripper.dataReturnIn
      ComCcsdsNoRouter.aggregator.comStatusOut -> cfsBridge.comStatusIn
    }

    connections RateGroup {
      # The polling timer is cycled from the application main loop; the rate group flushes
      # partially filled downlink frames so telemetry is not held until a frame is full
      pollingTimer.CycleOut -> rateGroup.CycleIn
      rateGroup.RateGroupMemberOut[0] -> ComCcsdsNoRouter.aggregator.timeout
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
