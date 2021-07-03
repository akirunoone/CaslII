#include <gtest/gtest.h>

#include <list>
#include <utility>

#include "../../src/assem_mem.h"
#include "../../src/assembler.h"
#include "../../src/comet_ii.h"
#include "../test_base.h"
#include "../test_config.h"

#if TEST_CONFIG_ASSEMBLER_TEST

namespace {
class AssTest : public TestBase<1024> {
   protected:
    ass::Assembler assem;
    void SetUp() {}
    void TearDown() {}
};

TEST_F(AssTest, LAD_0001) {
    mem.Start();

    // cii_cpu.GR2 = 10;

    // assem.Assemble("LAD GR1,3", mem);
    // assem.Assemble("LAD GR2,13", mem);
    // assem.Assemble("LAD GR3,5,GR1", mem);
    assem.Assemble("LAD GR4,-1", mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Run();

    // EXPECT_EQ(3, cii_cpu.GR1);
    // EXPECT_EQ(13, cii_cpu.GR2);
    // EXPECT_EQ(8, cii_cpu.GR3);
    EXPECT_EQ((int16_t)-1, (int16_t)cii_cpu.GR4);
}

TEST_F(AssTest, LD_0001) {
    std::stringstream ss{
        " LAD GR4,3\n"
        " LAD GR5,1\n"
        " LD GR6,GR4\n"
        " LD GR7,MEM,GR5\n"
        " HLT\n"
        "MEM DC 0\n"
        "    DC 11\n"};

    mem.Start();

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR6);
    EXPECT_EQ(11, cii_cpu.GR7);
}

TEST_F(AssTest, LD_0002) {
    std::stringstream ss{
        " LD GR6,=1\n"
        " LD GR7,=-1\n"
        " HLT\n"
        ""};

    mem.Start();

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    // mem.Dump();

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(1, cii_cpu.GR6);
    EXPECT_EQ(0xffff, cii_cpu.GR7);
}
TEST_F(AssTest, LD_0003) {
    std::stringstream ss{
        " LD GR6,=#ABC\n"
        " LD GR7,=#1234\n"
        " HLT\n"
        ""};

    mem.Start();

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    // mem.Dump();

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(0xabc, cii_cpu.GR6);
    EXPECT_EQ(0x1234, cii_cpu.GR7);
}
TEST_F(AssTest, LD_0004) {
    std::stringstream ss{
        " LD GR6,='A'\n"
        " LD GR7,='B'\n"
        " LD GR0,='C'\n"
        " HLT\n"
        ""};

    mem.Start();

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    mem.Dump();

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ('A', cii_cpu.GR6);
    EXPECT_EQ('B', cii_cpu.GR7);
    EXPECT_EQ('C', cii_cpu.GR0);
}

TEST_F(AssTest, LAD_0002) {
    std::stringstream ss{
        " XOR GR0,GR0\n"
        " LAD GR0,2,GR0\n"
        " HLT\n"};

    mem.Start();

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(2, cii_cpu.GR0);
}

TEST_F(AssTest, ST_0001) {
    mem.Start();

    assem.Assemble("LAD GR1,3", mem);
    assem.Assemble("LAD GR2,5", mem);
    assem.Assemble("LAD GR3,1", mem);
    assem.Assemble("ST GR1,MEM", mem);
    assem.Assemble("ST GR2,MEM,GR3", mem);
    assem.Assemble("LD GR4,MEM", mem);
    assem.Assemble("LD GR5,MEM,GR3", mem);

    mem << cii::Halt();

    mem << cii::SymDef("MEM") << 1 << 11;

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR1);
    EXPECT_EQ(5, cii_cpu.GR2);
    EXPECT_EQ(1, cii_cpu.GR3);
    uint16_t m1 = 0;
    uint16_t m2 = 0;
    bool ret = mem.Dump("MEM", m1);
    mem.Dump("MEM", m2, 1);
    EXPECT_EQ(true, ret);
    if (ret) {
        EXPECT_EQ(3, m1);
        EXPECT_EQ(5, m2);
    }

    EXPECT_EQ(3, cii_cpu.GR4);
    EXPECT_EQ(5, cii_cpu.GR5);
}

TEST_F(AssTest, ADDA_0001) {
    mem.Start();

    assem.Assemble("LAD GR4,3", mem);
    assem.Assemble("LAD GR5,1", mem);
    assem.Assemble("ADDA GR5,GR4", mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR4);
    EXPECT_EQ(4, cii_cpu.GR5);
}

TEST_F(AssTest, ADDL_0001) {
    mem.Start();

    std::stringstream ss{
        " LAD GR4,3\n"
        " LAD GR5,5\n"
        " ADDL GR5,GR4\n"};

    assem.Assemble(ss, mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR4);
    EXPECT_EQ(8, cii_cpu.GR5);
}

TEST_F(AssTest, DC_0001) {
    mem.Start();

    std::stringstream ss{
        " DC 1    \n"
        "   DC 2\n"
        "   DC 3,4 \n"
        "DATA    DC        12,34,56,78,90  \n"
        " DC -1,-2 \n"
        " DC #10 \n"};

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    // mem.Dump();

    EXPECT_EQ(1, words[0]);
    EXPECT_EQ(2, words[1]);
    EXPECT_EQ(3, words[2]);
    EXPECT_EQ(4, words[3]);
    EXPECT_EQ(12, words[4]);
    EXPECT_EQ(34, words[5]);
    EXPECT_EQ(56, words[6]);
    EXPECT_EQ(78, words[7]);
    EXPECT_EQ(90, words[8]);
    EXPECT_EQ(-1, (int)(int16_t)words[9]);
    EXPECT_EQ(-2, (int)(int16_t)words[10]);
    EXPECT_EQ(16, words[11]);
}

TEST_F(AssTest, SUBA_0001) {
    mem.Start();

    std::stringstream ss{
        " LAD GR4,3\n"
        " LAD GR5,5\n"
        " SUBA GR5,GR4\n"};

    assem.Assemble(ss, mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR4);
    EXPECT_EQ(2, cii_cpu.GR5);
}

TEST_F(AssTest, SUBL_0001) {
    mem.Start();

    std::stringstream ss{
        " LAD GR4,3\n"
        " LAD GR5,5\n"
        " SUBL GR5,GR4\n"};

    assem.Assemble(ss, mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR4);
    EXPECT_EQ(2, cii_cpu.GR5);
}

TEST_F(AssTest, AND_0001) {
    mem.Start();

    std::stringstream ss{
        " LAD GR4,3\n"
        " LAD GR5,5\n"
        " AND GR5,GR4\n"};

    assem.Assemble(ss, mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR4);
    EXPECT_EQ(1, cii_cpu.GR5);
}

TEST_F(AssTest, PUSH_0001) {
    mem.Start();

    std::stringstream ss{
        " LAD  GR1,1 \n"
        " LAD  GR2,2 \n"
        " PUSH 0,GR1 \n"
        " PUSH 0,GR2 \n"
        " LAD  GR1,-1 \n"
        " LAD  GR2,-1 \n"
        " POP  GR2 \n"
        " POP  GR1 \n"
        ""};

    assem.Assemble(ss, mem);

    mem << cii::Halt();

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(1, cii_cpu.GR1);
    EXPECT_EQ(2, cii_cpu.GR2);
}

TEST_F(AssTest, CALL_0001) {
    mem.Start();

    std::stringstream ss{
        "        CALL      SUB\n"
        "        HLT          \n"
        "SUB   	 LD        GR0,DATA1\n"
        "        ADDA      GR0,DATA2\n"
        "        ST        GR0,ANS\n"
        "        RET              \n"
        "DATA1 	 DC        123\n"
        "DATA2 	 DC        456\n"
        "ANS     DS        1  \n"};

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(579, cii_cpu.GR0);
}
TEST_F(AssTest, CALL_0002) {
    mem.Start();

    std::stringstream ss{
        "        CALL      PROG1\n"
        "        HLT          \n"
        "PROG1   START                     \n"
        "        XOR       GR0,GR0         \n"
        "        XOR       GR1,GR1         \n"
        "L1      CPA       GR1,LEN         \n"
        "        JZE       L2              \n"
        "        ADDA      GR0,DATA,GR1    \n"
        "        LAD       GR1,1,GR1       \n"
        "        JUMP      L1              \n"
        "L2      ST        GR0,ANS         \n"
        "        RET                       \n"
        "DATA    DC        12,34,56,78,90  \n"
        "LEN     DC        5               \n"
        "ANS     DS        1               \n"
        "        END                       \n"};

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();
    // mem.Dump();
    EXPECT_EQ(270, cii_cpu.GR0);
}

TEST_F(AssTest, CALL_0003) {
    mem.Start();

    std::stringstream ss{
        "   LAD   GR1,7       \n"
        "   CALL  L1          \n"
        "   HLT               \n"
        "L1 PUSH  0,GR1        \n"
        "   PUSH  0,GR2        \n"
        "   SUBA  GR2,GR2      \n"
        "   AND    GR1,GR1      \n"
        "   JZE    RETURN      \n"
        "MORE  LAD    GR2,1,GR2  \n"
        "   LAD    GR0,-1,GR1  \n"
        "   AND    GR1,GR0      \n"
        "   JNZ    MORE        \n"
        "RETURN  LD    GR0,GR2  \n"
        "   POP    GR2          \n"
        "   POP    GR1          \n"
        "   RET                \n"
        ""};

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(3, cii_cpu.GR0);
}

TEST_F(AssTest, CPL_0001) {
    mem.Start();

    std::stringstream ss{
        "   LAD    GR0,#FFFF    \n"
        "   LAD    GR1,#FFF0    \n"
        "L1 CPL    GR0,GR1      \n"
        "   JZE    L2           \n"
        "   LAD    GR1,1,GR1    \n"
        "   JUMP   L1           \n"
        "L2                     \n"
        "   HLT                 \n"
        ""};

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    // mem.Dump();

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(0xffff, cii_cpu.GR1);
}

TEST_F(AssTest, OUT_0001) {
    mem.Start();

    std::stringstream ss{
        "   OUT    MSG,LEN      \n"
        "   HLT                 \n"
        "MSG  DC 'Hello ''World!''' \n"
        "LEN  DC  14             \n"
        ""};

    assem.Assemble(ss, mem);

    EXPECT_EQ(true, mem.End());

    // mem.Dump();

    std::stringstream os;

    cii_cpu.SetSvcOut(os);

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ("Hello 'World!'\n", os.str());
}

TEST_F(AssTest, IN_0001) {
    mem.Start();

    std::stringstream ss{
        "   IN    MSG,LEN      \n"
        "   HLT                 \n"
        "MSG  DS 256\n"
        "LEN  DS 1              \n"
        ""};

    assem.Assemble(ss, mem);
    EXPECT_EQ(true, mem.End());

    uint16_t msg_offset = mem.FindSym("MSG");
    uint16_t len_offset = mem.FindSym("LEN");

    std::stringstream sin{"Hello"};
    cii_cpu.SetSvcIn(sin);

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(5, mem.memory[len_offset]);
    EXPECT_EQ('H', mem.memory[msg_offset]);
    EXPECT_EQ('e', mem.memory[msg_offset + 1]);
    EXPECT_EQ('l', mem.memory[msg_offset + 2]);
    EXPECT_EQ('l', mem.memory[msg_offset + 3]);
    EXPECT_EQ('o', mem.memory[msg_offset + 4]);
}

TEST_F(AssTest, ERR_0001) {
    mem.Start();

    std::stringstream ss{
        " GR1"
        ""};

    assem.Assemble(ss, mem);

    EXPECT_EQ(ass::AsmErrCode::NO_OPERATION, assem.error);
}

TEST_F(AssTest, ERR_0002) {
    mem.Start();

    std::stringstream ss{
        " ,"
        ""};

    assem.Assemble(ss, mem);

    EXPECT_EQ(ass::AsmErrCode::NO_OPERATION, assem.error);
}

TEST_F(AssTest, ERR_0003) {
    mem.Start();

    std::stringstream ss{
        "LABEL"
        ""};

    assem.Assemble(ss, mem);

    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}

TEST_F(AssTest, ERR_0004) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " LABEL   ;kkkkkkkk"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}

TEST_F(AssTest, ERR_0005) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        "LABEL   LDDDD"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::NO_OPERATION, assem.error);
}
TEST_F(AssTest, ERR_0006) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        "1111"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::NO_OPERATION, assem.error);
}
TEST_F(AssTest, ERR_0007) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " POP GR1"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}
TEST_F(AssTest, ERR_0008) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " POP GR9"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::INVALID_OPERAND, assem.error);
}
TEST_F(AssTest, ERR_0009) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " POP GR7,"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::INVALID_OPERAND, assem.error);
}
TEST_F(AssTest, ERR_0010) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " POP GR7 ;ddddddd"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}

TEST_F(AssTest, ERR_0011) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " RET"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}
TEST_F(AssTest, ERR_0012) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " RET ;llll"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}
TEST_F(AssTest, ERR_0013) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " NOP"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}

TEST_F(AssTest, ERR_0014) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " RET GR0"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::INVALID_OPERAND, assem.error);
}

TEST_F(AssTest, ERR_0020) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " LAD GR1, AAAAA"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}

TEST_F(AssTest, ERR_0022) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " LAD GR2, 10,GR1"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}

TEST_F(AssTest, ERR_0023) {
    mem.Start();

    std::stringstream ss{
        "DATA    DC        12,34,56,78,90  \n"
        "DATA     DC        5               \n"
        "ANS     DS        1               \n"};

    assem.Assemble(ss, mem);

    EXPECT_EQ(ass::AsmErrCode::MULTI_DEF_SYM, assem.dbg_infos[1].err);
}
TEST_F(AssTest, ERR_0024) {
    mem.Start();

    std::stringstream ss{
        ";AAA  \n"
        ""};

    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::OK, assem.error);
}
TEST_F(AssTest, ERR_0025) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " DC ####"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::INVALID_OPERAND, assem.error);
}

TEST_F(AssTest, ERR_0026) {
    //
    mem.Start();
    std::stringstream ss = std::stringstream{
        " DC 10 ###"
        ""};
    assem.Assemble(ss, mem);
    EXPECT_EQ(ass::AsmErrCode::INVALID_OPERAND, assem.error);
}

}  // namespace
#endif