// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for CLI-based systems (Linux, macOS)
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
    
    // Initialize the CFS application and the F Prime setup within 
    CFE_Status_t status = FPRIME_GDS_Init(inputs);
    if (status != CFE_SUCCESS) {
        FPRIME_GDS_Shutdown(inputs, status);
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
        // Sleep to avoid a busy loop when idle
        Os::Task::delay(Fw::TimeInterval(0, 100));
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
        CFE_ES_WriteToSysLog("F Prime GDS App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }  else
    {
        status = FprimeGds::cfsBridge.configure(FPRIME_GDS_PLATFORM_PIPE_DEPTH, FPRIME_GDS_PLATFORM_PIPE_NAME, true);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(2, CFE_EVS_EventType_ERROR,
                              "F Prime GDS App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }
    if (status == CFE_SUCCESS)
    {
        const ComCfg::Apid::T downlinkApids[] = {
            ComCfg::Apid::FW_PACKET_DP,
            ComCfg::Apid::FW_PACKET_TELEM,
            ComCfg::Apid::FW_PACKET_LOG,
            ComCfg::Apid::FW_PACKET_PACKETIZED_TLM,
            ComCfg::Apid::FW_PACKET_FILE,
        };
        for (FwSizeType i = 0; (i < FW_NUM_ARRAY_ELEMENTS(downlinkApids)) && (status == CFE_SUCCESS); i++)
        {
            status = FprimeGds::cfsBridge.subscribe(downlinkApids[i]);
        }
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(2, CFE_EVS_EventType_ERROR,
                              "F Prime GDS App: Error subscribing to cFS messages, RC = 0x%08lX", (unsigned long)status);
        }
    }
    // Set up the topology that runs the GDS bridge application
    OS_TaskInstallDeleteHandler(&FPRIME_GDS_delete_callback);
    FprimeGds::setupTopology(inputs);

    // If setup was successful so far, then print the version message
    if (status == CFE_SUCCESS)
    {
        CFE_Config_GetVersionString(VersionString, FPRIME_GDS_CFG_MAX_VERSION_STR_LEN, "F Prime GDS App", FPRIME_GDS_VERSION,
                                    FPRIME_GDS_BUILD_CODENAME, FPRIME_GDS_LAST_OFFICIAL);
        CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "F Prime GDS App Initialized.%s", VersionString);
    }

    return status;
}
