// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for CLI-based systems (Linux, macOS)
//
// ======================================================================
// Used to access topology functions
#include <FPrimeDeployment/Top/FPrimeDeploymentTopology.hpp>
// OSAL initialization
#include <Os/Os.hpp>
// Used for signal handling shutdown
#include <signal.h>
// Used for command line argument processing
#include <getopt.h>
// Used for printf functions
#include <cstdlib>

FPrimeApp::TopologyState inputs;

extern "C" {
    #include "cfe.h"
    #include "cfe_config.h"
    #include "fprime_app_version.h"
    #include "fprime_app_internal_cfg.h"

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

CFE_SB_PipeId_t CommandPipe;

/**
 * \brief Initializes the F Prime GDS bridge application
 * 
 * This initializes the F Prime GDS bridge application including a setup of the topology.
 */
CFE_Status_t FPRIME_GDS_Init();

// Main entry point (see above)
void FPRIME_GDS_Main(void) {
    Os::init();
    inputs.hostname = hostname;
    inputs.port = port_number;

    uint32 run_status = CFE_ES_RunStatus_APP_RUN;
    
    // Initialize the CFS application and the F Prime setup within 
    CFE_Status_t status = FPRIME_GDS_Init();

    // Main loop to run the GDS bridge application
    while (CFE_ES_RunLoop(&run_status) == true) {
        // TODO: stroke that com driver
    }
    // Shudown, shutdown, everybody shutdown!
    FPrimeApp::teardownTopology(inputs);
    CFE_ES_ExitApp(run_status);
}

// Initialize the GDS bridge
CFE_Status_t FPRIME_GDS_Init() {
    printf("Initializing FPrime GDS App...\n");
    CFE_Status_t status;
    char         VersionString[FPRIME_APP_CFG_MAX_VERSION_STR_LEN];

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("F Prime GDS App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
    }
    else
    {
        /*
         ** Create Software Bus message pipe.
         */
        status = CFE_SB_CreatePipe(&CommandPipe, FPRIME_APP_PLATFORM_PIPE_DEPTH,
                                   FPRIME_APP_PLATFORM_PIPE_NAME);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(2, CFE_EVS_EventType_ERROR,
                              "F Prime GDS App: Error creating SB Command Pipe, RC = 0x%08lX", (unsigned long)status);
        }
    }
    // Set up the topology that runs the GDS bridge application
    FPrimeApp::setupTopology(inputs);

    // If setup was successful so far, then print the version message
    if (status == CFE_SUCCESS)
    {
        CFE_Config_GetVersionString(VersionString, FPRIME_APP_CFG_MAX_VERSION_STR_LEN, "F Prime GDS App", FPRIME_GDS_VERSION,
                                    FPRIME_APP_BUILD_CODENAME, FPRIME_APP_LAST_OFFICIAL);
        CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "F Prime GDS App Initialized.%s", VersionString);
    }

    return status;
}
