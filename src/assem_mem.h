#ifndef ASEM_MEM_H_
#define ASEM_MEM_H_

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "comet_ii.h"

namespace cii {
struct SymDef {
    std::string sym_def;
    SymDef(const std::string &sym) : sym_def(sym) {}
};
struct SymStart : SymDef {
    SymStart(const std::string &sym) : SymDef(sym) {}
};

struct SymRef {
    std::string sym_ref;
    // bool sym_def_found;
    SymRef(const std::string &sym) : sym_ref(sym) {}
};

struct SymDC : public SymDef {
    uint16_t def_const;
    SymDC(const std::string &sym, uint16_t v) : SymDef(sym), def_const(v) {}
};

struct SymDS : public SymDef {
    uint16_t ds_size;
    SymDS(const std::string &sym, uint16_t size) : SymDef(sym) { ds_size = size; }
};

struct SymConst : public SymRef {
    uint16_t def_const;
    SymConst(const std::string &sym, uint16_t v) : SymRef(sym), def_const(v) {
        // std::cout << sym << " " << v << std::endl;
    }
};

struct DcDef {
    uint16_t value;
    DcDef(uint16_t v) : value(v) {}
};

struct DsDef : public DcDef {
    DsDef(uint16_t v) : DcDef(v) {}
};

struct Halt {};

struct IOSVC {
    SVCNo svc_no;
    std::string buf_name;
    std::string len_name;
    IOSVC(SVCNo no, const std::string &buf, const std::string &len) : svc_no(no), buf_name(buf), len_name(len) {}
};

template <class T>
T &operator<<(T &mem, IOSVC svc) {
    mem << OpWord(OpCode::PUSH, Reg::GR1) << 0;
    mem << OpWord(OpCode::PUSH, Reg::GR2) << 0;
    mem << OpWord(OpCode::LAD, Reg::GR1) << SymRef(svc.buf_name);
    mem << OpWord(OpCode::LAD, Reg::GR2) << SymRef(svc.len_name);
    mem << OpWord(OpCode::SVC) << static_cast<uint16_t>(svc.svc_no);
    mem << OpWord(OpCode::POP, Reg::GR2);
    mem << OpWord(OpCode::POP, Reg::GR1);

    return mem;
}

template <class T>
T &operator<<(T &mem, Halt) {
    mem << OpWord(OpCode::HLT);
    return mem;
}
using SymValue = std::pair<std::string, uint16_t>;

/**
 * @brief メモリクラス
 *
 */
class AssmMem : public Memory {
   private:
    std::vector<SymValue> sym_defs;     //!< シンボル定義
    std::vector<SymValue> sym_externs;  //!< 外部シンボル
    std::vector<SymValue> sym_refs;     //!< シンボル参照
    std::vector<SymValue> sym_consts;   //!< コンスタント　シンボル

    std::vector<std::pair<std::vector<SymValue>, std::vector<SymValue>>> syms;
    int offset;  //!< アセンブル出力最終位置

   public:
    AssmMem() = delete;
    AssmMem(uint32_t msize, WordData *mem, int off);

    inline void Start() { Clear(); }
    /**
     * @brief アセンブル終了。シンボルのリンクを行う。
     *
     * @return true リンク成功
     * @return false リンクエラー
     */
    bool End();
    /**
     * @brief メモリの初期化を行う。
     *
     */
    void ClearMem();

    /**
     * @brief OpWordのメモリへの挿入
     *
     * @param op 命令ワード
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(OpWord op);
    /**
     * @brief シンボル参照の挿入
     *
     * @param sym シンボル情報
     * @return AssmMem&　アセンブルメモリ
     */
    AssmMem &operator<<(SymRef sym);
    /**
     * @brief シンボル定義の挿入
     *
     * @param sym シンボル定義情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(SymDef sym);
    /**
     * @brief Startシンボルの挿入
     *
     * @param sym Startシンボル情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(SymStart sym);
    /**
     * @brief DCシンボルの挿入
     *
     * @param sym DCシンボル定義情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(SymDC sym);
    /**
     * @brief DSシンボルの挿入
     *
     * @param sym DSシンボル定義情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(SymDS sym);
    /**
     * @brief Const定義の挿入
     *
     * @param sym Cons定義情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(SymConst sym);
    /**
     * @brief DC定義の挿入
     *
     * @param dc_def DC定義情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(DcDef dc_def);
    /**
     * @brief DS定義の挿入
     *
     * @param ds_def DS定義情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(DsDef ds_def);
    /**
     * @brief 値の挿入
     *
     * @param v 値情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(uint16_t v);
    /**
     * @brief 値の挿入
     *
     * @param v 値情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(int v);
    /**
     * @brief 文字列の挿入
     *
     * @param str 文字列情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(const char *str);
    /**
     * @brief 文字列の挿入
     *
     * @param str 文字列情報
     * @return AssmMem& アセンブルメモリ
     */
    AssmMem &operator<<(const std::string str) { return operator<<(str.c_str()); }

    // for DEBUG
    uint16_t Dump(const char *sym, uint16_t &m, int offset = 0) {
        auto itr = std::find_if(sym_defs.begin(), sym_defs.end(), [&](SymValue s) { return s.first == sym; });
        if (itr != sym_defs.end()) {
            m = memory[itr->second + offset];
            return true;
        }
        return false;
    }
    void Dump() {
        std::cout << "offset:" << offset << std::endl;
        for (int i = 0; i < offset; i++) {
            std::cout << std::hex << std::setw(4) << std::setfill('0') << i << "    " << std::hex << std::setw(4)
                      << std::setfill('0') << memory[i].data << std::endl;
        }
        std::cout << std::resetiosflags(std::ios_base::floatfield);
    }

    /**
     * @brief Get the Offset object
     * アセンブルの出力位置を取得する。
     * @return uint16_t
     */
    uint16_t GetOffset() const { return offset; }
    /**
     * @brief シンボルのoffsetを取得する
     *
     * @param sym　シンボル
     * @return uint16_t　offset
     */
    uint16_t FindSym(std::string sym) const;
    const std::vector<SymValue> &GetSymExtern() const { return sym_externs; }
    const auto &GetSyms() const { return syms; }

    void SnapShot();
    bool CheckSym(std::string &sym_name);

   private:
    void Clear();
    bool LinkSym(std::vector<SymValue> &def, std::vector<SymValue> &ref);
};

}  // namespace cii
#endif