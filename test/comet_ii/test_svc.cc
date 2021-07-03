#include <gtest/gtest.h>

#include <list>
#include <utility>

#include "../test_base.h"
#include "../test_config.h"
#include "assem_mem.h"
#include "comet_ii.h"

#if TEST_CONFIG_SVC_TEST

namespace {
class SVCTest : public TestBase<1024> {
   protected:
    // cii::WordData words[1024] = {};
    // cii::AsmMem assmMem = {(uint32_t)std::size(words), words, 0};
    // cii::CommetII cii_cpu = {&asem};

    void SetUp() {}
    void TearDown() {}
};

TEST_F(SVCTest, SVC_IN_001) {
    mem.Start();
    mem << cii::OpWord(cii::OpCode::LAD, cii::Reg::GR1) << 5;
    mem << cii::OpWord(cii::OpCode::LAD, cii::Reg::GR2) << 7;
    mem << cii::IOSVC(cii::SVCNo::SVC_IN, "INBUF", "LEN");
    mem << cii::Halt();
    uint16_t buff_offset = mem.GetOffset();
    mem << cii::SymDS("INBUF", 256);
    uint16_t len_offset = mem.GetOffset();
    mem << cii::SymDC("LEN", 0);

    EXPECT_EQ(true, mem.End());

    // mem.Dump();

    std::stringstream ss{"Hello"};
    cii_cpu.SetSvcIn(ss);

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(5, cii_cpu.GR1);
    EXPECT_EQ(7, cii_cpu.GR2);

    uint16_t len = mem.memory[len_offset];

    EXPECT_EQ(5, len);
    EXPECT_EQ('H', mem.memory[buff_offset + 0]);
    EXPECT_EQ('e', mem.memory[buff_offset + 1]);
    EXPECT_EQ('l', mem.memory[buff_offset + 2]);
    EXPECT_EQ('l', mem.memory[buff_offset + 3]);
    EXPECT_EQ('o', mem.memory[buff_offset + 4]);
}
TEST_F(SVCTest, SVC_IN_002) {
    mem.Start();
    mem << cii::OpWord(cii::OpCode::LAD, cii::Reg::GR1) << 5;
    mem << cii::OpWord(cii::OpCode::LAD, cii::Reg::GR2) << 7;
    mem << cii::IOSVC(cii::SVCNo::SVC_IN, "INBUF", "LEN");
    mem << cii::Halt();
    uint16_t buff_offset = mem.GetOffset();
    mem << cii::SymDS("INBUF", 256);
    uint16_t len_offset = mem.GetOffset();
    mem << cii::SymDC("LEN", 0);

    EXPECT_EQ(true, mem.End());

    std::stringstream ss;
    cii_cpu.SetSvcIn(ss);

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(5, cii_cpu.GR1);
    EXPECT_EQ(7, cii_cpu.GR2);

    uint16_t len = mem.memory[len_offset];

    EXPECT_EQ(0xffff, len);
}

TEST_F(SVCTest, SVC_OUT) {
    std::string msg = "Hello World!";
    mem.Start();
    mem << cii::OpWord(cii::OpCode::LAD, cii::Reg::GR1) << 5;
    mem << cii::OpWord(cii::OpCode::LAD, cii::Reg::GR2) << 7;
    mem << cii::IOSVC(cii::SVCNo::SVC_OUT, "OUTBUF", "LEN");
    mem << cii::Halt();
    mem << cii::SymDef("OUTBUF") << msg.c_str() << cii::SymDC("LEN", msg.size());

    EXPECT_EQ(true, mem.End());

    std::stringstream os;
    cii_cpu.SetSvcOut(os);

    cii_cpu.Reset();
    cii_cpu.Run();

    EXPECT_EQ(5, cii_cpu.GR1);
    EXPECT_EQ(7, cii_cpu.GR2);

    EXPECT_EQ("Hello World!\n", os.str());
}
}  // namespace
#endif