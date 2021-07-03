#include "assem_mem.h"

#include <list>
#include <utility>

#include "comet_ii.h"

namespace cii {
AssmMem::AssmMem(uint32_t msize, WordData *mem, int off) {
    size = msize;
    memory = mem;
    offset = off;
}

AssmMem &AssmMem::operator<<(OpWord op) {
    memory[offset++].opword = op;
    return *this;
}

AssmMem &AssmMem::operator<<(SymRef sym) {
    sym_refs.push_back(std::make_pair(sym.sym_ref, offset));
    memory[offset++].opword = 0;
    return *this;
}

AssmMem &AssmMem::operator<<(SymDef sym) {
    sym_defs.push_back(std::make_pair(sym.sym_def, offset));
    return *this;
}
AssmMem &AssmMem::operator<<(SymStart sym) {
    sym_externs.push_back(std::make_pair(sym.sym_def, offset));
    return *this;
}

AssmMem &AssmMem::operator<<(SymConst sym) {
    operator<<(static_cast<SymRef>(sym));
    sym_consts.push_back(std::make_pair(sym.sym_ref, sym.def_const));
    return *this;
}

AssmMem &AssmMem::operator<<(SymDC sym) {
    sym_defs.push_back(std::make_pair(sym.sym_def, offset));
    memory[offset++].opword = sym.def_const;
    return *this;
}
AssmMem &AssmMem::operator<<(SymDS sym) {
    sym_defs.push_back(std::make_pair(sym.sym_def, offset));

    for (int i = 0; i < sym.ds_size; i++) {
        memory[offset++].opword = 0;
    }
    return *this;
}

AssmMem &AssmMem::operator<<(DcDef dc_def) {
    memory[offset++].opword = dc_def.value;
    return *this;
}

AssmMem &AssmMem::operator<<(DsDef ds_def) {
    for (int i = 0; i < ds_def.value; i++) {
        memory[offset++].opword = 0;
    }
    return *this;
}

AssmMem &AssmMem::operator<<(uint16_t v) {
    memory[offset++].opword = v;
    return *this;
}

AssmMem &AssmMem::operator<<(int v) {
    memory[offset++].opword = v;
    return *this;
}

AssmMem &AssmMem::operator<<(const char *str) {
    while (*str != '\0') {
        memory[offset++].opword = *str++;
    }
    return *this;
}

void AssmMem::Clear() {
    offset = 0;
    sym_defs.clear();
    sym_refs.clear();
    sym_consts.clear();
    sym_externs.clear();
    syms.clear();
    ClearMem();
}

uint16_t AssmMem::FindSym(std::string sym) const {
    if (auto itr = std::find_if(sym_externs.begin(), sym_externs.end(), [&](SymValue v) { return v.first == sym; });
        itr != sym_externs.end()) {
        return itr->second;
    }

    if (syms.size() == 0) {
        if (auto itr = std::find_if(sym_defs.begin(), sym_defs.end(), [&](SymValue v) { return v.first == sym; });
            itr != sym_defs.end()) {
            return itr->second;
        }
    } else {
        for (auto [defs, refs] : syms) {
            if (auto itr = std::find_if(defs.begin(), defs.end(), [&](SymValue v) { return v.first == sym; });
                itr != defs.end()) {
                return itr->second;
            }
        }
    }
    return UINT16_MAX;
}

bool AssmMem::LinkSym(std::vector<SymValue> &defs, std::vector<SymValue> &refs) {
    int find_count = 0;

    for (auto &ref : refs) {
        // externシンボル
        if (auto itr = std::find_if(sym_externs.begin(), sym_externs.end(),
                                    [&ref](SymValue sym) { return ref.first == sym.first; });
            itr != sym_externs.end()) {
            memory[ref.second].data = itr->second;
            find_count++;
            continue;
        }
        // localシンボル
        if (auto itr = std::find_if(defs.begin(), defs.end(), [&ref](SymValue sym) { return ref.first == sym.first; });
            itr != defs.end()) {
            memory[ref.second].data = itr->second;
            find_count++;
        }
    }
    return refs.size() == find_count;
}

bool AssmMem::CheckSym(std::string &sym_name) {
    // externシンボル
    if (auto itr = std::find_if(sym_externs.begin(), sym_externs.end(),
                                [&sym_name](SymValue sym) { return sym_name == sym.first; });
        itr != sym_externs.end()) {
        return false;
    }
    // localシンボル
    if (auto itr =
            std::find_if(sym_defs.begin(), sym_defs.end(), [&sym_name](SymValue sym) { return sym_name == sym.first; });
        itr != sym_defs.end()) {
        return false;
    }
    return true;
}

void AssmMem::SnapShot() {
    syms.push_back(std::make_pair(sym_defs, sym_refs));

    sym_defs.clear();
    sym_refs.clear();
}

bool AssmMem::End() {
    // 定数で重複しているものを削除
    std::sort(sym_consts.begin(), sym_consts.end(), [](SymValue s, SymValue d) { return s.first < d.first; });
    sym_consts.erase(std::unique(sym_consts.begin(), sym_consts.end()), sym_consts.end());
    // 定数を登録
    for (auto &&key : sym_consts) {
        sym_externs.push_back(std::make_pair(key.first, offset));
        operator<<(key.second);
    }

    bool link_ok = true;
    if (syms.size() > 0) {
        for (auto [defs, refs] : syms) {
            if (!LinkSym(defs, refs)) link_ok = false;
        }
    } else {
        if (!LinkSym(sym_defs, sym_refs)) link_ok = false;
    }

    return link_ok;
}

void AssmMem::ClearMem() {
    for (size_t i = 0; i < size; i++) {
        memory[i].data = 0;
    }
}
}  // namespace cii
