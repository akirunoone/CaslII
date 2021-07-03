#include <gtest/gtest.h>

#include <iostream>
#include <list>
#include <map>
#include <utility>

#include "../../src/reader.h"
#include "../test_base.h"
#include "../test_config.h"

#if TEST_CONFIG_READER_TEST

namespace {

class ReaderTest : public ::testing::Test {
   protected:
    ass::Reader reader;
    void SetUp() {}
    void TearDown() {}
};

std::ostream& operator<<(std::ostream& os, ass::TokenId& tid) {
    for (auto& v : ass::Reader::key_words) {
        if (v.second.token_id == tid) {
            os << v.first;
            return os;
        }
    }
    if (tid == ass::TokenId::DIGIT) {
        os << "DIGIT";
        return os;
    } else if (tid == ass::TokenId::LABEL) {
        os << "LABEL";
        return os;
    } else if (tid == ass::TokenId::OTHER) {
        os << "OTHER";
        return os;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, ass::Tokens& tokens) {
    for (auto&& t : tokens) {
        os << "token_id:" << t.token_id;
        if (t.token_id == ass::TokenId::LABEL) {
            os << ":" << t.label;
        } else if (t.token_id == ass::TokenId::DIGIT) {
            os << ":" << t.digit;
        } else if (t.token_id == ass::TokenId::OTHER) {
            os << ":" << t.label;
        }
        os << std::endl;
    }
    return os;
}

inline int CastInt(uint16_t d) { return (int)(int16_t)d; }

TEST_F(ReaderTest, 0001) {
    std::string line{"L1 LD GR1,-1,GR2"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(7, tokens.size());
    EXPECT_EQ(ass::TokenId::LABEL, tokens[0].token_id);
    EXPECT_EQ(std::string("L1"), tokens[0].label);
    EXPECT_EQ(ass::TokenId::LD, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[3].token_id);
    EXPECT_EQ(ass::TokenId::DIGIT, tokens[4].token_id);
    EXPECT_EQ(-1, CastInt(tokens[4].digit));
    EXPECT_EQ(ass::TokenId::COMMA, tokens[5].token_id);
    EXPECT_EQ(ass::TokenId::GR2, tokens[6].token_id);
}

TEST_F(ReaderTest, 0002) {
    std::string line{"L2  DS 10"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(3, tokens.size());
    EXPECT_EQ(ass::TokenId::LABEL, tokens[0].token_id);
    EXPECT_EQ(std::string("L2"), tokens[0].label);
    EXPECT_EQ(ass::TokenId::DS, tokens[1].token_id);
    EXPECT_EQ(10, CastInt(tokens[2].digit));
}
TEST_F(ReaderTest, 0003) {
    std::string line{"  DC 1,2,#10,4"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(8, tokens.size());
    EXPECT_EQ(ass::TokenId::DC, tokens[0].token_id);
    EXPECT_EQ(1, tokens[1].digit);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(2, tokens[3].digit);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[4].token_id);
    EXPECT_EQ(16, tokens[5].digit);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[6].token_id);
    EXPECT_EQ(4, tokens[7].digit);
}
TEST_F(ReaderTest, 0004) {
    std::string line{"  LD GR1,=10"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(4, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::CONST, tokens[3].token_id);
    EXPECT_EQ(10, tokens[3].digit);
}
TEST_F(ReaderTest, 0005) {
    std::string line{" LD GR1   ;,=10"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(2, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    // delete EXPECT_EQ(ass::TokenId::COMMENT, tokens[2].token_id);
}
TEST_F(ReaderTest, 0006) {
    std::string line{" LD GR1,=#10"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(4, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::CONST, tokens[3].token_id);
    EXPECT_EQ(16, tokens[3].digit);
}
TEST_F(ReaderTest, 0007) {
    std::string line{" LD GR1,=#10Z"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(4, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::OTHER, tokens[3].token_id);
}
TEST_F(ReaderTest, 0008) {
    std::string line{" LD GR1,#10Z"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(4, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::OTHER, tokens[3].token_id);
}

TEST_F(ReaderTest, 0009) {
    std::string line{" LD GR1,10Z"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(4, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::OTHER, tokens[3].token_id);
}
TEST_F(ReaderTest, 0010) {
    std::string line{" LD GR1,A"};

    ass::Tokens tokens = reader.Parse(line);

    EXPECT_EQ(4, tokens.size());
    EXPECT_EQ(ass::TokenId::LD, tokens[0].token_id);
    EXPECT_EQ(ass::TokenId::GR1, tokens[1].token_id);
    EXPECT_EQ(ass::TokenId::COMMA, tokens[2].token_id);
    EXPECT_EQ(ass::TokenId::LABEL, tokens[3].token_id);
}

}  // namespace
#endif