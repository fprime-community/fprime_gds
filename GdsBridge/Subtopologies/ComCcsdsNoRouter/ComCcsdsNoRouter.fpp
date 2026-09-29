module ComCcsdsNoRouter {

    # ComPacket Queue enum for queue types
    enum Ports_ComPacketQueue : U8 {
        EVENTS,
        TELEMETRY 
    }

    enum Ports_ComBufferQueue : U8 {
        FILE
    }

    # ----------------------------------------------------------------------
    # Active Components
    # ----------------------------------------------------------------------
    instance aggregator: Svc.ComAggregator base id ComCcsdsNoRouterConfig.BASE_ID + 0x06000 \
        queue size ComCcsdsNoRouterConfig.QueueSizes.aggregator \
        stack size ComCcsdsNoRouterConfig.StackSizes.aggregator \
        priority ComCcsdsNoRouterConfig.Priorities.aggregator \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        // Non-spanning: each space packet stays whole inside one idle-filled TM data field
        ComCcsdsNoRouter::aggregator.configure(
            static_cast<FwSizeType>(Svc::Ccsds::TmDataFieldSize),
            false,
            2,
            ComCcsdsNoRouter::Allocation::memAllocator
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCcsdsNoRouter::aggregator.cleanup();
        """
    }

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance frameAccumulator: Svc.FrameAccumulator base id ComCcsdsNoRouterConfig.BASE_ID + 0x01000 \ 
    {

        phase Fpp.ToCpp.Phases.configObjects """
        Svc::FrameDetectors::CcsdsTcFrameDetector frameDetector;
        """
        phase Fpp.ToCpp.Phases.configComponents """
        ComCcsdsNoRouter::frameAccumulator.configure(
            ConfigObjects::ComCcsdsNoRouter_frameAccumulator::frameDetector,
            1,
            ComCcsdsNoRouter::Allocation::memAllocator,
            ComCcsdsNoRouterConfig::BuffMgr::frameAccumulatorSize
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCcsdsNoRouter::frameAccumulator.cleanup();
        """
    }

    instance commsBufferManager: Svc.BufferManager base id ComCcsdsNoRouterConfig.BASE_ID + 0x02000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::BufferManager::BufferBins bins;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        memset(&ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins, 0, sizeof(ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins));
        ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins.bins[0].bufferSize = ComCcsdsNoRouterConfig::BuffMgr::commsBuffSize;
        ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins.bins[0].numBuffers = ComCcsdsNoRouterConfig::BuffMgr::commsBuffCount;
        ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins.bins[1].bufferSize = ComCcsdsNoRouterConfig::BuffMgr::commsFileBuffSize;
        ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins.bins[1].numBuffers = ComCcsdsNoRouterConfig::BuffMgr::commsFileBuffCount;
        ComCcsdsNoRouter::commsBufferManager.setup(
            ComCcsdsNoRouterConfig::BuffMgr::commsBuffMgrId,
            0,
            ComCcsdsNoRouter::Allocation::memAllocator,
            ConfigObjects::ComCcsdsNoRouter_commsBufferManager::bins
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ComCcsdsNoRouter::commsBufferManager.cleanup();
        """
    }

    instance tcDeframer: Svc.Ccsds.TcDeframer base id ComCcsdsNoRouterConfig.BASE_ID + 0x04000

    # NOTE: name 'framer' is used for the framer that connects to the Com Adapter Interface for better subtopology interoperability
    instance framer: Svc.Ccsds.TmFramer base id ComCcsdsNoRouterConfig.BASE_ID + 0x07000

    instance comStub: Svc.ComStub base id ComCcsdsNoRouterConfig.BASE_ID + 0x0A000

    topology FramingSubtopology {
        # Usage Note:
        #
        # When importing this subtopology, users shall establish 5 port connections with a component implementing
        # the Svc.Com (Svc/Interfaces/Com.fpp) interface. They are as follows:
        #
        # 1) Outputs:
        #     - ComCcsdsNoRouter.framer.dataOut                 -> [Svc.Com].dataIn
        #     - ComCcsdsNoRouter.frameAccumulator.dataReturnOut -> [Svc.Com].dataReturnIn
        # 2) Inputs:
        #     - [Svc.Com].dataReturnOut -> ComCcsdsNoRouter.framer.dataReturnIn
        #     - [Svc.Com].comStatusOut  -> ComCcsdsNoRouter.framer.comStatusIn
        #     - [Svc.Com].dataOut       -> ComCcsdsNoRouter.frameAccumulator.dataIn
        #
        # The source of complete space packets connects to the aggregator, and a rate group must drive
        # ComCcsdsNoRouter.aggregator.timeout so that partially filled frames are flushed:
        #     - [source].dataOut                          -> ComCcsdsNoRouter.aggregator.dataIn
        #     - ComCcsdsNoRouter.aggregator.dataReturnOut -> [source].dataReturnIn
        #     - ComCcsdsNoRouter.aggregator.comStatusOut  -> [source].comStatusIn
        #     - [rate group].RateGroupMemberOut[i]        -> ComCcsdsNoRouter.aggregator.timeout

        # Active Components
        instance aggregator

        # Passive Components
        instance commsBufferManager
        instance frameAccumulator
        instance tcDeframer
        instance framer

        connections Downlink {
            # Space packets are packed whole into fixed-size, idle-filled TM data fields by the aggregator,
            # then wrapped in TM frames. The aggregator input connections shall be established by the user.
            aggregator.dataOut     -> framer.dataIn
            framer.dataReturnOut   -> aggregator.dataReturnIn
            framer.comStatusOut    -> aggregator.comStatusIn
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
            # TcDeframer output carries complete space packets; the downstream user connects it to the
            # consumer of complete space packets (e.g. a CfsBridge)
        }
    } # end FramingSubtopology

    # This subtopology uses FramingSubtopology with a ComStub component for Com Interface
    topology Subtopology {
        import FramingSubtopology

        instance comStub

        connections ComStub {
            # Framer <-> ComStub (Downlink)
            ComCcsdsNoRouter.framer.dataOut -> comStub.dataIn
            comStub.dataReturnOut   -> ComCcsdsNoRouter.framer.dataReturnIn
            comStub.comStatusOut    -> ComCcsdsNoRouter.framer.comStatusIn

            # ComStub <-> FrameAccumulator (Uplink)
            comStub.dataOut -> ComCcsdsNoRouter.frameAccumulator.dataIn
            ComCcsdsNoRouter.frameAccumulator.dataReturnOut -> comStub.dataReturnIn
        }
    } # end Subtopology

} # end ComCcsdsNoRouter
