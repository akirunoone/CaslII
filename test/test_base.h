#ifndef TEST_BASE_H_
#define TEST_BASE_H_

#include "../src/assem_mem.h"
#include "../src/comet_ii.h"
#include "../src/reader.h"

template <int MEM_SIZE = 1024>
class TestBase : public ::testing::Test {
   protected:
    cii::WordData words[MEM_SIZE] = {};
    cii::AssmMem mem = {(uint32_t)MEM_SIZE, words, 0};
    cii::CometII cii_cpu = {&mem};
    void SetUp() {}
    void TearDown() {}
};
#endif