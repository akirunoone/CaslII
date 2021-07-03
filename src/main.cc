
#include <fstream>
#include <iostream>

#include "assembler.h"
#include "builder.h"
#include "conf.h"
#include "debugger.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace {
const cii::ColorChar C_START(cmn::Color::F_BLUE);
const cii::ColorChar C_ERROR(cmn::Color::F_BRIGHT_RED);
const cii::ColorChar C_RESET(cmn::Color::RESET);

cii::CommetIIEnv commetII_env;
}  // namespace

int main(int argc, char* argv[]) {
#ifdef _MSC_VER
    // DOS窓のcode pageをUTF-8にする
    SetConsoleOutputCP(CP_UTF8);
    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

    if (argc < 2) {
        cmn::C << "ファイル名が指定されていません。";
        return 1;
    }

    std::vector<std::string> files;

    std::copy(&argv[1], &argv[argc], std::back_inserter(files));

    Builder build{commetII_env};

    ass::DbgInfos all_dbg_infos;
    bool is_ok = build.Build(files, all_dbg_infos);

    if (!is_ok) return 1;

    cmn::C << C_START << "Casl Debugger 1.0\n"
           << "Debugger Starting...\n"
           << "Memory Word Size: " << commetII_env.mem.size << std::endl
           << "Used Word Size: " << commetII_env.mem.GetOffset() << std::endl
           << C_RESET << std::endl;

    cii::Debugger debug(commetII_env.cii_cpu, all_dbg_infos, commetII_env.mem);
    debug.Start();
    return 0;
}