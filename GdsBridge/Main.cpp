// ======================================================================
// \title  Main.cpp
// \brief cFS application entry point hosting the F Prime GDS bridge topology
//
// ======================================================================
// Used to access topology functions
#include <GdsBridge/Top/GdsBridgeTopology.hpp>
#include <GdsBridge/Top/GdsBridgeTopologyAc.hpp>
// OSAL initialization
#include <Os/Os.hpp>
#include "Fw/Logger/Logger.hpp"

extern "C" {
    #include "cfe.h"
    #include "cfe_config.h"
    #include "fprime_gds_version.h"
    #include "fprime_gds_internal_cfg.h"

    /**
     * \brief GDS bridge application entry point
     * 
     * This is the main entry point for the F Prime GDS bridge application. It initializes the OS, sets up the
     * topology, and enters the main loop to poll the communications driver.
     * 
     * WARNING: this function **must** have C linkage to be called from within cFE.
     */
    void FPRIME_GDS_Main(void);
}

/**
 * \brief Initializes the F Prime GDS bridge application
 * 
 * This initializes the F Prime GDS bridge application including a setup of the topology.
 */
CFE_Status_t FPRIME_GDS_Init(FprimeGds::TopologyState& inputs);

// Event IDs for events emitted directly by the application entry point
enum FPrimeGdsEventIds {
    FPRIME_GDS_INIT_INF_EID = 1,      //!< Application initialized
    FPRIME_GDS_PIPE_ERR_EID = 2,      //!< Software bus pipe creation failed
    FPRIME_GDS_SUBSCRIBE_ERR_EID = 3  //!< Software bus subscription failed
};

// Topology state stored at file scope so the delete callback can always reach it.
static FprimeGds::TopologyState g_topologyState;

static void FPRIME_GDS_StopAndTeardown(FprimeGds::TopologyState& inputs)
{
    FprimeGds::teardownTopology(inputs);
}

static void FPRIME_GDS_Shutdown(FprimeGds::TopologyState& inputs, uint32 status)
{
    FPRIME_GDS_StopAndTeardown(inputs);
    CFE_ES_ExitApp(status);
}

void FPRIME_GDS_delete_callback(void)
{
    FPRIME_GDS_StopAndTeardown(g_topologyState);
}

// Main entry point (see above)
void FPRIME_GDS_Main(void) {
    Os::init();
    FprimeGds::TopologyState& inputs = g_topologyState;
    inputs.hostname = "0.0.0.0";
    inputs.port = 15010;

    uint32 run_status = CFE_ES_RunStatus_APP_RUN;
    
    // Initialization failure is fatal: the topology is not started and the app exits with error status
    CFE_Status_t status = FPRIME_GDS_Init(inputs);
    if (status != CFE_SUCCESS) {
        CFE_ES_ExitApp(CFE_ES_RunStatus_APP_ERROR);
        return;
    }
    // Main loop to run the GDS bridge application
    while (CFE_ES_RunLoop(&run_status) == true) {
        // Uplink processing
        Fw::QueuedComponentBase::MsgDispatchStatus status = FprimeGds::cfsBridge.process();
        if (status == Fw::QueuedComponentBase::MSG_DISPATCH_EXIT) {
            run_status = CFE_ES_RunStatus_APP_EXIT;
        } else if (status == Fw::QueuedComponentBase::MSG_DISPATCH_EMPTY) {
            // No messages to process, do nothing
        } else if (status != Fw::QueuedComponentBase::MSG_DISPATCH_OK) {
            Fw::Logger::log("[WARNING] Failed to process a message in the queue: %d\n", status);
        }
        // Sleep to avoid a busy loop when idle; a failed delay only costs idle sleep
        (void)Os::Task::delay(Fw::TimeInterval(0, 100));
    }
    FPRIME_GDS_Shutdown(inputs, run_status);
}

// Initialize the GDS bridge
CFE_Status_t FPRIME_GDS_Init(FprimeGds::TopologyState& inputs) {
    CFE_Status_t status;
    char         VersionString[FPRIME_GDS_CFG_MAX_VERSION_STR_LEN];

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("F Prime GDS App: Error Registering Events, RC = 0x%08lX\n",
                             static_cast<unsigned long>(status));
    }  else
    {
        // Wrap uplinked F Prime command packets as valid cFS command packets (secondary header + checksum)
        status = FprimeGds::cfsBridge.configure(FPRIME_GDS_PLATFORM_PIPE_DEPTH, FPRIME_GDS_PLATFORM_PIPE_NAME, true, true);
        if (status != CFE_SUCCESS)
        {
            (void)CFE_EVS_SendEvent(FPRIME_GDS_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "F Prime GDS App: Error creating SB Command Pipe, RC = 0x%08lX",
                                    static_cast<unsigned long>(status));
        }
    }
    if (status == CFE_SUCCESS)
    {
        static constexpr ComCfg::Apid::T downlinkApids[] = {
            ComCfg::Apid::FW_PACKET_DP,
            ComCfg::Apid::FW_PACKET_TELEM,
            ComCfg::Apid::FW_PACKET_LOG,
            ComCfg::Apid::FW_PACKET_PACKETIZED_TLM,
            ComCfg::Apid::FW_PACKET_FILE,
        };
        FwSizeType failedIndex = 0;
        for (FwSizeType i = 0; (i < FW_NUM_ARRAY_ELEMENTS(downlinkApids)) && (status == CFE_SUCCESS); i++)
        {
            failedIndex = i;
            // Downlink packets are published as cFS telemetry messages (secondary header flag set)
            status = FprimeGds::cfsBridge.subscribeCfs(downlinkApids[i],
                                                       FPrimeCfs::CfsBridge::CfsMessageType::TELEMETRY);
        }
        if (status != CFE_SUCCESS)
        {
            (void)CFE_EVS_SendEvent(FPRIME_GDS_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "F Prime GDS App: Error subscribing to APID 0x%04X, RC = 0x%08lX",
                                    static_cast<unsigned int>(downlinkApids[failedIndex]),
                                    static_cast<unsigned long>(status));
        }
    }

    // Only bring up the topology (and its teardown handler) when initialization succeeded
    if (status == CFE_SUCCESS)
    {
        OS_TaskInstallDeleteHandler(&FPRIME_GDS_delete_callback);
        FprimeGds::setupTopology(inputs);

        CFE_Config_GetVersionString(VersionString, FPRIME_GDS_CFG_MAX_VERSION_STR_LEN, "F Prime GDS App", FPRIME_GDS_VERSION,
                                    FPRIME_GDS_BUILD_CODENAME, FPRIME_GDS_LAST_OFFICIAL);
        (void)CFE_EVS_SendEvent(FPRIME_GDS_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                                "F Prime GDS App Initialized.%s", VersionString);
    }

    return status;
}
