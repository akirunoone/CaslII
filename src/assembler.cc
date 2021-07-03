#include "assembler.h"

#include <iostream>
#include <map>
#include <regex>
#include <vector>

#include "assem_mem.h"
#include "comet_ii.h"
#include "reader.h"

namespace ass {

std::map<TokenId, OpInfo> op_table = {
    {TokenId::ST, OpInfo(REG_MEM, cii::OpCode::ST, cii::OpCode::ST)},
    {TokenId::LD, OpInfo(REG_REGorMEM, cii::OpCode::LD_R, cii::OpCode::LD_M)},
    {TokenId::LAD, OpInfo(REG_EADR, cii::OpCode::LAD)},
    {TokenId::ADDA, OpInfo(REG_REGorMEM, cii::OpCode::ADDA_R, cii::OpCode::ADDA_M)},
    {TokenId::ADDL, OpInfo(REG_REGorMEM, cii::OpCode::ADDL_R, cii::OpCode::ADDL_M)},
    {TokenId::SUBA, OpInfo(REG_REGorMEM, cii::OpCode::SUBA_R, cii::OpCode::SUBA_M)},
    {TokenId::SUBL, OpInfo(REG_REGorMEM, cii::OpCode::SUBL_R, cii::OpCode::SUBL_M)},
    {TokenId::AND, OpInfo(REG_REGorMEM, cii::OpCode::AND_R, cii::OpCode::AND_M)},
    {TokenId::OR, OpInfo(REG_REGorMEM, cii::OpCode::OR_R, cii::OpCode::OR_M)},
    {TokenId::XOR, OpInfo(REG_REGorMEM, cii::OpCode::XOR_R, cii::OpCode::XOR_M)},
    {TokenId::CPA, OpInfo(REG_REGorMEM, cii::OpCode::CPA_R, cii::OpCode::CPA_M)},
    {TokenId::CPL, OpInfo(REG_REGorMEM, cii::OpCode::CPL_R, cii::OpCode::CPL_M)},
    {TokenId::SLA, OpInfo(REG_EADR, cii::OpCode::SLA)},
    {TokenId::SRA, OpInfo(REG_EADR, cii::OpCode::SRA)},
    {TokenId::SLL, OpInfo(REG_EADR, cii::OpCode::SLL)},
    {TokenId::SRL, OpInfo(REG_EADR, cii::OpCode::SRL)},
    {TokenId::JPL, OpInfo(EADR, cii::OpCode::JPL)},
    {TokenId::JMI, OpInfo(EADR, cii::OpCode::JMI)},
    {TokenId::JNZ, OpInfo(EADR, cii::OpCode::JNZ)},
    {TokenId::JZE, OpInfo(EADR, cii::OpCode::JZE)},
    {TokenId::JOV, OpInfo(EADR, cii::OpCode::JOV)},
    {TokenId::JUMP, OpInfo(EADR, cii::OpCode::JUMP)},
    {TokenId::PUSH, OpInfo(EADR, cii::OpCode::PUSH)},
    {TokenId::POP, OpInfo(REG, cii::OpCode::POP)},
    {TokenId::CALL, OpInfo(EADR, cii::OpCode::CALL)},
    {TokenId::RET, OpInfo(NONE, cii::OpCode::RET)},
    {TokenId::IN, OpInfo(ADR_ADR, cii::OpCode::SVC)},
    {TokenId::OUT, OpInfo(ADR_ADR, cii::OpCode::SVC)},
    {TokenId::NOP, OpInfo(NONE, cii::OpCode::NOP)},
    {TokenId::HLT, OpInfo(NONE, cii::OpCode::HLT)},
};

Assembler::Assembler() {}

void Assembler::Assemble(std::string line, cii::AssmMem& asem) {
    error = AsmErrCode::OK;

    auto tokens = reader.Parse(line);
    CheckSyntax(tokens);

    if (error != AsmErrCode::OK) is_error = true;

    uint16_t start_offset = asem.GetOffset();

    auto tokens2 = tokens;
    Assemble(tokens, asem);
    DbgInfo dbg_info{line, error, false, start_offset, asem.GetOffset(), std::move(tokens2)};

    dbg_infos.push_back(dbg_info);
}

void Assembler::Assemble(std::stringstream& ss, cii::AssmMem& asem) {
    std::string line;

    is_error = false;
    dbg_infos.clear();

    while (std::getline(ss, line)) {
        Assemble(line, asem);
    }
}

void Assembler::Assemble(std::ifstream& ss, cii::AssmMem& asem) {
    std::string line;

    is_error = false;
    dbg_infos.clear();

    while (std::getline(ss, line)) {
        Assemble(line, asem);
    }
}

void Assembler::Assemble(Tokens& tokens, cii::AssmMem& mem) {
    if (tokens.size() == 0) return;

    if (tokens[0].token_id == TokenId::LABEL) {
        // 後のチェックを簡単にするために、最初にチェックし、ラベルのときは消去する

        if (!mem.CheckSym(tokens[0].label)) {
            error = AsmErrCode::MULTI_DEF_SYM;
            return;
        }
        if (tokens.size() > 1 && tokens[1].token_id == TokenId::START)
            mem << cii::SymStart(tokens[0].label.c_str());
        else
            mem << cii::SymDef(tokens[0].label.c_str());
        tokens.erase(tokens.begin());
    }

    if (tokens.size() == 0) return;

    TokenClass tc = GetTokenClass(tokens[0].token_id);

    switch (tc) {
    case OPE_CLASS:
        AssembleOpe(tokens, mem);
        break;
    case ASEM_CLASS:
        AssembleMacro(tokens, mem);
        break;
    case MACRO_CLASS:
        AssembleOpe(tokens, mem);
        break;
    case REG_CLASS:
    case COMMENT_CLASS:
        // ErrorCheckFirst(tokens);
        break;
    default:
        break;
    }
}

void Assembler::AssembleOpe(const Tokens& tokens, cii::AssmMem& mem) {
    auto itr = op_table.find(tokens[0].token_id);
    if (itr == op_table.end()) {
        return;
    }

    OpInfo op_info = itr->second;
    switch (op_info.operand_type) {
    case REG_EADR:
        AssembleRegEadr(op_info, tokens, mem);
        break;
    case REG_MEM:
        AssembleRegMem(op_info, tokens, mem);
        break;
    case REG_REGorMEM:
        if (CheckRegReg(tokens)) {
            AssembleRegReg(op_info, tokens, mem);
        } else {
            AssembleRegMem(op_info, tokens, mem);
        }
        break;
    case EADR:
        AssembleEadr(op_info, tokens, mem);
        break;
    case ADR_ADR:
        AssembleAdrAdr(op_info, tokens, mem);
        break;
    case REG:
        AssembleReg(op_info, tokens, mem);
        break;
    case NONE:
        AssembleNone(op_info, tokens, mem);
        break;
    default:
        break;
    }
}

bool Assembler::CheckRegReg(const Tokens& tokens) {
    if (tokens.size() < 4) return false;

    return (GetTokenClass(tokens[1].token_id) == TokenClass::REG_CLASS) && (tokens[2].token_id == TokenId::COMMA) &&
           (GetTokenClass(tokens[3].token_id) == TokenClass::REG_CLASS);
}

void Assembler::AssembleRegReg(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {
    mem << cii::OpWord(op_info.opcode1, GetRegNo(tokens[1].token_id), GetRegNo(tokens[3].token_id));
}
void Assembler::AssembleReg(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {
    mem << cii::OpWord(op_info.opcode1, GetRegNo(tokens[1].token_id));
}
void Assembler::AssembleNone(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {
    mem << cii::OpWord(op_info.opcode1);
}

void Assembler::AssembleEadr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {
    cii::Reg src_reg = cii::Reg::GR0;
    if (tokens.size() >= 4) {
        src_reg = GetRegNo(tokens[3].token_id);
    }
    mem << cii::OpWord(op_info.opcode1, src_reg);
    if (tokens[1].token_id == TokenId::DIGIT) {
        mem << tokens[1].digit;
    } else {
        mem << cii::SymRef(tokens[1].label.c_str());
    }
}
void Assembler::AssembleAdrAdr(OpInfo, const Tokens& tokens, cii::AssmMem& mem) {
    Tokens* ts = const_cast<Tokens*>(&tokens);

    if (tokens[0].token_id == TokenId::OUT) {
        mem << cii::IOSVC(cii::SVCNo::SVC_OUT, tokens[1].label, tokens[3].label);
    } else if (tokens[0].token_id == TokenId::IN) {
        mem << cii::IOSVC(cii::SVCNo::SVC_IN, tokens[1].label, tokens[3].label);
    }
}
void Assembler::AssembleRegEadr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {
    cii::Reg src_reg = cii::Reg::GR0;
    if (tokens.size() >= 6) {
        src_reg = GetRegNo(tokens[5].token_id);
    }
    mem << cii::OpWord(op_info.opcode1, GetRegNo(tokens[1].token_id), src_reg);
    if (tokens[3].token_id == TokenId::DIGIT) {
        mem << tokens[3].digit;
    } else {
        mem << cii::SymRef(tokens[3].label.c_str());
    }
}

void Assembler::AssembleRegMem(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {
    cii::Reg src_reg = cii::Reg::GR0;
    if (tokens.size() >= 6) {
        src_reg = GetRegNo(tokens[5].token_id);
    }
    mem << cii::OpWord(op_info.opcode2, GetRegNo(tokens[1].token_id), src_reg);
    if (tokens[3].token_id == TokenId::DIGIT) {
        mem << tokens[3].digit;
    } else {
        if (tokens[3].token_id == TokenId::CONST) {
            mem << cii::SymConst(tokens[3].label.c_str(), tokens[3].digit);
        } else if (tokens[3].token_id == TokenId::LABEL) {
            mem << cii::SymRef(tokens[3].label.c_str());
        }
    }
}

void Assembler::AssembleRegRegOrEadr(OpInfo op_info, const Tokens& tokens, cii::AssmMem& mem) {}

void Assembler::CheckSyntax(const Tokens& tokens) {
    int pos = 0;

    // コメントはtokenに入っていない

    if (tokens.size() == 0) return;

    if (IsLabel(tokens[pos])) {
        pos++;
        if (tokens.size() == pos) return;
    }

    TokenClass tc = GetTokenClass(tokens[pos].token_id);
    if (!((tc == OPE_CLASS || tc == ASEM_CLASS || tc == MACRO_CLASS))) {
        error = AsmErrCode::NO_OPERATION;
        return;
    }

    OperandType type = tokens[pos].operand_type;

    int tk_num = (int)tokens.size() - pos;

    if (type == NONE) {
        if (tk_num == 1) return;
        error = AsmErrCode::INVALID_OPERAND;
        return;
    }

    if (tk_num == 1) {
        error = AsmErrCode::NO_OPERAND;
        return;
    }

    if (type == REG) {
        // op r
        if (tk_num == 2 && IsReg(tokens[pos + 1])) return;

    } else if (type == EADR) {
        // op adr
        if (tk_num == 2 && IsAdr(tokens[pos + 1])) return;
        // op adr,x
        if (tk_num == 4 && IsAdr(tokens[pos + 1]) && IsComma(tokens[pos + 2]) && IsReg(tokens[pos + 3])) return;

    } else if (type == REG_EADR || type == REG_REGorMEM) {
        if (type == REG_REGorMEM) {
            // op r1,r2
            if (tk_num == 4 && IsReg(tokens[pos + 1]) && IsComma(tokens[pos + 2]) && IsReg(tokens[pos + 3])) return;
        }
        // op r,adr
        if (tk_num == 4 && IsReg(tokens[pos + 1]) && IsComma(tokens[pos + 2]) && IsAdr(tokens[pos + 3])) return;
        // op r,adr,x
        if (tk_num == 6 && IsReg(tokens[pos + 1]) && IsComma(tokens[pos + 2]) && IsAdr(tokens[pos + 3]) &&
            IsComma(tokens[pos + 4]) && IsReg(tokens[pos + 5]))
            return;
    } else if (type == DC_CONST) {
        // DC 定数[,定数 ...]
        bool error = false;
        int end_pos = tk_num + pos;
        for (int index = pos + 1; index < end_pos; index += 2) {
            if (IsDCConst(tokens[index])) {
                if ((index + 1) < end_pos) {
                    if (!IsComma(tokens[index + 1])) {
                        error = true;
                        break;
                    }
                }
            } else {
                error = true;
                break;
            }
        }
        if (!error) return;
    } else if (type == DS_CONST) {
        // DS 語数
        if (tk_num == 2 && IsDigit(tokens[pos + 1]) && tokens[pos + 1].digit >= 0) return;
    } else if (type == ADR_ADR) {
        if (tk_num == 4 && IsAdr(tokens[pos + 1]) && IsComma(tokens[pos + 2]) && IsAdr(tokens[pos + 3])) return;
    }
    error = AsmErrCode::INVALID_OPERAND;
    return;
}

void Assembler::AssembleMacro(const Tokens& tokens, cii::AssmMem& mem) {
    int ix = 0;
    // LABELは削除されているのでいらない
    // if (tokens[ix].token_id == TokenId::LABEL) {
    //     mem << cii::SymDef(tokens[0].label.c_str());
    //     // std::cout << tokens[0].label.c_str() << std::endl;
    //     ix++;
    // }
    // if (tokens.size() <= ix) {
    //     return;
    // }
    if (tokens[ix].token_id == TokenId::DS) {
        mem << cii::DsDef(tokens[ix + 1].digit);
    } else if (tokens[ix].token_id == TokenId::DC) {
        ++ix;
        if (tokens[ix].token_id == TokenId::STRING) {
            mem << tokens[ix].label;
        } else {
            mem << cii::DcDef(tokens[ix].digit);
            ix++;
            for (; (ix + 1) < tokens.size();) {
                if (tokens[ix].token_id == TokenId::COMMA && tokens[ix + 1].token_id == TokenId::DIGIT) {
                    mem << tokens[ix + 1].digit;
                }
                ix += 2;
            }
        }
    }
}

}  // namespace ass
