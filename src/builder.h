#ifndef BUILDER_H_
#define BUILDER_H_

#include <string>
#include <vector>

#include "assem_mem.h"
#include "assembler.h"
#include "comet_ii.h"
#include "common.h"
#include "conf.h"

/**
 * @brief ビルド
 *
 */
class Builder {
    cii::AssmMem& mem;
    cii::CometII& cii_cpu;
    ass::Assembler assem;

   public:
    Builder(cii::CommetIIEnv& commetII_env) : mem(commetII_env.mem), cii_cpu(commetII_env.cii_cpu) {}
    bool Build(std::vector<std ::string> files, ass::DbgInfos& all_dbg_infos);
    void LinkError(ass::DbgInfos& dbg_infos, std::vector<int>& dbg_info_index, std::vector<std ::string> files);
};

#endif