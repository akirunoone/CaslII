
#include "builder.h"

#include <fstream>
#include <iostream>

#include "assembler.h"
#include "debugger.h"

namespace {
const cii::ColorChar C_START(cmn::Color::F_BLUE);
const cii::ColorChar C_ERROR(cmn::Color::F_BRIGHT_RED);
const cii::ColorChar C_RESET(cmn::Color::RESET);
static const char* err_msg[] = {"OK",
                                "\t\t命令コードがありません",
                                "\t\tオペランドがありません",
                                "\t\tオペランドが間違ってます",
                                "\t\tDC, DSが宣言されていません",
                                "\t\t\"%s\"が見つかりません",
                                "\t\tシンボルが既に定義されています",
                                "\t\t不明なエラーです"};

std::ostream& operator<<(std::ostream& os, ass::AsmErrCode err) {
    cmn::C << C_ERROR << err_msg[(int)err] << C_RESET;
    return os;
}

std::ostream& operator<<(std::ostream& os, ass::DbgInfo dbg_info) {
    if (dbg_info.err == ass::AsmErrCode::NO_DEF_SYM) {
        auto itr = std::find_if(dbg_info.tokens.begin(), dbg_info.tokens.end(),
                                [](ass::TokenInfo token_info) { return token_info.token_id == ass::TokenId::LABEL; });
        if (itr != dbg_info.tokens.end()) {
            cmn::C << C_ERROR << cmn::Format(err_msg[(int)dbg_info.err], itr->label.c_str()) << C_RESET;
        } else {
            cmn::C << C_ERROR << err_msg[(int)ass::AsmErrCode::ERR] << C_RESET;
        }
    } else {
        cmn::C << C_ERROR << err_msg[(int)dbg_info.err] << C_RESET;
    }

    return os;
}
}  // namespace
// Builder::Builder(cmn::CommetIIEnv& commetII_env) {}

void Builder::LinkError(ass::DbgInfos& dbg_infos, std::vector<int>& dbg_info_index, std::vector<std ::string> files) {
    // リンクエラー
    auto syms = mem.GetSyms();
    auto sym_externs = mem.GetSymExtern();

    int index = 0;
    for (auto&& [sym_def, sym_ref] : syms) {
        // 未定義のシンボルを検索してエラー行を特定する
        for (auto& ref : sym_ref) {
            auto itr_ext = std::find_if(sym_externs.begin(), sym_externs.end(),
                                        [&ref](auto sym) { return ref.first == sym.first; });
            if (itr_ext == sym_externs.end()) {
                if (auto itr = std::find_if(sym_def.begin(), sym_def.end(),
                                            [&ref](auto sym) { return ref.first == sym.first; });
                    itr == sym_def.end()) {
                    auto itr_dbg =
                        std::find_if(dbg_infos.begin() + dbg_info_index[index], dbg_infos.end(), [&](ass::DbgInfo e) {
                            uint16_t off_diff = e.end_offset - e.start_offset;
                            if (e.err == ass::AsmErrCode::OK && off_diff != 0 &&
                                (ref.second >= e.start_offset && ref.second < e.end_offset)) {
                                // e.err = ass::AsmErrCode::NO_DEF_SYM;
                                return true;
                            }
                            return false;
                        });

                    if (itr_dbg != dbg_infos.end()) itr_dbg->err = ass::AsmErrCode::NO_DEF_SYM;
                }
            }
        }
        index++;
    }

    // エラー行の表示
    int line_num = 1;
    int f_index = 0;
    index = 0;
    for (auto&& dbg_info : dbg_infos) {
        if (dbg_info.err == ass::AsmErrCode::NO_DEF_SYM) {
            cmn::C << files[f_index] << ":" << line_num << " " << dbg_info.line << std::endl;
            cmn::C << dbg_info << std::endl;
        }
        index++;
        line_num++;
        if (dbg_info_index[f_index] == index) {
            line_num = 1;
            f_index++;
        }
    }
}

bool Builder::Build(std::vector<std ::string> files, ass::DbgInfos& all_dbg_infos) {
    mem.Start();

    bool asm_error = false;

    // ass::DbgInfos all_dbg_infos;

    std::vector<int> dbg_info_index;
    dbg_info_index.push_back(0);
    for (auto file : files) {
        std::ifstream ifs(file);
        if (!ifs.is_open()) {
            cmn::C << "ファイルのオープンに失敗しました:" << file << std::endl;
            return false;
        }

        assem.Assemble(ifs, mem);

        std::copy(assem.dbg_infos.begin(), assem.dbg_infos.end(), std::back_inserter(all_dbg_infos));

        if (assem.is_error) {
            int num = 1;
            for (auto& e : assem.dbg_infos) {
                if (e.err != ass::AsmErrCode::OK) {
                    cmn::C << file << ":" << num << " " << e.line << std::endl;
                    cmn::C << e << std::endl;
                }
                num++;
            }
            asm_error = true;
        }
        mem.SnapShot();
        dbg_info_index.push_back((int)assem.dbg_infos.size());
    }
    if (asm_error) return false;

    if (!mem.End()) {
        // リンクエラー
        LinkError(all_dbg_infos, dbg_info_index, files);
        return false;
    }

    return true;
}
