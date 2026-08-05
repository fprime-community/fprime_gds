// ======================================================================
// \title  CfsTlmStripperTestMain.cpp
// \brief  cpp file for CfsTlmStripper component test main function
// ======================================================================

#include "CfsTlmStripperTester.hpp"
#include "Fw/Test/UnitTest.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, TelemetryStrip) {
    COMMENT("A telemetry packet's cFS secondary header is stripped in place with a rebuilt primary header");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-001");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-002");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-003");
    FprimeGds::CfsTlmStripperTester tester;
    tester.testTelemetryStrip();
}

TEST(Nominal, CommandPassthrough) {
    COMMENT("A command packet is forwarded unchanged");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-004");
    FprimeGds::CfsTlmStripperTester tester;
    tester.testCommandPassthrough();
}

TEST(Nominal, NoSecHdrPassthrough) {
    COMMENT("A telemetry packet without the secondary header flag is forwarded unchanged");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-004");
    FprimeGds::CfsTlmStripperTester tester;
    tester.testNoSecHdrPassthrough();
}

TEST(OffNominal, MalformedPacket) {
    COMMENT("A buffer too small for a primary header is dropped with a warning and returned");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-005");
    FprimeGds::CfsTlmStripperTester tester;
    tester.testMalformedPacket();
}

TEST(Nominal, DataReturnPassthrough) {
    COMMENT("dataReturnIn is passed through to dataReturnOut");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-006");
    FprimeGds::CfsTlmStripperTester tester;
    tester.testDataReturnPassthrough();
}

TEST(Nominal, ComStatusPassthrough) {
    COMMENT("comStatusIn is passed through to comStatusOut");
    REQUIREMENT("FPRIMEGDS-CFSTLMSTRIPPER-007");
    FprimeGds::CfsTlmStripperTester tester;
    tester.testComStatusPassthrough();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}
