#include <gtest/gtest.h>

#include <list>
#include <utility>

#include "../../src/assem_mem.h"
#include "../../src/comet_ii.h"
#include "../test_config.h"

#if TEST_CONFIG_TEST_TEST

using namespace cii;

WordData words[128] = {};
AssmMem asem = {128, words, 0};

TEST(Prog, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::XOR_R, Reg::GR0, Reg::GR0);
    asem << OpWord(OpCode::XOR_R, Reg::GR1, Reg::GR1);
    asem << SymDef("L1");
    asem << OpWord(OpCode::CPA_M, Reg::GR1) << SymRef("LEN");
    asem << OpWord(OpCode::JZE) << SymRef("L2");
    asem << OpWord(OpCode::ADDA_M, Reg::GR0, Reg::GR1) << SymRef("DATA");
    asem << OpWord(OpCode::LAD, Reg::GR1, Reg::GR1) << 1;
    asem << OpWord(OpCode::JUMP) << SymRef("L1");
    asem << SymDef("L2");
    asem << OpWord(OpCode::ST, Reg::GR0) << SymRef("ANS");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("DATA") << 12 << 34 << 56 << 78 << 90;
    asem << SymDef("LEN") << 5;
    asem << SymDef("ANS") << 0;
    bool ret = asem.End();

    EXPECT_EQ(true, ret);

    cii.Run();

    EXPECT_EQ(5, cii.GR1);
    EXPECT_EQ(270, cii.GR0);
}

void CountBitSub(AssmMem &asem) {
    asem << SymDef("COUNT1");
    asem << OpWord(OpCode::PUSH, Reg::GR1) << 0;
    asem << OpWord(OpCode::PUSH, Reg::GR2) << 0;
    asem << OpWord(OpCode::SUBA_R, Reg::GR2, Reg::GR2);
    asem << OpWord(OpCode::AND_R, Reg::GR1, Reg::GR1);
    asem << OpWord(OpCode::JZE) << SymRef("RETURN");
    asem << SymDef("MORE");
    asem << OpWord(OpCode::LAD, Reg::GR2, Reg::GR2) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR0, Reg::GR1) << -1;
    asem << OpWord(OpCode::AND_R, Reg::GR1, Reg::GR0);
    asem << OpWord(OpCode::JNZ) << SymRef("MORE");
    asem << SymDef("RETURN");
    asem << OpWord(OpCode::LD_R, Reg::GR0, Reg::GR2) << 1;
    asem << OpWord(OpCode::POP, Reg::GR2);
    asem << OpWord(OpCode::POP, Reg::GR1);
    asem << OpWord(OpCode::RET);
}

TEST(Prog, 0002) {
    CometII cii(&asem);

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR1) << 0x4f63;
    asem << OpWord(OpCode::CALL) << SymRef("COUNT1");
    asem << OpWord(OpCode::HLT);

    CountBitSub(asem);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(9, cii.GR0);

    //
    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR1) << 0;
    asem << OpWord(OpCode::CALL) << SymRef("COUNT1");
    asem << OpWord(OpCode::HLT);

    CountBitSub(asem);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(0, cii.GR0);
}

TEST(SHIFTA, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 0x44;
    asem << OpWord(OpCode::LAD, Reg::GR2) << 0;
    asem << OpWord(OpCode::SLA, Reg::GR1, Reg::GR2) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(0x88, cii.GR1);
    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR3) << 0x4000;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::SLA, Reg::GR3, Reg::GR2) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(0, cii.GR3);
    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(true, cii.FR.IsZero());

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR3) << -4;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::SLA, Reg::GR3, Reg::GR2) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ((uint16_t)-8, cii.GR3);
    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR3) << 0xc000;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::SLA, Reg::GR3, Reg::GR2) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(0x8000, cii.GR3);
    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
}

TEST(SHIFTL, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 0x44;
    asem << OpWord(OpCode::LAD, Reg::GR2) << 0;
    asem << OpWord(OpCode::SLL, Reg::GR1, Reg::GR2) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(0x88, cii.GR1);
    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR3) << 0x8000;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::SLL, Reg::GR3, Reg::GR4) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(0, cii.GR3);
    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(true, cii.FR.IsZero());

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR3) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::SRL, Reg::GR3, Reg::GR4) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ((uint16_t)0, cii.GR3);
    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(true, cii.FR.IsZero());

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR3) << 0x44;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::SRL, Reg::GR3, Reg::GR2) << 1;

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ((uint16_t)0x22, cii.GR3);
    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
}

TEST(CALL, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::CALL, Reg::GR1) << SymRef("SUB");

    asem << OpWord(OpCode::HLT);

    asem << SymDef("SUB");
    asem << OpWord(OpCode::LAD, Reg::GR1) << 100;
    asem << OpWord(OpCode::RET);

    EXPECT_EQ(true, asem.End());

    cii.Run();

    EXPECT_EQ(100, cii.GR1);
}

TEST(PUSH_POP, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::PUSH, Reg::GR1) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 100;
    asem << OpWord(OpCode::POP, Reg::GR1);

    asem << OpWord(OpCode::LAD, Reg::GR2) << 1;
    asem << OpWord(OpCode::PUSH, Reg::GR2) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR2) << 100;
    asem << OpWord(OpCode::POP, Reg::GR2);

    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::PUSH, Reg::GR7) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR7) << 100;
    asem << OpWord(OpCode::POP, Reg::GR7);

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Run();

    EXPECT_EQ(1, cii.GR1);
    EXPECT_EQ(1, cii.GR2);
    EXPECT_EQ(1, cii.GR7);
}

TEST(JPL, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JPL) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << -1;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JPL) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JPL) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);
}

TEST(JMI, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JMI) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JMI) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JMI) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);
}

TEST(JNZ, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JNZ) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JNZ) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JNZ) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);
}
TEST(JZE, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JZE) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JZE) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::SUBA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JZE) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);
}
TEST(JOV, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JOV) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    bool ret = asem.End();

    EXPECT_EQ(true, ret);

    cii.Reset();
    cii.Run();

    EXPECT_EQ(2, cii.GR7);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 0xffff;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::ADDL_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JOV) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);
}
TEST(JUMP, 0001) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR0, Reg::GR1);
    asem << OpWord(OpCode::JUMP) << SymRef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 2;
    asem << OpWord(OpCode::HLT);
    asem << SymDef("L1");
    asem << OpWord(OpCode::LAD, Reg::GR7) << 1;
    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Reset();
    cii.Run();

    EXPECT_EQ(1, cii.GR7);
}

TEST(LogicalOpe, Reg) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 0xffff;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 0xffff;
    asem << OpWord(OpCode::XOR_R, Reg::GR0, Reg::GR1);

    asem << OpWord(OpCode::LAD, Reg::GR2) << 0xffff;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 0;
    asem << OpWord(OpCode::XOR_R, Reg::GR2, Reg::GR3);

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0x9111;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 0x8888;
    asem << OpWord(OpCode::OR_R, Reg::GR4, Reg::GR5);

    asem << OpWord(OpCode::LAD, Reg::GR6) << 0xffff;
    asem << OpWord(OpCode::LAD, Reg::GR7) << 0x8f66;
    asem << OpWord(OpCode::AND_R, Reg::GR6, Reg::GR7);

    asem << OpWord(OpCode::HLT);

    EXPECT_EQ(true, asem.End());

    cii.Run();

    EXPECT_EQ(0, cii.GR0);
    EXPECT_EQ(0xffff, cii.GR2);
    EXPECT_EQ(0x9999, cii.GR4);
    EXPECT_EQ(0x8f66, cii.GR6);
}

TEST(LogicalOpe, Mem) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR0) << 0xffff;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::XOR_M, Reg::GR0, Reg::GR1) << SymRef("0xffff");

    asem << OpWord(OpCode::LAD, Reg::GR2) << 0xffff;
    asem << OpWord(OpCode::XOR_M, Reg::GR2, Reg::GR1) << SymRef("0");

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0x9111;
    asem << OpWord(OpCode::OR_M, Reg::GR4, Reg::GR1) << SymRef("0x8888");

    asem << OpWord(OpCode::LAD, Reg::GR6) << 0xffff;
    asem << OpWord(OpCode::AND_M, Reg::GR6, Reg::GR1) << SymRef("0x8f66");

    asem << OpWord(OpCode::HLT);

    asem << SymDef("0xffff") << 0 << 0xffff;
    asem << SymDef("0") << 0 << 0;
    asem << SymDef("0x8888") << 0 << 0x8888;
    asem << SymDef("0x8f66") << 0 << 0x8f66;

    EXPECT_EQ(true, asem.End());

    cii.Run();

    EXPECT_EQ(0, cii.GR0);
    EXPECT_EQ(0xffff, cii.GR2);
    EXPECT_EQ(0x9999, cii.GR4);
    EXPECT_EQ(0x8f66, cii.GR6);
}

TEST(Load, LAD) {
    CometII cii(&asem);

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::LAD, Reg::GR2) << 3;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 4;
    asem << OpWord(OpCode::LAD, Reg::GR4) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 6;
    asem << OpWord(OpCode::LAD, Reg::GR6) << 7;
    asem << OpWord(OpCode::LAD, Reg::GR7) << 0xffff;
    asem << OpWord(OpCode::HLT);
    asem.End();

    cii.Run();

    EXPECT_EQ(1, cii.GR0);
    EXPECT_EQ(2, cii.GR1);
    EXPECT_EQ(3, cii.GR2);
    EXPECT_EQ(4, cii.GR3);
    EXPECT_EQ(5, cii.GR4);
    EXPECT_EQ(6, cii.GR5);
    EXPECT_EQ(7, cii.GR6);
    EXPECT_EQ(0xffff, cii.GR7);

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());

    /*
     *
     */
    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::LAD, Reg::GR2, Reg::GR1) << 3;

    asem << OpWord(OpCode::LAD, Reg::GR3) << 7;
    asem << OpWord(OpCode::LAD, Reg::GR5, Reg::GR1) << -1;

    asem.End();

    cii.Run();

    EXPECT_EQ(4, cii.GR3);
    EXPECT_EQ(6, cii.GR5);

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
}

TEST(Load, LAD_Reg) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::LAD, Reg::GR2, Reg::GR1) << 3;

    asem << OpWord(OpCode::LAD, Reg::GR3) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR4, Reg::GR3) << -1;

    asem.End();
    cii.Run();

    EXPECT_EQ(5, cii.GR2);
    EXPECT_EQ(4, cii.GR4);
}

TEST(Load, LD_R) {
    CometII cii(&asem);

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR4) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 6;
    asem << OpWord(OpCode::LAD, Reg::GR6) << 7;
    asem << OpWord(OpCode::LAD, Reg::GR7) << 0xffff;

    asem << OpWord(OpCode::LD_R, Reg::GR0, Reg::GR4);
    asem << OpWord(OpCode::LD_R, Reg::GR1, Reg::GR5);
    asem << OpWord(OpCode::LD_R, Reg::GR2, Reg::GR6);
    asem << OpWord(OpCode::LD_R, Reg::GR3, Reg::GR7);
    asem.End();
    cii.Run();

    EXPECT_EQ(5, cii.GR0);
    EXPECT_EQ(6, cii.GR1);
    EXPECT_EQ(7, cii.GR2);
    EXPECT_EQ(0xffff, cii.GR3);

    /*
     *
     */
    cii.Reset();

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR1) << 2;
    asem << OpWord(OpCode::LAD, Reg::GR2) << 3;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 4;

    asem << OpWord(OpCode::LD_R, Reg::GR4, Reg::GR0);
    asem << OpWord(OpCode::LD_R, Reg::GR5, Reg::GR1);
    asem << OpWord(OpCode::LD_R, Reg::GR6, Reg::GR2);
    asem << OpWord(OpCode::LD_R, Reg::GR7, Reg::GR3);

    asem.End();
    cii.Run();

    EXPECT_EQ(1, cii.GR4);
    EXPECT_EQ(2, cii.GR5);
    EXPECT_EQ(3, cii.GR6);
    EXPECT_EQ(4, cii.GR7);

    /*
     * Overflow flag
     */
    cii.Reset();

    cii.FR.OF = ON;
    cii.FR.SF = ON;
    cii.FR.ZF = ON;

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR0) << 1;
    asem << OpWord(OpCode::LD_R, Reg::GR1, Reg::GR0);
    asem.End();

    cii.Run();

    EXPECT_EQ(1, cii.GR1);
    EXPECT_EQ(OFF, cii.FR.OF);
    EXPECT_EQ(OFF, cii.FR.SF);
    EXPECT_EQ(OFF, cii.FR.ZF);
    /*
     * Signed flag
     */
    cii.Reset();

    cii.FR.OF = ON;
    cii.FR.SF = OFF;
    cii.FR.ZF = ON;

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR0) << 0xffff;
    asem << OpWord(OpCode::LD_R, Reg::GR1, Reg::GR0);
    asem.End();
    cii.Run();

    EXPECT_EQ(0xffff, cii.GR1);
    EXPECT_EQ(OFF, cii.FR.OF);
    EXPECT_EQ(ON, cii.FR.SF);
    EXPECT_EQ(OFF, cii.FR.ZF);
    /*
     * Zero Flag
     */
    cii.Reset();

    cii.FR.OF = ON;
    cii.FR.SF = ON;
    cii.FR.ZF = OFF;

    asem.Start();
    asem << OpWord(OpCode::LAD, Reg::GR0) << 0;
    asem << OpWord(OpCode::LD_R, Reg::GR1, Reg::GR0);
    asem.End();

    cii.Run();

    EXPECT_EQ(0, cii.GR1);
    EXPECT_EQ(OFF, cii.FR.OF);
    EXPECT_EQ(OFF, cii.FR.SF);
    EXPECT_EQ(ON, cii.FR.ZF);
}
TEST(Load, LD_M) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 2;
    asem << OpWord(OpCode::LD_M, Reg::GR0) << SymRef("MEM1");
    asem << OpWord(OpCode::LD_M, Reg::GR2, Reg::GR1) << SymRef("MEM2");
    asem << OpWord(OpCode::LD_M, Reg::GR4, Reg::GR3) << SymRef("MEM2");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 11;
    asem << SymDef("MEM2") << 5 << 13 << 0xffff;

    asem.End();

    EXPECT_EQ(true, asem.End());

    cii.Run();

    EXPECT_EQ(11, cii.GR0);
    EXPECT_EQ(1, cii.GR1);
    EXPECT_EQ(13, cii.GR2);
    EXPECT_EQ(0xffff, cii.GR4);
}

TEST(Load, ADDA_R) {
    CometII cii(&asem);
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 2;
    asem << OpWord(OpCode::ADDA_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(3, cii.GR1);

    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 3;
    asem << OpWord(OpCode::LAD, Reg::GR3) << -3;
    asem << OpWord(OpCode::ADDA_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(true, cii.FR.IsZero());
    EXPECT_EQ(0, cii.GR1);
    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR3) << -6;
    asem << OpWord(OpCode::ADDA_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ((uint16_t)-1, cii.GR1);
    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 32767;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 1;
    asem << OpWord(OpCode::ADDA_R, Reg::GR4, Reg::GR5);

    asem.End();

    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(0x8000, cii.GR4);

    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << -32768;
    asem << OpWord(OpCode::LAD, Reg::GR5) << -1;
    asem << OpWord(OpCode::ADDA_R, Reg::GR4, Reg::GR5);

    asem.End();

    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(0x7fff, cii.GR4);
}

TEST(Load, ADDA_M) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 2;
    asem << OpWord(OpCode::ADDA_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 1 << 2;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(4, cii.GR5);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR5) << -2;
    asem << OpWord(OpCode::ADDA_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 1;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ((uint16_t)-1, cii.GR5);
}

TEST(Load, SUBA_R) {
    CometII cii(&asem);
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 3;
    asem << OpWord(OpCode::SUBA_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(2, cii.GR1);

    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 3;
    asem << OpWord(OpCode::LAD, Reg::GR3) << -3;
    asem << OpWord(OpCode::ADDA_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(true, cii.FR.IsZero());
    EXPECT_EQ(0, cii.GR1);
    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR3) << -6;
    asem << OpWord(OpCode::ADDA_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ((uint16_t)-1, cii.GR1);
    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << -1;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 0x8000;
    asem << OpWord(OpCode::ADDA_R, Reg::GR4, Reg::GR5);

    asem.End();

    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(0x7fff, cii.GR4);

    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << -32768;
    asem << OpWord(OpCode::LAD, Reg::GR5) << -1;
    asem << OpWord(OpCode::ADDA_R, Reg::GR4, Reg::GR5);

    asem.End();

    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(0x7fff, cii.GR4);
}
TEST(Load, SUBA_M) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 5;
    asem << OpWord(OpCode::SUBA_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 1 << 2;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(3, cii.GR5);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR5) << -2;
    asem << OpWord(OpCode::SUBA_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << -1;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ((int16_t)-1, (int16_t)cii.GR5);
}

TEST(Load, ADDL_R) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 5;
    asem << OpWord(OpCode::ADDL_R, Reg::GR5, Reg::GR4);
    asem << OpWord(OpCode::HLT);

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(6, cii.GR5);

    /*
     *
     */
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0xfffe;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 1;
    asem << OpWord(OpCode::ADDL_R, Reg::GR5, Reg::GR4);
    asem << OpWord(OpCode::HLT);

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(0xffff, cii.GR5);

    /*
     *
     */
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0xfffe;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 2;
    asem << OpWord(OpCode::ADDL_R, Reg::GR5, Reg::GR4);
    asem << OpWord(OpCode::HLT);

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(true, cii.FR.IsZero());
    EXPECT_EQ(0, cii.GR5);
}

TEST(Load, ADDL_M) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 2;
    asem << OpWord(OpCode::ADDL_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 1 << 2;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(4, cii.GR5);
}
TEST(Load, SUBL_R) {
    CometII cii(&asem);
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 5;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 3;
    asem << OpWord(OpCode::SUBL_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(2, cii.GR1);

    //
    cii.Reset();
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR1) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR3) << 1;
    asem << OpWord(OpCode::SUBL_R, Reg::GR1, Reg::GR3);

    asem.End();

    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(0xffff, cii.GR1);
}

TEST(Load, SUBL_M) {
    CometII cii(&asem);

    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 1;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 5;
    asem << OpWord(OpCode::SUBL_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 1 << 2;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(false, cii.FR.IsOverflow());
    EXPECT_EQ(false, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ(3, cii.GR5);

    //
    asem.Start();

    asem << OpWord(OpCode::LAD, Reg::GR4) << 0;
    asem << OpWord(OpCode::LAD, Reg::GR5) << 10;
    asem << OpWord(OpCode::SUBL_M, Reg::GR5, Reg::GR4) << SymRef("MEM1");
    asem << OpWord(OpCode::HLT);
    asem << SymDef("MEM1") << 11;

    asem.End();

    cii.Reset();
    cii.Run();

    EXPECT_EQ(true, cii.FR.IsOverflow());
    EXPECT_EQ(true, cii.FR.IsSigned());
    EXPECT_EQ(false, cii.FR.IsZero());
    EXPECT_EQ((int16_t)-1, (int16_t)cii.GR5);
}

#endif