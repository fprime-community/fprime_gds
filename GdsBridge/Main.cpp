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
// Used for signal handling shutdown
#include <signal.h>
// Used for command line argument processing
#include <getopt.h>
// Used for printf functions
#include <cstdlib>

FprimeGds::TopologyState inputs;

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

#include "Fw/Logger/Logger.hpp"

#include <algorithm>    
#include <cstring>
extern "C" {
    #include "cfe.h"
    #include "cfe_config.h"
    #include "fprime_gds_msgstruct.h"
    #include "fprime_gds_msgids.h"
    #include "cfe_sb.h"   // for CFE_SB_TransmitMsg
}



CFE_SB_PipeId_t CommandPipe;

/**
 * \brief Initializes the F Prime GDS bridge application
 * 
 * This initializes the F Prime GDS bridge application including a setup of the topology.
 */
CFE_Status_t FPRIME_GDS_Init();

void send() {
     FPRIME_GDS_PassThroughCmd_t message;
    
    CFE_MSG_Size_t message_size = sizeof(CFE_MSG_CommandHeader_t) + 1;
    Fw::Logger::log("[DEBUG] Calculated message size: %zu\n", message_size);
    CFE_Status_t status = CFE_MSG_Init(reinterpret_cast<CFE_MSG_Message_t*>(&message),  CFE_SB_ValueToMsgId(FPRIME_GDS_CMD_MID), message_size);
    if (status != CFE_SUCCESS)
    {
        Fw::Logger::log("[ERROR] Failed to initialize CFS message: %x\n", status);
    } else {
        Fw::Logger::log("[INFO] Successfully initialized CFS message\n");
    }
    std::memcpy(message.data, reinterpret_cast<const U8*>("!"), 1);

    CFE_MSG_Message_t *msg = reinterpret_cast<CFE_MSG_Message_t *>(&message);
    CFE_MSG_Size_t decoded_size = 0;
    CFE_MSG_GetSize(msg, &decoded_size);
    CFE_SB_MsgId_t msgid;
    CFE_MSG_GetMsgId(msg, &msgid);

    printf("msgid=0x%04x size=%u actual=%u\n",
       (unsigned)CFE_SB_MsgIdToValue(msgid),
       (unsigned)decoded_size,
       1);


    status = CFE_SB_TransmitMsg(reinterpret_cast<CFE_MSG_Message_t*>(&message), false);

    //CFE_MSG_Message_t *msg = reinterpret_cast<CFE_MSG_Message_t *>(&message);
    Fw::Logger::log("[DEBUG] Message: ");
    for (FwSizeType i = 0; i < message_size; i++) {
        Fw::Logger::log("%02x ", reinterpret_cast<uint8_t *>(&message)[i]);
    }
    Fw::Logger::log("\n");

    Fw::Logger::log("[DEBUG] (Length): ");
    for (FwSizeType i = 0; i < 2; i++) {
        Fw::Logger::log("%02x",reinterpret_cast<uint8_t *>(&msg->CCSDS.Pri.Length)[i]);
        if ((i % 2) == 1) {
            Fw::Logger::log(" ");
        }
    }
    Fw::Logger::log("\n");

    Fw::Logger::log("[INFO] CCSDS Primary Header Length: %u\n", (msg->CCSDS.Pri.Length[0] << 8) | msg->CCSDS.Pri.Length[1]);
    if (status != CFE_SUCCESS)
    {
        Fw::Logger::log("[ERROR] Failed to send CFS message: %x\n", status);
    } else {
        Fw::Logger::log("[INFO] Successfully sent CFS message\n");
    }
}

// Main entry point (see above)
void FPRIME_GDS_Main(void) {
    Os::init();
    inputs.hostname = "0.0.0.0";
    inputs.port = 15010;

    uint32 run_status = CFE_ES_RunStatus_APP_RUN;
    
    // Initialize the CFS application and the F Prime setup within 
    CFE_Status_t status = FPRIME_GDS_Init();
    if (status != CFE_SUCCESS) {
        FprimeGds::teardownTopology(inputs);
        CFE_ES_ExitApp(status);
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
    }
    // Shudown, shutdown, everybody shutdown!
    FprimeGds::teardownTopology(inputs);
    CFE_ES_ExitApp(run_status);
}

// Initialize the GDS bridge
CFE_Status_t FPRIME_GDS_Init() {
    printf("Initializing FPrime GDS App...\n");
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
            printf("F Prime GDS App: Error creating SB Command Pipe, RC = 0x%08lX\n", (unsigned long)status);
            CFE_EVS_SendEvent(2, CFE_EVS_EventType_ERROR,
                              "F Prime GDS App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }
    if (status == CFE_SUCCESS)
    {
        printf("Subscribing to cFS messages...\n");
        FprimeGds::cfsBridge.subscribe(ComCfg::Apid::FW_PACKET_DP);
        FprimeGds::cfsBridge.subscribe(ComCfg::Apid::FW_PACKET_TELEM);
        FprimeGds::cfsBridge.subscribe(ComCfg::Apid::FW_PACKET_LOG);
        FprimeGds::cfsBridge.subscribe(ComCfg::Apid::FW_PACKET_PACKETIZED_TLM);
        FprimeGds::cfsBridge.subscribe(ComCfg::Apid::FW_PACKET_FILE);

    }
    // Set up the topology that runs the GDS bridge application
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
