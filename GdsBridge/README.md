# GdsBridge

An F Prime deployment that runs as a cFS application and bridges the cFS software bus to the
F Prime GDS. The deployment's topology is set up and torn down by the cFS application entry point
in `Main.cpp` (`FPRIME_GDS_Main`), which is registered with cFE Executive Services rather than run
as a standalone binary.

The deployment:

1. Configures the `FPrimeCfs::CfsBridge` with a software bus pipe (flow controlled) and subscribes
   to the F Prime downlink APIDs (telemetry, events, files, data products, packetized telemetry).
2. Sets up the F Prime topology (`Top/`), which hosts a TCP server (port 15010) that the F Prime
   GDS connects to.
3. Drives the bridge from the cFS run loop via `cfsBridge.process()`.

## Initialization error handling

Initialization failures are fatal to the application. If pipe creation or any downlink APID
subscription fails during `FPRIME_GDS_Init`, an EVS error event is emitted (event IDs 2 and 3
respectively; the subscription event identifies the failing APID), the topology is not started,
and the application exits via `CFE_ES_ExitApp(CFE_ES_RunStatus_APP_ERROR)`. On success an
informational event (event ID 1) reports the application version.

This application is not built standalone: it is compiled as part of a cFS mission build. See
[fprime_cfs_reference](https://github.com/fprime-community/fprime_cfs_reference) for a complete
system that builds and runs this application.
