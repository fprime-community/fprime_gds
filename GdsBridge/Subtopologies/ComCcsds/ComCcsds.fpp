module FprimeGds {
    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance frameAccumulator: Svc.FrameAccumulator base id 0xFF001000 \ 
    {

        phase Fpp.ToCpp.Phases.configObjects """
        Svc::FrameDetectors::CcsdsTcFrameDetector frameDetector;
        """
        phase Fpp.ToCpp.Phases.configComponents """
        FprimeGds::frameAccumulator.configure(
            ConfigObjects::FprimeGds_frameAccumulator::frameDetector,
            1,
            FprimeGds::Allocation::memAllocator,
            FprimeGdsConfig::BuffMgr::frameAccumulatorSize
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        FprimeGds::frameAccumulator.cleanup();
        """
    }

    instance commsBufferManager: Svc.BufferManager base id 0xFF002000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::BufferManager::BufferBins bins;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        memset(&ConfigObjects::FprimeGds_commsBufferManager::bins, 0, sizeof(ConfigObjects::FprimeGds_commsBufferManager::bins));
        ConfigObjects::FprimeGds_commsBufferManager::bins.bins[0].bufferSize = FprimeGdsConfig::BuffMgr::commsBuffSize;
        ConfigObjects::FprimeGds_commsBufferManager::bins.bins[0].numBuffers = FprimeGdsConfig::BuffMgr::commsBuffCount;
        ConfigObjects::FprimeGds_commsBufferManager::bins.bins[1].bufferSize = FprimeGdsConfig::BuffMgr::commsFileBuffSize;
        ConfigObjects::FprimeGds_commsBufferManager::bins.bins[1].numBuffers = FprimeGdsConfig::BuffMgr::commsFileBuffCount;
        FprimeGds::commsBufferManager.setup(
            FprimeGdsConfig::BuffMgr::commsBuffMgrId,
            0,
            FprimeGds::Allocation::memAllocator,
            ConfigObjects::FprimeGds_commsBufferManager::bins
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        FprimeGds::commsBufferManager.cleanup();
        """
    }

    instance tcDeframer: Svc.Ccsds.TcDeframer base id 0xFF004000


    # NOTE: name 'framer' is used for the framer that connects to the Com Adapter Interface for better subtopology interoperability
    instance framer: Svc.Ccsds.TmFramer base id 0xFF007000

    instance comStub: Svc.ComStub base id 0xFF00A000

    topology FramingSubtopology {
        # Usage Note:
        #
        # When importing this subtopology, users shall establish 5 port connections with a component implementing
        # the Svc.Com (Svc/Interfaces/Com.fpp) interface. They are as follows:
        #
        # 1) Outputs:
        #     - ComCcsds.framer.dataOut                 -> [Svc.Com].dataIn
        #     - ComCcsds.frameAccumulator.dataReturnOut -> [Svc.Com].dataReturnIn
        # 2) Inputs:
        #     - [Svc.Com].dataReturnOut -> ComCcsds.framer.dataReturnIn
        #     - [Svc.Com].comStatusOut  -> ComCcsds.framer.comStatusIn
        #     - [Svc.Com].dataOut       -> ComCcsds.frameAccumulator.dataIn



        # Passive Components
        instance commsBufferManager
        instance frameAccumulator
        instance tcDeframer
        instance framer

        connections Downlink {
            # SpacePacketFramer <-> TmFramer
            # ATTACH ME -> framer.dataIn
            # framer.dataReturnOut -> ATTACH ME

            # ComStatus
            #framer.comStatusOut            -> ATTACHME
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
            # tcDeframer.dataOut                -> ATTACH ME
            # ATTACH ME -> tcDeframer.dataReturnIn
        }
    } # end FramingSubtopology

    # This subtopology uses FramingSubtopology with a ComStub component for Com Interface
    topology ComCccsdsSubtopology {
        import FramingSubtopology

        instance comStub

        connections ComStub {
            # Framer <-> ComStub (Downlink)
            FprimeGds.framer.dataOut -> comStub.dataIn
            comStub.dataReturnOut   -> FprimeGds.framer.dataReturnIn
            comStub.comStatusOut    -> FprimeGds.framer.comStatusIn

            # ComStub <-> FrameAccumulator (Uplink)
            comStub.dataOut -> FprimeGds.frameAccumulator.dataIn
            FprimeGds.frameAccumulator.dataReturnOut -> comStub.dataReturnIn
        }
    } # end Subtopology

} # end ComCcsds
