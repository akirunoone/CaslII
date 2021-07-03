
#include "debugger.h"

#include <algorithm>
#include <cctype>
#include <locale>

#include "common.h"
#include "reader.h"

namespace cii {

/**
 * 出力文字の色設定
 */
const ColorChar C_PROMPT(cmn::Color::F_BRIGHT_BLUE, "$ ");
const ColorChar C_EC(cmn::Color::F_YELLOW);
const ColorChar C_REG(cmn::Color::F_GREEN);
const ColorChar C_REG_DIFF(cmn::Color::F_BRIGHT_MAGENTA);
const ColorChar C_BREAK(cmn::Color::F_RED, "* ");
const ColorChar C_ADDR(cmn::Color::F_BRIGHT_BLUE);
const ColorChar C_LIST(cmn::Color::F_BRIGHT_GREEN);
const ColorChar C_ERROR(cmn::Color::F_BRIGHT_RED);
//
const ColorChar C_EXEC_ADR(cmn::Color::B_YELLOW);

const ColorChar C_OP(cmn::Color::F_GREEN);
const ColorChar C_REGSTER(cmn::Color::F_BRIGHT_BLUE);
const ColorChar C_ASMOP(cmn::Color::F_BRIGHT_BLUE);
const ColorChar C_LABEL(cmn::Color::F_MAGENTA);
const ColorChar C_RESET(cmn::Color::RESET);
//
const ColorChar C_HELP_CMD(cmn::Color::F_YELLOW);
const ColorChar C_HELP_DES(cmn::Color::F_GREEN);
const ColorChar C_MACRO(cmn::Color::F_YELLOW);

/**
 * レジスタ値の保存
 */
struct SaveRegs {
    uint16_t PR, SP, GR0, GR1, GR2, GR3, GR4, GR5, GR6, GR7;
    bool FR_OF, FR_ZF, FR_SF;
    uint32_t executed_counter;
};

SaveRegs save_regs{};

static const CmdDef cmds[] = {
    {"R", "全レジスタ表示", "R", CmdId::SHOW_REG_ALL, CmdParam::NO_PARAM},
    {"L", "ソースリスト表示。offsetの指定がないときは、PRレジスタが指す位置から最後まで表示",
     "L [start offset] [end offset]", CmdId::LIST_SRC, CmdParam::OPT_NUM1},
    {"S", "シングルステップ", "S", CmdId::SINGLE_STEP, CmdParam::NO_PARAM},
    {"C", "現在状態からの実行", "C", CmdId::CONTINUE, CmdParam::NO_PARAM},
    {"BP", "ブレークポイントの設定", "BP [offset1] [offset2] ...", CmdId::BREAK_POINT, CmdParam::NUM1},
    {"BC", "全ブレークポイントのクリアまたは指定ブレークポイントのクリア", "BC * | offset1 [offset2] ...",
     CmdId::CLEAR_BREAK_POINTS, CmdParam::NUM1},
    {"GO", "レジスタをリセットして実行", "GO", CmdId::RUN, CmdParam::NO_PARAM},
    {"RESET", "レジスタをリセット", "RESET", CmdId::RESET, CmdParam::NO_PARAM},
    {"GR0", "GR0の表示", "GR0", CmdId::SHOW_REG_GR0, CmdParam::NO_PARAM},
    {"GR1", "GR1の表示", "GR1", CmdId::SHOW_REG_GR1, CmdParam::NO_PARAM},
    {"GR2", "GR2の表示", "GR2", CmdId::SHOW_REG_GR2, CmdParam::NO_PARAM},
    {"GR3", "GR3の表示", "GR3", CmdId::SHOW_REG_GR3, CmdParam::NO_PARAM},
    {"GR4", "GR4の表示", "GR4", CmdId::SHOW_REG_GR4, CmdParam::NO_PARAM},
    {"GR5", "GR5の表示", "GR5", CmdId::SHOW_REG_GR5, CmdParam::NO_PARAM},
    {"GR6", "GR6の表示", "GR6", CmdId::SHOW_REG_GR6, CmdParam::NO_PARAM},
    {"GR7", "GR7の表示", "GR7", CmdId::SHOW_REG_GR7, CmdParam::NO_PARAM},
    {"H", "ヘルプ", "H", CmdId::HELP, CmdParam::NO_PARAM},
    {"Q", "終了", "Q", CmdId::QUIT, CmdParam::NO_PARAM},

#if 0
    {"R", "Print All Registers", "R", CmdId::SHOW_REG_ALL, CmdParam::NO_PARAM},
    {"L", "List Sources. Default start offset is PR reg.", "L [start offset] [end offset]", CmdId::LIST_SRC,
     CmdParam::OPT_NUM1},
    {"S", "Single Step", "S", CmdId::SINGLE_STEP, CmdParam::NO_PARAM},
    {"C", "Continue", "C", CmdId::CONTINUE, CmdParam::NO_PARAM},
    {"BP", "Set Break Points", "BP [offset1|label1] [offset2|label2] ... [pointN|labelN]", CmdId::BREAK_POINT,
     CmdParam::NUM1},
    {"BC", "Break Points All Or offset ... Clear", "BC * | offset1|label  [offset2|label2] ... [offsetN|labelN]",
     CmdId::CLEAR_BREAK_POINTS, CmdParam::NUM1},
    {"GO", "Reset CommetII And Run", "GO", CmdId::RUN, CmdParam::NO_PARAM},
    {"RESET", "Reset CommetII", "RESET", CmdId::RESET, CmdParam::NO_PARAM},
    {"GR0", "Print GR0", "GR0", CmdId::SHOW_REG_GR0, CmdParam::NO_PARAM},
    {"GR1", "Print GR1", "GR1", CmdId::SHOW_REG_GR1, CmdParam::NO_PARAM},
    {"GR2", "Print GR2", "GR2", CmdId::SHOW_REG_GR2, CmdParam::NO_PARAM},
    {"GR3", "Print GR3", "GR3", CmdId::SHOW_REG_GR3, CmdParam::NO_PARAM},
    {"GR4", "Print GR4", "GR4", CmdId::SHOW_REG_GR4, CmdParam::NO_PARAM},
    {"GR5", "Print GR5", "GR5", CmdId::SHOW_REG_GR5, CmdParam::NO_PARAM},
    {"GR6", "Print GR6", "GR6", CmdId::SHOW_REG_GR6, CmdParam::NO_PARAM},
    {"GR7", "Print GR7", "GR7", CmdId::SHOW_REG_GR7, CmdParam::NO_PARAM},
    {"H", "Help Commands", "H", CmdId::HELP, CmdParam::NO_PARAM},
    {"Q", "Quit", "Q", CmdId::QUIT, CmdParam::NO_PARAM},
#endif
};

bool Debugger::ParseCmd(std::string& cmd_string, CmdDef& cmd_def, std::vector<std::string>& params) {
    std::locale l = std::locale::classic();
    cmd_def.cmd_id = CmdId::NONE;
    std::stringstream ss{cmd_string};
    std::string param;
    while (std::getline(ss, param, ' ')) {
        // toupper
        std::transform(param.begin(), param.end(), param.begin(), [&](char c) { return std::toupper(c, l); });
        if (param.size() > 0) params.push_back(param);
    }

    if (params.size() > 0) {
        if (auto itr = std::find_if(std::begin(cmds), std::end(cmds),
                                    [&](CmdDef cmd) { return params.at(0) == cmd.cmd_name; });
            itr != std::end(cmds)) {
            cmd_def = *itr;
            params.erase(params.begin());
            if (itr->cmd_param == CmdParam::NO_PARAM && params.size() > 0) {
                cmn::C << itr->cmd_fullname << ": パラメタは指定できません。\n";
                return false;
            }
            return true;
        }
        cmn::C << cmn::Format("コマンド\"%s\"はありません。\n", params[0].c_str());
    }
    return false;
}

void Debugger::DisplayHelp() {
    cmn::C << C_HELP_DES << " Debugger Commands:\n";
    for (auto cmd_def : cmds) {
        cmn::C << C_HELP_CMD << cmn::Format("   %s : ", cmd_def.cmd_description) << C_HELP_DES << cmd_def.cmd_fullname
               << C_RESET << std::endl;
    }
    cmn::C << C_HELP_DES << "    ※ Offset は10進数、16進数またはラベルで指定\n"
           << "    　例) BP 123 #12AB LABEL" << C_RESET << std::endl;
}

void Debugger::Start() {
    DisplayRegs();

    std::string key;
    std::string prev_key;
    cmn::C << C_PROMPT;
    CmdDef cmd{};
    std::vector<std::string> params;
    while (std::getline(std::cin, key)) {
        bool quit = false;
        if (key.size() == 0 && prev_key.size() > 0) {
            cmn::C << C_PROMPT << prev_key << std::endl;
        } else {
            params.clear();
            prev_key = key;
        }
        if (key.size() == 0 || ParseCmd(key, cmd, params)) {
            switch (cmd.cmd_id) {
            case CmdId::BREAK_POINT:
                SetBreakPoint(params);
                break;
            case CmdId::CLEAR_BREAK_POINTS:
                ClearBreakPoints(params);
                break;
            case CmdId::LIST_SRC:
                DisplaySrc(params);
                break; /*  */
            case CmdId::SHOW_REG_ALL:
                DisplayRegs();
                break;
            case CmdId::SINGLE_STEP:
                SingleStep();
                break;
            case CmdId::RUN:
                cii_cpu.Reset();
                Run();
                break;
            case CmdId::RESET:
                cii_cpu.Reset();
                SaveRegs();
                DisplayRegs();
                break;
            case CmdId::CONTINUE:
                Run();
                break;
            case CmdId::QUIT:
                quit = true;
                break;
            case CmdId::SHOW_REG_GR0:
            case CmdId::SHOW_REG_GR1:
            case CmdId::SHOW_REG_GR2:
            case CmdId::SHOW_REG_GR3:
            case CmdId::SHOW_REG_GR4:
            case CmdId::SHOW_REG_GR5:
            case CmdId::SHOW_REG_GR6:
            case CmdId::SHOW_REG_GR7:
                DisplayReg(cmd.cmd_id);
                break;
            case CmdId::HELP:
                DisplayHelp();
                break;
            default:
                break;
            }
        }
        if (quit) break;
        cmn::C << C_PROMPT;
    }
}

void Debugger::SingleStep() {
    SetSingleStep();
    Run();
}

void Debugger::SaveRegs() {
    save_regs.PR = cii_cpu.PR;
    save_regs.SP = cii_cpu.SP;
    save_regs.GR0 = cii_cpu.GR0;
    save_regs.GR1 = cii_cpu.GR1;
    save_regs.GR2 = cii_cpu.GR2;
    save_regs.GR3 = cii_cpu.GR3;
    save_regs.GR4 = cii_cpu.GR4;
    save_regs.GR5 = cii_cpu.GR5;
    save_regs.GR6 = cii_cpu.GR6;
    save_regs.GR7 = cii_cpu.GR7;
    save_regs.FR_OF = cii_cpu.FR.IsOverflow();
    save_regs.FR_ZF = cii_cpu.FR.IsZero();
    save_regs.FR_SF = cii_cpu.FR.IsSigned();
    save_regs.executed_counter = cii_cpu.GetExcutedCounter();
}

void Debugger::Run() {
    SaveRegs();
    cii::CauseOfStop status = cii_cpu.Run();
    if (status != cii::CauseOfStop::OK) {
        if (status == cii::CauseOfStop::STACK_UNDERFLOW) {
            cmn::C << C_ERROR << "* STACK UNDERFLOW" << C_RESET << std::endl;
        } else if (status == cii::CauseOfStop::STACK_OVERFLOW) {
            cmn::C << C_ERROR << "* STACK UNDERFLOW" << C_RESET << std::endl;
        } else if (status == cii::CauseOfStop::HALT) {
            cmn::C << C_ERROR << "* HALT" << C_RESET << std::endl;
        } else if (status == cii::CauseOfStop::ILLEGAL_ACCESS) {
            cmn::C << C_ERROR << "* ILLEAGAL ACCESS" << C_RESET << std::endl;
        } else if (status == cii::CauseOfStop::INVALID_OPERATION) {
            cmn::C << C_ERROR << "* INVALID OPERATION" << C_RESET << std::endl;
        } else if (status == cii::CauseOfStop::SINGLE_STEP) {
            cmn::C << C_ERROR << "* SINGLE STEP" << C_RESET << std::endl;
        } else if (status == cii::CauseOfStop::BREAK_POINT) {
            cmn::C << C_ERROR << "* BREAK POINT" << C_RESET << std::endl;
        } else {
            cmn::C << C_ERROR << "* OTHER ERROR" << C_RESET << std::endl;
        }
    }
    DisplayRegs();
    cmn::C << std::endl;

    if (auto itr = std::find_if(dbg_infos.rbegin(), dbg_infos.rend(),
                                [&](ass::DbgInfo dbg_info) {
                                    return (dbg_info.start_offset <= cii_cpu.PR && dbg_info.end_offset > cii_cpu.PR) &&
                                           (dbg_info.end_offset - dbg_info.start_offset) > 0;
                                    // return dbg_info.start_offset == cii_cpu.PR &&
                                    //        (dbg_info.end_offset - dbg_info.start_offset) > 0;
                                });
        itr != dbg_infos.rend()) {
        DisplaySrc(itr->start_offset, itr->end_offset + 1, true);
    }
}

void Debugger::DisplayRegs() const {
    cmn::C << C_EC << "EC = " << cii_cpu.GetExcutedCounter() << C_RESET << std::endl;

    DisplayOneReg("PR", cii_cpu.PR, save_regs.PR);
    cmn::C << ", ";
    DisplayOneReg("SR", cii_cpu.SP, save_regs.SP);
    cmn::C << ", ";
    DisplayOneReg("OF", cii_cpu.FR.IsOverflow(), save_regs.FR_OF);
    cmn::C << ", ";
    DisplayOneReg("ZF", cii_cpu.FR.IsZero(), save_regs.FR_ZF);
    cmn::C << ", ";
    DisplayOneReg("SF", cii_cpu.FR.IsSigned(), save_regs.FR_SF);
    cmn::C << "\n";

    DisplayOneReg("GR0", cii_cpu.GR0, save_regs.GR0);
    cmn::C << ", ";
    DisplayOneReg("GR1", cii_cpu.GR1, save_regs.GR1);
    cmn::C << ", ";
    DisplayOneReg("GR2", cii_cpu.GR2, save_regs.GR2);
    cmn::C << ", ";
    DisplayOneReg("GR3", cii_cpu.GR3, save_regs.GR3);
    cmn::C << "\n";

    DisplayOneReg("GR4", cii_cpu.GR4, save_regs.GR4);
    cmn::C << ", ";
    DisplayOneReg("GR5", cii_cpu.GR5, save_regs.GR5);
    cmn::C << ", ";
    DisplayOneReg("GR6", cii_cpu.GR6, save_regs.GR6);
    cmn::C << ", ";
    DisplayOneReg("GR7", cii_cpu.GR7, save_regs.GR7);
    cmn::C << "\n";
}

void Debugger::DisplayReg(CmdId cmd_id) const {
    int reg_no = (int)cmd_id - (int)CmdId::SHOW_REG_GR0;

    cmn::C << C_REG << cmn::Format("GR%d = %04x(%d)\n", reg_no, cii_cpu.GetReg(reg_no), cii_cpu.GetReg(reg_no));
}

int Debugger::DisplayLine(const ass::DbgInfo dbg_info) const {
    const ass::Tokens& tokens = dbg_info.tokens;
    if (auto itr = std::find_if(tokens.begin(), tokens.end(),
                                [](ass::TokenInfo token_info) { return token_info.token_id == ass::TokenId::DC; });
        itr != tokens.end()) {
        // TODO:DC
    }

    //! 一行をトークンに分割するための正規表現
    const std::regex regsp = std::regex(R"(([,\s]*)([^,\s]+))");

    int index = 0;
    int start = 0;
    bool is_label = tokens.size() > 0 && tokens[0].token_id == ass::TokenId::LABEL;

    for (std::sregex_iterator it(dbg_info.line.cbegin(), dbg_info.line.cend(), regsp), end; it != end; ++it) {
        auto&& m = *it;

        cmn::C << C_OP << m[1].str();

        if (index == 0) start = (int)m[1].str().size();

        if (index >= 1 && is_label) {
            start += (int)m[1].str().size();
            is_label = false;
        }

        if (index < tokens.size() && tokens[index].token_id == ass::TokenId::COMMA) index++;
        if (index < tokens.size()) {
            if (tokens[index].token_id == ass::TokenId::LABEL) {
                cmn::C << C_LABEL;
            } else if (ass::GetTokenClass(tokens[index].token_id) == ass::REG_CLASS) {
                cmn::C << C_REGSTER;
            } else if (ass::GetTokenClass(tokens[index].token_id) == ass::ASEM_CLASS) {
                cmn::C << C_ASMOP;
            }
        }
        cmn::C << m[2].str();
        if (is_label) start += (int)m[2].str().size();
        index++;
    }
    cmn::C << C_RESET << std::endl;

    return start;
}

void Debugger::DisplaySrc(std::vector<std::string>& params) const {
    if (params.size() == 0)
        DisplaySrc(cii_cpu.PR);
    else {
        uint16_t start;
        uint16_t end;
        if (!CheckAddr(params[0], start)) {
            start = 0;
        }
        if (!(params.size() >= 2 && CheckAddr(params[1], end))) {
            end = UINT16_MAX;
        }
        DisplaySrc(start, end);
    }
}
void Debugger::DisplaySrc(uint16_t start, uint16_t end, bool opt) const {
    for (const auto& dbg_info : dbg_infos) {
        // cmn::C << "dbg_info.start_offset:" << dbg_info.start_offset << std::endl;
        // cmn::C << "dbg_info.end_offset:" << dbg_info.end_offset << std::endl;

        uint16_t off = dbg_info.end_offset - dbg_info.start_offset;
        bool next = true;
        if (opt) {
            if (off == 0) next = false;
        }

        if (dbg_info.start_offset >= start && dbg_info.end_offset < end && next) {
            const Memory& mem = cii_cpu.GetMemory();
            int offset_len = dbg_info.end_offset - dbg_info.start_offset;

            // label以外のTokenIdを取得
            ass::TokenId token_id = ass::TokenId::OTHER;
            if (dbg_info.tokens.size() > 0) token_id = dbg_info.tokens[0].token_id;
            if (token_id == ass::TokenId::LABEL && dbg_info.tokens.size() > 1) token_id = dbg_info.tokens[1].token_id;

            // アドレス表示
            if (!(token_id == ass::TokenId::IN || token_id == ass::TokenId::OUT) && offset_len > 0 &&
                dbg_info.start_offset == cii_cpu.PR)
                cmn::C << C_EXEC_ADR;
            cmn::C << C_ADDR << cmn::Format("%04x", dbg_info.start_offset);
            cmn::C << cmn::Color::RESET;

            // ブレークポイント表示
            if (dbg_info.is_break)
                cmn::C << C_BREAK;
            else
                cmn::C << "  ";
            cmn::C << C_ADDR;

            // マクロ表示
            if (token_id == ass::TokenId::IN || token_id == ass::TokenId::OUT) {
                cmn::C << "++++" << std::string(6, ' ');
                int start_offset = DisplayLine(dbg_info);
                DisplayMacro(start_offset, token_id, dbg_info);
                continue;
            }

            if (offset_len >= 1) {
                cmn::C << cmn::Format("%04x ", mem.memory[dbg_info.start_offset].data);
            }
            if (offset_len >= 2) {
                cmn::C << cmn::Format("%04x ", mem.memory[dbg_info.start_offset + 1].data);
            }
            if (offset_len == 0) cmn::C << std::string(10, ' ');
            if (offset_len == 1) cmn::C << std::string(5, ' ');

            DisplayLine(dbg_info);

            offset_len -= 2;
            int offset = dbg_info.start_offset + 2;

            // DSのときは、長さが長いと表示が長くなってしまうため、ちじめる
            // auto itr = std::find_if(dbg_info.tokens.begin(), dbg_info.tokens.end(), [](const ass::TokenInfo& info) {
            //     return info.token_id == ass::TokenId::DS || info.token_id == ass::TokenId::DC;
            // });
            bool is_ds = token_id == ass::TokenId::DS || token_id == ass::TokenId::DC;
            while (offset_len > 0) {
                cmn::C << C_ADDR << cmn::Format("%04x  ", offset);

                if (is_ds) {
                    for (int i = 0; i < 8 && offset_len > 0; i++) {
                        if (offset_len >= 1) {
                            cmn::C << cmn::Format("%04x ", mem.memory[offset].data);
                            offset_len--;
                            offset++;
                        }
                    }
                } else {
                    if (offset_len >= 1) {
                        cmn::C << cmn::Format("%04x ", mem.memory[offset].data);
                        offset_len--;
                        offset++;
                    }
                    if (offset_len >= 1) {
                        cmn::C << cmn::Format("%04x ", mem.memory[offset].data);
                        offset_len--;
                        offset++;
                    }
                }
                cmn::C << std::endl;
            }
        }
    }
    cmn::C << cmn::Color::RESET;
}

void Debugger::DisplayContents(int start, uint16_t offset, std::string line, int no, uint16_t data1,
                               uint16_t data2) const {
    cmn::C << C_ADDR;
    if (offset == cii_cpu.PR) cmn::C << C_EXEC_ADR;

    cmn::C << cmn::Format("%04x", offset) << C_RESET;
    cmn::C << "  " << C_ADDR;
    cmn::C << cmn::Format("%04x ", data1);
    if (no == 2)
        cmn::C << cmn::Format("%04x ", data2);
    else
        cmn::C << "     ";

    cmn::C << std::string(start, ' ');
    cmn::C << C_MACRO << line << C_RESET;
}

void Debugger::DisplayMacro(int start, ass::TokenId token_id, const ass::DbgInfo& dbg_info) const {
    int label_off = 1;
    if (dbg_info.tokens[0].token_id == ass::TokenId::LABEL) label_off = 2;

    std::string buf_name = dbg_info.tokens[label_off].label;
    std::string len_name = dbg_info.tokens[label_off + 2].label;

    std::string svc_no = token_id == ass::TokenId::IN ? "1" : "2";

    int index = 0;

    DisplayContents(start, dbg_info.start_offset + index, "PUSH\tGR1,0\n", 2,
                    mem.memory[dbg_info.start_offset + index].data, mem.memory[dbg_info.start_offset + index + 1].data);
    index += 2;

    DisplayContents(start, dbg_info.start_offset + index, "PUSH\tGR2,0\n", 2,
                    mem.memory[dbg_info.start_offset + index].data, mem.memory[dbg_info.start_offset + index + 1].data);
    index += 2;
    DisplayContents(start, dbg_info.start_offset + index, "LAD\tGR1," + buf_name + "\n", 2,
                    mem.memory[dbg_info.start_offset + index].data, mem.memory[dbg_info.start_offset + index + 1].data);
    index += 2;
    DisplayContents(start, dbg_info.start_offset + index, "LAD\tGR2," + len_name + "\n", 2,
                    mem.memory[dbg_info.start_offset + index].data, mem.memory[dbg_info.start_offset + index + 1].data);
    index += 2;
    DisplayContents(start, dbg_info.start_offset + index, "SVC\t" + svc_no + "\n", 2,
                    mem.memory[dbg_info.start_offset + index].data, mem.memory[dbg_info.start_offset + index + 1].data);
    index += 2;
    DisplayContents(start, dbg_info.start_offset + index, "POP\tGR2\n", 1,
                    mem.memory[dbg_info.start_offset + index].data, 0);
    index += 1;
    DisplayContents(start, dbg_info.start_offset + index, "POP\tGR1\n", 1,
                    mem.memory[dbg_info.start_offset + index].data, 0);
}
void Debugger::SetSingleStep() { cii_cpu.FR.SetSingleStep(ON); }

bool Debugger::CheckAddr(std::string param, uint16_t& addr) const {
    addr = 0;
    if (ass::Reader::IsDigit(param)) {
        addr = std::stoi(param, nullptr);
    } else if (ass::Reader::IsHex(param)) {
        std::string hex(param.begin() + 1, param.end());
        addr = std::stoi(hex, nullptr, 16);
    } else if (ass::Reader::IsLabel(param)) {
        addr = mem.FindSym(param);
        if (addr == UINT16_MAX) return false;
    } else {
        return false;
    }
    return true;
}

bool Debugger::SetBreakPoint(const std::vector<std::string>& params) {
    bool is_ok = true;
    for (auto& param : params) {
        uint16_t point;
        if (CheckAddr(param, point)) {
            if (!SetBreakPoint(point)) cmn::C << cmn::Format("Offset'%s'は範囲外です。\n", param.c_str());
        } else
            cmn::C << cmn::Format("'%s'はoffset形式ではありません。\n16進数で指定するときは'#12ab'です。\n",
                                  param.c_str());
    }
    DisplaySrc(0);
    return true;
}

bool Debugger::SetBreakPoint(uint16_t point) {
    if (auto itr = std::find_if(dbg_infos.begin(), dbg_infos.end(),
                                [&point](auto dbg_info) {
                                    return dbg_info.end_offset > dbg_info.start_offset &&
                                           point >= dbg_info.start_offset && point < dbg_info.end_offset;
                                });
        itr != dbg_infos.end()) {
        itr->is_break = true;
        cii_cpu.SetBreakPoint(itr->start_offset);
        return true;
    }
    return false;
}

void Debugger::ClearBreakPoints(std::vector<std::string>& params) {
    for (auto& param : params) {
        if (param == "*") {
            ClearAllBreakPoints();
            break;
        }
        uint16_t point;
        if (CheckAddr(param, point)) ClearBreakPoint(point);
    }
}

void Debugger::ClearBreakPoint(uint16_t point) {
    if (auto itr =
            std::find_if(dbg_infos.begin(), dbg_infos.end(), [&point](auto dbg_info) { return dbg_info.is_break; });
        itr != dbg_infos.end()) {
        itr->is_break = false;
        cii_cpu.DeleteBreakPoint(itr->start_offset);
    }
}
void Debugger::ClearAllBreakPoints() {
    for (auto& dbg_info : dbg_infos) {
        if (dbg_info.is_break) {
            cii_cpu.DeleteBreakPoint(dbg_info.start_offset);
            dbg_info.is_break = false;
        }
    }
}

void Debugger::DisplayOneReg(const char* reg_name, uint16_t reg, uint16_t pre_reg) const {
    cmn::C << C_REG << reg_name << " = ";
    if (reg != pre_reg) cmn::C << C_REG_DIFF;
    cmn::C << cmn::Format("%04x", reg) << cmn::Color::RESET;
}
void Debugger::DisplayOneReg(const char* reg_name, bool flag, bool pre_flag) const {
    cmn::C << C_REG << reg_name << " = ";
    if (flag != pre_flag) cmn::C << C_REG_DIFF;
    cmn::C << cmn::Format("%d", flag) << cmn::Color::RESET;
}

}  // namespace cii
