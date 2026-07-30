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

This application is not built standalone: it is compiled as part of a cFS mission build. See
[fprime_cfs_reference](https://github.com/fprime-community/fprime_cfs_reference) for a complete
system that builds and runs this application.
