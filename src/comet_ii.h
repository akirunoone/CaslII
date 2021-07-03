#ifndef COMMETII_H__
#define COMMETII_H__

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <vector>

namespace cii {
class CommetError : public std::exception {};
class InvalidOperationError : public CommetError {};
class IlleagalAccessError : public CommetError {};
class StackOverflowError : public CommetError {};
class StackUnderflowError : public CommetError {};

enum class CauseOfStop {
    OK,
    SINGLE_STEP,
    HALT,
    ILLEGAL_ACCESS,
    INVALID_OPERATION,
    STACK_OVERFLOW,
    STACK_UNDERFLOW,
    BREAK_POINT,
};
/**
 * @enum class Reg
 * レジスタ ID
 */
enum class Reg : uint8_t {
    GR0,       // 汎用レジスタ
    GR1,       // 汎用レジスタ
    GR2,       // 汎用レジスタ
    GR3,       // 汎用レジスタ
    GR4,       // 汎用レジスタ
    GR5,       // 汎用レジスタ
    GR6,       // 汎用レジスタ
    GR7,       // 汎用レジスタ
    SP,        // スタックポインタ
    PR,        // プログラムレジスタ
    FR,        // フラグレジスタ
    NONE = 0,  // なし
};
/**
 * @enum class OpCode
 * 命令語
 */
enum class OpCode : uint8_t {
    NOP,                     //! ノーオペレーション
    PRI_MOVE = 0x10,         //! ★ムーブ
    LD_M = PRI_MOVE | 0,     //! メモリロード
    ST = PRI_MOVE | 1,       //! ストア
    LAD = PRI_MOVE | 2,      //! ロードアドレス
    LD_R = PRI_MOVE | 4,     //! ロードレジスタ
    PRI_ARITH = 0x20,        //! ★算術演算
    ADDA_M = PRI_ARITH | 0,  //! 算術加算
    ADDL_M = PRI_ARITH | 1,  //! 論理加算
    SUBA_M = PRI_ARITH | 2,  //! 算術減算
    SUBL_M = PRI_ARITH | 3,  //! 論理減算
    ADDA_R = PRI_ARITH | 4,  //! 算術加算レジスタ
    ADDL_R = PRI_ARITH | 5,  //! 論理加算レジスタ
    SUBA_R = PRI_ARITH | 6,  //! 算術減算レジスタ
    SUBL_R = PRI_ARITH | 7,  //! 論理減算レジスタ
    PRI_LOGI = 0x30,         //! ★論理演算
    AND_M = PRI_LOGI | 0,    //! 論理積
    OR_M = PRI_LOGI | 1,     //! 論理和
    XOR_M = PRI_LOGI | 2,    //! 排他的論理和
    AND_R = PRI_LOGI | 4,    //! 論理積
    OR_R = PRI_LOGI | 5,     //! 論理和
    XOR_R = PRI_LOGI | 6,    //! 排他的論理和
    PRI_CMP = 0x40,          //! ★比較
    CPA_M = PRI_CMP | 0,     //! 算術比較
    CPL_M = PRI_CMP | 1,     //! 論理比較
    CPA_R = PRI_CMP | 4,     //! 算術比較
    CPL_R = PRI_CMP | 5,     //! 論理比較
    PRI_SHIFT = 0x50,        //! ★シフト演算
    SLA = PRI_SHIFT | 0,     //! 算術左シフト
    SRA = PRI_SHIFT | 1,     //! 算術右シフト
    SLL = PRI_SHIFT | 2,     //! 論理左シフト
    SRL = PRI_SHIFT | 3,     //! 論理右シフト
    PRI_JUMP = 0x60,         //! ★分岐命令
    JPL = PRI_JUMP | 0,      //! 正分岐
    JMI = PRI_JUMP | 1,      //! 負分岐
    JNZ = PRI_JUMP | 2,      //! 非零分岐
    JZE = PRI_JUMP | 3,      //! 零分岐
    JOV = PRI_JUMP | 4,      //! オーバーフロー分岐
    JUMP = PRI_JUMP | 5,     //! 無条件分岐
    PRI_STACKOP = 0x70,      //! ★スタック操作命令
    PUSH = PRI_STACKOP | 0,  //! プッシュ
    POP = PRI_STACKOP | 1,   //! ポップ
    PRI_CALL = 0x80,         //! ★コール
    CALL = PRI_CALL | 0,     //! コール
    RET = PRI_CALL | 1,      //! リターン
    PRI_SVC = 0xf0,          //! ★SVC
    SVC = PRI_SVC | 0,       //! スーパーバイザコール
    HLT,
};

enum class SVCNo : uint16_t {
    SVC_IN = 1,
    SVC_OUT,
};

enum ExeError { EXE_ };

enum Flag { OFF = 0, ON = 1 };

constexpr uint16_t SIGNED_BIT = 0x8000;
constexpr int32_t MIN_WORD_NUM = (int32_t)((int16_t)SIGNED_BIT);
constexpr int32_t MAX_WORD_NUM = (int32_t)((int16_t)0x7fff);

inline Flag IsSigned(uint16_t v) { return (v & SIGNED_BIT) ? ON : OFF; }

/**
 * 命令ワード
 */
struct OpWord {
    uint16_t des_reg : 4;  //!< 代入先レジスタ番号
    uint16_t src_reg : 4;  //!< ソースレジスタ番号
    uint16_t op_code : 8;  //!< 命令コード

    OpWord(uint16_t w) {
        union OpWordDummy {
            OpWord op_word;
            uint16_t w;
            OpWordDummy(uint16_t w16) : w(w16) {}
        };
        OpWordDummy word{w};
        des_reg = word.op_word.des_reg;
        src_reg = word.op_word.src_reg;
        op_code = word.op_word.op_code;
    }

    OpWord(OpCode op, Reg dreg = Reg::GR0, Reg sreg = Reg::GR0) {
        if (op == OpCode::JMI || op == OpCode::JNZ || op == OpCode::JZE || op == OpCode::JUMP || op == OpCode::JPL ||
            op == OpCode::JOV || op == OpCode::PUSH || op == OpCode::CALL || op == OpCode::SVC) {
            SetOp(op, dreg, Reg::GR0);
        } else {
            SetOp(op, sreg, dreg);
        }
    }

    void SetOp(OpCode op, Reg sreg, Reg dreg) {
        op_code = static_cast<uint16_t>(op);
        src_reg = static_cast<uint16_t>(sreg);
        des_reg = static_cast<uint16_t>(dreg);
    }
    /**
     * 命令コードを取り出す
     * @return
     * 命令コード
     */
    OpCode GetOpCode() const { return static_cast<OpCode>(op_code); }
};

/**
 * CommetII ワードデータ定義
 */
union WordData {
    uint16_t data;
    OpWord opword;

    WordData() {}
    operator uint16_t() const { return data; }
};

/**
 * @struct
 * CommetII メモリ
 */
struct Memory {
    uint32_t size;     //!< メモリワードサイズ
    WordData *memory;  //!< メモリ実態
};

/**
 * @class
 * Commet II
 */
class CometII {
    Memory *ram;     //!< メモリ
    uint16_t GR[8];  //!< 汎用レジスタ配列
    std::ostream *svc_out;
    std::istream *svc_in;
    std::vector<uint16_t> break_points;
    uint16_t pre_pr;
    uint32_t counter;
    /**
     * フラグレジスタ
     */
    struct FlagReg {
        uint8_t OF : 1;   //!< オーバーフローフラグ
        uint8_t SF : 1;   //!< 符号フラグ
        uint8_t ZF : 1;   //!< ゼロフラグ
        uint8_t HLT : 1;  //!< HALT フラグ
        uint8_t SS : 1;   //!< シングルスッテップフラグ
        /**
         * フラグをクリアする
         */
        void Clear() {
            OF = OFF;
            SF = OFF;
            ZF = OFF;
            HLT = OFF;
            SS = OFF;
        };

        /**
         * 算術演算計算結果からオーバフローフラグを設定する
         * @param
         * result 算術演算計算結果
         */
        void SetOverflow(int32_t result) { OF = (result >= MIN_WORD_NUM && result <= MAX_WORD_NUM) ? OFF : ON; }

        /**
         * 論理演算計算結果からオーバフローフラグを設定する
         * @param
         * result 論理演算計算結果
         */
        void SetOverflow(uint32_t result) {
            const uint32_t overflow_bit = 1 << 16;
            OF = (result & overflow_bit) ? ON : OFF;
        }

        /**
         * 符号フラグを設定する
         * @param
         * result 計算結果
         */
        void SetSigned(uint16_t result) { SF = cii::IsSigned(result); }
        /**
         * ゼロフラグを設定する
         * @param
         * result 計算結果
         */
        void SetZero(uint16_t result) { ZF = result == 0 ? ON : OFF; }
        /**
         * 算術演算結果のフラグを設定する
         * @param
         * result 算術演算結果計算結果
         */
        void SetFlags(int32_t result) {
            SetOverflow(result);
            SetSigned(result);
            SetZero(result);
        }

        /**
         * 論理演算結果のフラグを設定する
         * @param
         * result 論理演算結果計算結果
         */
        void SetFlags(uint32_t result) {
            SetOverflow(result);
            SetSigned(result);
            SetZero(result);
        }

        void SetSingleStep(Flag f) { SS = f; }

        /**
         * オーバーフラグをクリアし、その他のフラグを設定する
         * @param
         * result 演算結果計算結果
         */
        void SetFlagsClearOver(uint16_t result) {
            OF = OFF;
            SetSigned(result);
            SetZero(result);
        }

        /**
         * オーバーフローかどうかを返す
         * @return
         * true オーバフロー
         */
        bool IsOverflow() const { return OF == ON; }
        /**
         * 負数であるかどうかを返す
         * @return
         * true 負数
         */
        bool IsSigned() const { return SF == ON; }
        /**
         * ゼロであるかどうかを返す
         * @return
         * true ゼロ
         */
        bool IsZero() const { return ZF == ON; }
        /**
         * HALTを検出したかどうかを返す
         * @return
         * true HALT
         */
        bool IsHalt() const { return HLT == ON; }

        bool IsSingleStep() const { return SS == ON; }
    };

   public:
    uint16_t &GR0 = GR[0];  //!< 汎用レジスタ0
    uint16_t &GR1 = GR[1];  //!< 汎用レジスタ1
    uint16_t &GR2 = GR[2];  //!< 汎用レジスタ2
    uint16_t &GR3 = GR[3];  //!< 汎用レジスタ3
    uint16_t &GR4 = GR[4];  //!< 汎用レジスタ4
    uint16_t &GR5 = GR[5];  //!< 汎用レジスタ5
    uint16_t &GR6 = GR[6];  //!< 汎用レジスタ6
    uint16_t &GR7 = GR[7];  //!< 汎用レジスタ7
    uint16_t SP;            //!< スタックポインタ
    uint16_t PR;            //!< プログラムカウンタ
    FlagReg FR;             //!< フラグレジスタ

   public:
    CometII(Memory *mem, std::ostream &out = std::cout, std::istream &in = std::cin);
    virtual ~CometII();
    /**
     * @brief
     * リセットレジスタ
     */
    void Reset();
    /**
     * @brief
     * CommetII実行
     */
    CauseOfStop Run();

    void SetSvcIn(std::istream &is) { svc_in = &is; }
    void SetSvcOut(std::ostream &os) { svc_out = &os; }

    uint16_t GetReg(int reg_no) const { return GR[reg_no]; }

    const Memory &GetMemory() const { return *ram; }

    void SetBreakPoint(uint16_t point) {
        auto itr = std::find(break_points.begin(), break_points.end(), point);
        if (itr == break_points.end()) {
            break_points.push_back(point);
        }
    }

    void DeleteBreakPoint(uint16_t point) {
        auto itr = std::find(break_points.begin(), break_points.end(), point);
        if (itr != break_points.end()) {
            break_points.erase(itr);
        }
    }
    const std::vector<uint16_t> &GetBreakPoints() const { return break_points; }
    uint32_t GetExcutedCounter() const { return counter; }

   protected:
    /**
     * @brief
     * PRがさすアドレスのワードデータをフェッチする
     * @return
     * ワードデータ
     * @note
     * PRは更新される
     */
    inline WordData FetchWordData() { return ram->memory[PR++]; }
    /**
     * @brief
     * 指定アドレスのワードデータをフェッチする
     * @param adr
     * フェッチアドレス
     * @return
     * ワードデータ
     */
    inline uint16_t FetchWordData(uint16_t adr) {
        if (adr >= ram->size) throw IlleagalAccessError();

        return ram->memory[adr].data;
    }
    /**
     * @brief
     * 指定アドレスにワードデータをストアする
     * @param adr
     * ストアアドレス
     * @param data
     * ストアデータ
     */
    inline void StoreData(uint16_t adr, uint16_t data) {
        if (adr >= ram->size) throw IlleagalAccessError();

        ram->memory[adr].data = data;
    }
    /**
     * @brief
     * ワードデータを符号あり32bitに変換する
     * @param data
     * ワードデータ
     */
    inline int32_t signed_cast32(uint16_t data) { return static_cast<int32_t>(static_cast<int16_t>(data)); }

    void ExecOneStep();
    uint16_t EffectiveAdr(OpWord opword);
    void LoadReg(OpWord opword);
    void LoadMem(OpWord opword);
    void Store(OpWord opword);
    void LoadAdr(OpWord opword);
    void AddAReg(OpWord opword);
    void AddAMem(OpWord opword);
    void SubAReg(OpWord opword);
    void SubAMem(OpWord opword);
    void AddLReg(OpWord opword);
    void AddLMem(OpWord opword);
    void SubLReg(OpWord opword);
    void SubLMem(OpWord opword);
    void AddA(uint16_t &des, uint16_t src);
    void SubA(uint16_t &des, uint16_t src);
    void AddL(uint16_t &des, uint16_t src);
    void SubL(uint16_t &des, uint16_t src);
    void AndReg(OpWord opword);
    void AndMem(OpWord opword);
    void OrReg(OpWord opword);
    void OrMem(OpWord opword);
    void XorReg(OpWord opword);
    void XorMem(OpWord opword);
    void CompAReg(OpWord opword);
    void CompLReg(OpWord opword);
    void CompAMem(OpWord opword);
    void CompLMem(OpWord opword);

    Flag IsSigned(uint16_t v);

    void ShiftLeftA(OpWord opword);

    void ShiftRightA(OpWord opword);
    void ShiftLeftL(OpWord opword);
    void ShiftRightL(OpWord opword);
    void JumpOnPlus(OpWord opword);
    void JumpOnMinus(OpWord opword);
    void JumpOnNonZero(OpWord opword);
    void JumpOnZero(OpWord opword);
    void JumpOnOverflow(OpWord opword);
    void Jump(OpWord opword);
    void Push(OpWord opword);
    void Pop(OpWord opword);
    void CallSub(OpWord opword);
    void ReturnFromSub(OpWord opword);
    void Svc(OpWord opword);
    void SvcIn(OpWord opword);
    void SvcOut(OpWord opword);
};
// test
// test2
}  // namespace cii
#endif