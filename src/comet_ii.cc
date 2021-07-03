#include "comet_ii.h"

#include <iostream>
#include <string>

namespace cii {
CometII::CometII(Memory *mem, std::ostream &out, std::istream &in) : ram(mem), svc_out(&out), svc_in(&in), counter(0) {
    Reset();
}
CometII::~CometII() {}

void CometII::Reset() {
    GR0 = GR1 = GR2 = GR3 = GR4 = GR5 = GR6 = GR7 = 0;
    SP = ram->size;
    PR = 0;
    FR.Clear();
    counter = 0;
    // break_points.clear();
    pre_pr = -1;
}

/**
 * @fn
 * ここに関数の説明を書く
 * @brief 要約説明
 * @param (引数名) 引数の説明
 * @param (引数名) 引数の説明
 * @return 戻り値の説明
 * @sa 参照すべき関数を書けばリンクが貼れる
 * @detail 詳細な説明
 */
CauseOfStop CometII::Run() {
    CauseOfStop cause = CauseOfStop::OK;

    FR.HLT = OFF;
    for (;;) {
        if (pre_pr != PR) {
            if (auto itr = std::find(break_points.begin(), break_points.end(), PR); itr != break_points.end()) {
                pre_pr = PR;
                cause = CauseOfStop::BREAK_POINT;
                break;
            }
        }
        pre_pr = -1;

        try {
            ExecOneStep();
        } catch (IlleagalAccessError) {
            cause = CauseOfStop::ILLEGAL_ACCESS;
            break;
        } catch (StackOverflowError) {
            cause = CauseOfStop::STACK_OVERFLOW;
            break;
        } catch (StackUnderflowError) {
            cause = CauseOfStop::STACK_UNDERFLOW;
            break;
        } catch (InvalidOperationError) {
            cause = CauseOfStop::INVALID_OPERATION;
            break;
        }
        if (FR.IsHalt()) {
            cause = CauseOfStop::HALT;
            break;
        }
        if (FR.IsSingleStep()) {
            cause = CauseOfStop::SINGLE_STEP;
            FR.SetSingleStep(OFF);
            break;
        }
    }
    return cause;
}

uint16_t CometII::EffectiveAdr(OpWord opword) {
    if (opword.src_reg == 0) {
        return FetchWordData().data;
    }
    return FetchWordData().data + GR[opword.src_reg];
}
/*
 *
 */
void CometII::LoadReg(OpWord opword) {
    GR[opword.des_reg] = GR[opword.src_reg];
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::LoadMem(OpWord opword) {
    GR[opword.des_reg] = FetchWordData(EffectiveAdr(opword));
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::Store(OpWord opword) {
    // store はdes_reg
    StoreData(EffectiveAdr(opword), GR[opword.des_reg]);
}
void CometII::LoadAdr(OpWord opword) { GR[opword.des_reg] = EffectiveAdr(opword); }
/*
 *
 */
void CometII::AddAReg(OpWord opword) { AddA(GR[opword.des_reg], GR[opword.src_reg]); }
void CometII::AddAMem(OpWord opword) { AddA(GR[opword.des_reg], FetchWordData(EffectiveAdr(opword))); }
void CometII::SubAReg(OpWord opword) { SubA(GR[opword.des_reg], GR[opword.src_reg]); }
void CometII::SubAMem(OpWord opword) { SubA(GR[opword.des_reg], FetchWordData(EffectiveAdr(opword))); }

void CometII::AddLReg(OpWord opword) { AddL(GR[opword.des_reg], GR[opword.src_reg]); }
void CometII::AddLMem(OpWord opword) { AddL(GR[opword.des_reg], FetchWordData(EffectiveAdr(opword))); }
void CometII::SubLReg(OpWord opword) { SubL(GR[opword.des_reg], GR[opword.src_reg]); }
void CometII::SubLMem(OpWord opword) { SubL(GR[opword.des_reg], FetchWordData(EffectiveAdr(opword))); }

void CometII::AddA(uint16_t &des, uint16_t src) {
    int32_t result;
    result = signed_cast32(des) + signed_cast32(src);
    des = static_cast<uint16_t>(result & 0xffff);
    FR.SetFlags(result);
}
void CometII::SubA(uint16_t &des, uint16_t src) {
    int32_t result;

    result = signed_cast32(des) - signed_cast32(src);
    des = static_cast<uint16_t>(result & 0xffff);
    FR.SetFlags(result);
}
void CometII::AddL(uint16_t &des, uint16_t src) {
    uint32_t result;

    result = static_cast<uint32_t>(des) + static_cast<uint32_t>(src);
    des = static_cast<uint16_t>(result & 0xffff);
    FR.SetFlags(result);
}
void CometII::SubL(uint16_t &des, uint16_t src) {
    uint32_t result;

    result = static_cast<uint32_t>(des) - static_cast<uint32_t>(src);
    des = static_cast<uint16_t>(result & 0xffff);
    FR.SetFlags(result);
}

void CometII::AndReg(OpWord opword) {
    GR[opword.des_reg] &= GR[opword.src_reg];
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::AndMem(OpWord opword) {
    GR[opword.des_reg] &= FetchWordData(EffectiveAdr(opword));
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::OrReg(OpWord opword) {
    GR[opword.des_reg] |= GR[opword.src_reg];
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::OrMem(OpWord opword) {
    GR[opword.des_reg] |= FetchWordData(EffectiveAdr(opword));
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::XorReg(OpWord opword) {
    GR[opword.des_reg] ^= GR[opword.src_reg];
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::XorMem(OpWord opword) {
    GR[opword.des_reg] ^= FetchWordData(EffectiveAdr(opword));
    FR.SetFlagsClearOver(GR[opword.des_reg]);
}
void CometII::CompAReg(OpWord opword) {
    uint16_t des = GR[opword.des_reg];
    SubA(des, GR[opword.src_reg]);
}
void CometII::CompLReg(OpWord opword) {
    uint16_t des = GR[opword.des_reg];
    SubL(des, GR[opword.src_reg]);
}
void CometII::CompAMem(OpWord opword) {
    uint16_t des = GR[opword.des_reg];
    SubA(des, FetchWordData(EffectiveAdr(opword)));
}
void CometII::CompLMem(OpWord opword) {
    uint16_t des = GR[opword.des_reg];
    SubL(des, FetchWordData(EffectiveAdr(opword)));
}

void CometII::ShiftLeftA(OpWord opword) {
    uint16_t result = GR[opword.des_reg];

    result <<= EffectiveAdr(opword);
    FR.OF = cii::IsSigned(result);

    if (cii::IsSigned(GR[opword.des_reg]) == OFF) {
        result &= 0x7fff;
    } else {
        result |= 0x8000;
    }
    GR[opword.des_reg] = result;
    FR.SetSigned(result);
    FR.SetZero(result);
}

void CometII::ShiftRightA(OpWord opword) {
    int16_t result = (int16_t)GR[opword.des_reg];

    result >>= EffectiveAdr(opword) - 1;

    FR.OF = (result & 1) == 0 ? OFF : ON;

    result >>= 1;

    GR[opword.des_reg] = result;
    FR.SetSigned(result);
    FR.SetZero(result);
}

void CometII::ShiftLeftL(OpWord opword) {
    uint32_t result;

    result = (uint32_t)GR[opword.des_reg] << EffectiveAdr(opword);
    GR[opword.des_reg] = result;
    FR.OF = (result & 0x10000) == 0 ? OFF : ON;
    FR.SetSigned(GR[opword.des_reg]);
    FR.SetZero(GR[opword.des_reg]);
}
void CometII::ShiftRightL(OpWord opword) {
    uint32_t result;

    result = (uint32_t)GR[opword.des_reg] >> (EffectiveAdr(opword) - 1);

    FR.OF = (result & 1) == 0 ? OFF : ON;

    GR[opword.des_reg] = result >> 1;
    FR.SetSigned(GR[opword.des_reg]);
    FR.SetZero(GR[opword.des_reg]);
}
/*
 *
 */
void CometII::JumpOnPlus(OpWord opword) {
    uint16_t jump_adr = EffectiveAdr(opword);
    if (!FR.IsSigned() && !FR.IsZero()) {
        if (jump_adr >= ram->size) throw IlleagalAccessError();

        PR = jump_adr;
    }
}
void CometII::JumpOnMinus(OpWord opword) {
    uint16_t jump_adr = EffectiveAdr(opword);
    if (FR.IsSigned()) {
        if (jump_adr >= ram->size) throw IlleagalAccessError();
        PR = jump_adr;
    }
}
void CometII::JumpOnNonZero(OpWord opword) {
    uint16_t jump_adr = EffectiveAdr(opword);
    if (!FR.IsZero()) {
        if (jump_adr >= ram->size) throw IlleagalAccessError();
        PR = jump_adr;
    }
}
void CometII::JumpOnZero(OpWord opword) {
    uint16_t jump_adr = EffectiveAdr(opword);
    if (FR.IsZero()) {
        if (jump_adr >= ram->size) throw IlleagalAccessError();
        PR = jump_adr;
    }
}
void CometII::JumpOnOverflow(OpWord opword) {
    uint16_t jump_adr = EffectiveAdr(opword);
    if (FR.IsOverflow()) {
        if (jump_adr >= ram->size) throw IlleagalAccessError();
        PR = jump_adr;
    }
}
void CometII::Jump(OpWord opword) {
    uint16_t jump_adr = EffectiveAdr(opword);
    if (jump_adr >= ram->size) throw IlleagalAccessError();
    PR = jump_adr;
}
void CometII::Push(OpWord opword) { StoreData(--SP, EffectiveAdr(opword)); }
void CometII::Pop(OpWord opword) { GR[opword.des_reg] = FetchWordData(SP++); }
void CometII::CallSub(OpWord opword) {
    if (SP == 0) throw StackOverflowError();

    SP--;
    uint16_t call_addr = EffectiveAdr(opword);
    StoreData(SP, PR);
    PR = call_addr;
}
void CometII::ReturnFromSub(OpWord opword) {
    if (SP >= ram->size) throw StackUnderflowError();

    PR = FetchWordData(SP);
    SP++;
}
void CometII::Svc(OpWord opword) {
    SVCNo svc_no = static_cast<SVCNo>(EffectiveAdr(opword));
    switch (svc_no) {
    case SVCNo::SVC_IN:
        SvcIn(opword);
        break;

    case SVCNo::SVC_OUT:
        SvcOut(opword);
        break;
    }
}

void CometII::SvcIn(OpWord opword) {
    std::string line;

    bool is_ok = (bool)std::getline(*svc_in, line);

    if (is_ok) {
        StoreData(GR2, (uint16_t)line.size());

        int i = 0;
        for (auto c : line) {
            StoreData(GR1 + i++, c);
        }
    } else {
        StoreData(GR2, -1);
    }
}

void CometII::SvcOut(OpWord opword) {
    uint16_t len = FetchWordData(GR2);

    for (uint16_t i = 0; i < len; i++) {
        int16_t data = FetchWordData(GR1 + i);
        *svc_out << static_cast<int8_t>(data);
    }
    *svc_out << std::endl;  // TODO
}

void CometII::ExecOneStep() {
    counter++;
    WordData word_data = FetchWordData();
    switch (word_data.opword.GetOpCode()) {
    case OpCode::LD_M:
        LoadMem(word_data.opword);
        break;
    case OpCode::ST:
        Store(word_data.opword);
        break;
    case OpCode::LAD:
        LoadAdr(word_data.opword);
        break;
    case OpCode::LD_R:
        LoadReg(word_data.opword);
        break;
    case OpCode::ADDA_R:
        AddAReg(word_data.opword);
        break;
    case OpCode::ADDA_M:
        AddAMem(word_data.opword);
        break;
    case OpCode::SUBA_R:
        SubAReg(word_data.opword);
        break;
    case OpCode::SUBA_M:
        SubAMem(word_data.opword);
        break;
    case OpCode::ADDL_R:
        AddLReg(word_data.opword);
        break;
    case OpCode::ADDL_M:
        AddLMem(word_data.opword);
        break;
    case OpCode::SUBL_R:
        SubLReg(word_data.opword);
        break;
    case OpCode::SUBL_M:
        SubLMem(word_data.opword);
        break;
    case OpCode::AND_R:
        AndReg(word_data.opword);
        break;
    case OpCode::AND_M:
        AndMem(word_data.opword);
        break;
    case OpCode::OR_R:
        OrReg(word_data.opword);
        break;
    case OpCode::OR_M:
        OrMem(word_data.opword);
        break;
    case OpCode::XOR_R:
        XorReg(word_data.opword);
        break;
    case OpCode::XOR_M:
        XorMem(word_data.opword);
        break;
    case OpCode::CPA_R:
        CompAReg(word_data.opword);
        break;
    case OpCode::CPL_R:
        CompLReg(word_data.opword);
        break;
    case OpCode::CPA_M:
        CompAMem(word_data.opword);
        break;
    case OpCode::CPL_M:
        CompLMem(word_data.opword);
        break;
    case OpCode::SLA:
        ShiftLeftA(word_data.opword);
        break;
    case OpCode::SRA:
        ShiftRightA(word_data.opword);
        break;
    case OpCode::SLL:
        ShiftLeftL(word_data.opword);
        break;
    case OpCode::SRL:
        ShiftRightL(word_data.opword);
        break;
    case OpCode::JPL:
        JumpOnPlus(word_data.opword);
        break;
    case OpCode::JMI:
        JumpOnMinus(word_data.opword);
        break;
    case OpCode::JNZ:
        JumpOnNonZero(word_data.opword);
        break;
    case OpCode::JZE:
        JumpOnZero(word_data.opword);
        break;
    case OpCode::JOV:
        JumpOnOverflow(word_data.opword);
        break;
    case OpCode::JUMP:
        Jump(word_data.opword);
        break;
    case OpCode::PUSH:
        Push(word_data.opword);
        break;
    case OpCode::POP:
        Pop(word_data.opword);
        break;
    case OpCode::CALL:
        CallSub(word_data.opword);
        break;
    case OpCode::RET:
        ReturnFromSub(word_data.opword);
        break;
    case OpCode::SVC:
        Svc(word_data.opword);
        break;
    case OpCode::HLT:
        FR.HLT = ON;
        break;
    default:
        throw InvalidOperationError();
        // FR.HLT = ON;
        break;
    }
}

}  // namespace cii
