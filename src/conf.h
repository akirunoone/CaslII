#ifndef CONF_H_
#define CONF_H_

#include "assem_mem.h"
#include "comet_ii.h"

namespace cii {

/**
 * @brief commetII 環境定義
 *
 */
struct CommetIIEnv {
    //! メモリワードサイズ
    static const size_t MEM_SIZE = 1024 * 4;

    //! メモリ
    cii::WordData words[MEM_SIZE];
    //! メモリ定義
    cii::AssmMem mem = {(uint32_t)MEM_SIZE, words, 0};
    //! commentII環境
    cii::CometII cii_cpu = {&mem};
};
}  // namespace cii

#endif