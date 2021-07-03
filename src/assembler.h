#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <fstream>
#include <iostream>
#include <sstream>

#include "assem_mem.h"
#include "comet_ii.h"
#include "reader.h"

namespace ass {

enum class AsmErrCode {
    OK,
    NO_OPERATION,     //<! 命令コードがない
    NO_OPERAND,       //<! オペランドがない
    INVALID_OPERAND,  //<! 余計なオペランド
    NO_DC_OR_DS,      //<! 定数、文字列が指定されたがDC,DSが宣言されていない
    NO_DEF_SYM,       //<! シンボルの定義が見つからない
    MULTI_DEF_SYM,    //<! シンボル多重定義
    ERR,              //!< その他のエラー
};

struct OpInfo {
    OperandType operand_type;
    cii::OpCode opcode1;
    cii::OpCode opcode2;
    OpInfo(OperandType ot, cii::OpCode op1 = cii::OpCode::NOP, cii::OpCode op2 = cii::OpCode::NOP)
        : operand_type(ot), opcode1(op1), opcode2(op2) {}
};

struct DbgInfo {
    std::string line;
    AsmErrCode err;
    bool is_break;
    uint16_t start_offset;
    uint16_t end_offset;
    ass::Tokens tokens;
};

using DbgInfos = std::vector<DbgInfo>;

/**
 * アセンブラクラス
 */
class Assembler {
    Reader reader;

   public:
    AsmErrCode error = AsmErrCode::OK;
    int number = 0;
    DbgInfos dbg_infos;
    bool is_error = false;

    Assembler();

    void Assemble(std::string line, cii::AssmMem& asem);
    void Assemble(std::stringstream& ss, cii::AssmMem& asem);
    void Assemble(std::ifstream& ss, cii::AssmMem& asem);

   private:
    void Assemble(Tokens& tokens, cii::AssmMem& mem);
    void AssembleOpe(const Tokens& tokens, cii::AssmMem& mem);
    void AssembleRegEadr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleEadr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleAdrAdr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleNone(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleReg(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleRegRegOrEadr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleRegMem(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleRegReg(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem);
    void AssembleMacro(const Tokens& tokens, cii::AssmMem& mem);
    void CheckSyntax(const Tokens& tokens);
    bool CheckRegReg(const Tokens& tokens);
    /**
     * TokeinId
     */
    inline cii::Reg GetRegNo(const TokenId tid) {
        uint8_t reg_no = (uint16_t)tid & (~(uint16_t)TOKEN_CLASS_MASK);
        return (cii::Reg)reg_no;
    }
    bool IsComment(const TokenInfo& token) const { return token.token_id == TokenId::COMMENT; }
    bool IsReg(const TokenInfo& token) const { return GetTokenClass(token.token_id) == REG_CLASS; }
    bool IsComma(const TokenInfo& token) const { return token.token_id == TokenId::COMMA; }
    bool IsLabel(const TokenInfo& token) const { return token.token_id == TokenId::LABEL; }
    bool IsDigit(const TokenInfo& token) const { return token.token_id == TokenId::DIGIT; }
    bool IsConst(const TokenInfo& token) const { return token.token_id == TokenId::CONST; }
    bool IsAdr(const TokenInfo& token) const { return IsLabel(token) || IsDigit(token) || IsConst(token); }
    bool IsString(const TokenInfo& token) const { return token.token_id == TokenId::STRING; }
    bool IsDCConst(const TokenInfo& token) const { return IsDigit(token) || IsString(token); }
};

}  // namespace ass
#endif