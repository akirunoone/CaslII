
#include "reader.h"

#include <iostream>
#include <map>
#include <regex>
#include <string>

namespace ass {

std::map<std::string, TokenInfo> Reader::key_words = {
    {"GR0", TokenInfo(TokenId::GR0)},
    {"GR1", TokenInfo(TokenId::GR1)},
    {"GR2", TokenInfo(TokenId::GR2)},
    {"GR3", TokenInfo(TokenId::GR3)},
    {"GR4", TokenInfo(TokenId::GR4)},
    {"GR5", TokenInfo(TokenId::GR5)},
    {"GR6", TokenInfo(TokenId::GR6)},
    {"GR7", TokenInfo(TokenId::GR7)},
    {"ST", TokenInfo(TokenId::ST, REG_EADR)},
    {"LD", TokenInfo(TokenId::LD, REG_REGorMEM)},
    {"LAD", TokenInfo(TokenId::LAD, REG_EADR)},
    {"ADDA", TokenInfo(TokenId::ADDA, REG_REGorMEM)},
    {"ADDL", TokenInfo(TokenId::ADDL, REG_REGorMEM)},
    {"SUBA", TokenInfo(TokenId::SUBA, REG_REGorMEM)},
    {"SUBL", TokenInfo(TokenId::SUBL, REG_REGorMEM)},
    {"AND", TokenInfo(TokenId::AND, REG_REGorMEM)},
    {"OR", TokenInfo(TokenId::OR, REG_REGorMEM)},
    {"XOR", TokenInfo(TokenId::XOR, REG_REGorMEM)},
    {"CPA", TokenInfo(TokenId::CPA, REG_REGorMEM)},
    {"CPL", TokenInfo(TokenId::CPL, REG_REGorMEM)},
    {"SLA", TokenInfo(TokenId::SLA, REG_EADR)},
    {"SRA", TokenInfo(TokenId::SRA, REG_EADR)},
    {"SLL", TokenInfo(TokenId::SLL, REG_EADR)},
    {"SRL", TokenInfo(TokenId::SRL, REG_EADR)},
    {"JPL", TokenInfo(TokenId::JPL, EADR)},
    {"JMI", TokenInfo(TokenId::JMI, EADR)},
    {"JNZ", TokenInfo(TokenId::JNZ, EADR)},
    {"JZE", TokenInfo(TokenId::JZE, EADR)},
    {"JOV", TokenInfo(TokenId::JOV, EADR)},
    {"JUMP", TokenInfo(TokenId::JUMP, EADR)},
    {"PUSH", TokenInfo(TokenId::PUSH, EADR)},
    {"POP", TokenInfo(TokenId::POP, REG)},
    {"CALL", TokenInfo(TokenId::CALL, EADR)},
    {"RET", TokenInfo(TokenId::RET, NONE)},
    {"CALL", TokenInfo(TokenId::CALL, EADR)},
    {"SVC", TokenInfo(TokenId::SVC, EADR)},
    {"NOP", TokenInfo(TokenId::NOP, NONE)},
    {"HLT", TokenInfo(TokenId::HLT, NONE)},
    {"START", TokenInfo(TokenId::START)},
    {"END", TokenInfo(TokenId::END)},
    {"DC", TokenInfo(TokenId::DC, DC_CONST)},
    {"DS", TokenInfo(TokenId::DS, DS_CONST)},
    {"IN", TokenInfo(TokenId::IN, ADR_ADR)},
    {"OUT", TokenInfo(TokenId::OUT, ADR_ADR)},
    {",", TokenInfo(TokenId::COMMA)},
    {";", TokenInfo(TokenId::COMMENT)},
};

const Tokens& Reader::Parse(const std::string& line) {
    tokenes.clear();
    enum STRING_MODE { NONE, START, CONTINUE, END };
    STRING_MODE st_mode = NONE;
    std::string string_s;
    bool check_const_char = false;

    for (std::sregex_iterator it(line.cbegin(), line.cend(), reg), end; it != end; ++it) {
        auto&& m = *it;

        // std::cout << m[1].str() << std::endl;
        if (st_mode == START) {
            // 文字列の最後の'\''まで検索
            if (m[1].str()[0] == '\'') {
                // '\'\''をチェック
                st_mode = CONTINUE;
            } else {
                string_s += m[0].str();
            }
            // std::cout << string_s << std::endl;
            continue;

        } else if (st_mode == CONTINUE) {
            if (m[1].str()[0] == '\'') {
                // '\'\''だった
                st_mode = START;
                string_s += '\'';
                continue;
            } else {
                if (check_const_char && string_s.size() == 1) {
                    tokenes.push_back(TokenInfo(TokenId::CONST, string_s[0], "'" + string_s + "'"));
                } else {
                    tokenes.push_back(TokenInfo(TokenId::STRING, string_s));
                }
                check_const_char = false;
                // TODO:Need??
                st_mode = NONE;
            }
        }
        if (auto itr = key_words.find(m[1].str()); itr != key_words.end()) {
            // std::cout << "OP:" << m[1].str() << std::endl;
            if (itr->second.token_id == TokenId::COMMENT) {
                // コメントはtokenにいれない
                break;
            }
            tokenes.push_back(itr->second);
        } else {
            //      std::cout << "NP:" << m[1].str() << std::endl;
            std::string s = m[1].str();

            if (s[0] == '=') {
                if (s.size() > 1) {
                    std::string x{s.begin() + 1, s.end()};
                    if (IsHex(x)) {
                        std::string hex{x.begin() + 1, x.end()};

                        tokenes.push_back(TokenInfo(TokenId::CONST, std::stoi(hex, nullptr, 16), s));
                    } else if (IsDigit(x)) {
                        tokenes.push_back(TokenInfo(TokenId::CONST, std::stoi(x, nullptr, 10), s));
                    } else {
                        tokenes.push_back(TokenInfo(TokenId::OTHER, s));
                    }
                } else {
                    check_const_char = true;
                }
            } else if (IsHex(s)) {
                std::string hex{s.begin() + 1, s.end()};
                tokenes.push_back(TokenInfo(TokenId::DIGIT, std::stoi(hex, nullptr, 16)));
            } else if (IsDigit(s)) {
                tokenes.push_back(TokenInfo(TokenId::DIGIT, std::stoi(s, nullptr, 10)));
            } else if (s[0] == '\'') {
                st_mode = START;
                string_s = "";
                // std::string str{s.begin() + 1, s.end() - 1};
                // tokenes.push_back(TokenInfo(TokenId::STRING, str));
            } else if (IsLabel(s)) {
                tokenes.push_back(TokenInfo(TokenId::LABEL, s));
            } else {
                tokenes.push_back(TokenInfo(TokenId::OTHER, s));
            }
        }
    }

    if (st_mode == CONTINUE) {
        if (check_const_char && string_s.size() == 1) {
            tokenes.push_back(TokenInfo(TokenId::CONST, string_s[0], "'" + string_s + "'"));
        } else {
            tokenes.push_back(TokenInfo(TokenId::STRING, string_s));
        }
    }
    // TODO:それ以外
    return tokenes;
}

bool Reader::IsDigit(const std::string& s) {
    if (s[0] == '-') {
        return std::all_of(s.cbegin() + 1, s.cend(), isdigit);
    }
    return std::all_of(s.cbegin(), s.cend(), isdigit);
}

bool Reader::IsHex(const std::string& s) {
    return s[0] == '#' && std::all_of(s.cbegin() + 1, s.cend(),
                                      [](char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'); });
}
bool Reader::IsLabel(const std::string& s) {
    if (isalpha(s[0])) {
        if (s.size() > 1) {
            if (std::all_of(s.cbegin() + 1, s.cend(), [](char c) { return isdigit(c) || isalpha(c); })) {
                // TODO:長さはノーチェック
                return true;
            }
        } else {
            return true;
        }
    }
    return false;
}
}  // namespace ass