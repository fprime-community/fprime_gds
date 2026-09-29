// ======================================================================
// \title  GdsBridgeTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <GdsBridge/Top/GdsBridgeTopologyAc.hpp>
// Note: Uncomment when using Svc:TlmPacketizer
//#include <GdsBridge/Top/GdsBridgePacketsAc.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>

// Public functions for use in main program are namespaced with deployment module FprimeGds
// This is also the namespace where the topology components are instantiated by FPP.
namespace FprimeGds {

// Instantiate a malloc allocator for cmdSeq buffer allocation
Fw::MallocAllocator mallocAllocator;
Fw::MemAllocator& bufferPoolAllocator = mallocAllocator;

enum TopologyConstants {
    COMM_PRIORITY = 34,
};

// Period of the downlink rate group that flushes partially filled TM frames out of the aggregator
const Fw::TimeInterval RATE_GROUP_PERIOD(1, 0);

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology() {
    pollingTimer.startTimer(RATE_GROUP_PERIOD);
}

void cycleTopology() {
    pollingTimer.cycle();
}

void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded command registration. Function provided by autocoder.
    //regCommands();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    if (state.hostname != nullptr && state.port != 0) {
        comDriver.configure(state.hostname, state.port);
    }
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);
    // Initialize socket communication if and only if there is a valid specification
    if (state.hostname != nullptr && state.port != 0) {
        Os::TaskString name("ReceiveTask");
        // Uplink is configured for receive so a socket task is started
        comDriver.start(name, COMM_PRIORITY, Default::STACK_SIZE);
    }
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);
    pollingTimer.stop();
    // Stop the server from listening
    comDriver.terminate();
    comDriver.stop();
    (void)comDriver.join();
    tearDownComponents(state);
}
};  // namespace FprimeGds
