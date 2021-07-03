#ifndef DEBUGGER_H_
#define DEBUGGER_H_
#include "assembler.h"
#include "common.h"

namespace cii {
/**
 * @brief デバッガーコマンドID
 *
 */
enum class CmdId {
    NONE,                //!< NONE
    SHOW_REG_ALL,        //!< レジスタ全表示
    SHOW_REG_GR0,        //!< GR0表示
    SHOW_REG_GR1,        //!< GR1表示
    SHOW_REG_GR2,        //!< GR2表示
    SHOW_REG_GR3,        //!< GR3表示
    SHOW_REG_GR4,        //!< GR4表示
    SHOW_REG_GR5,        //!< GR5表示
    SHOW_REG_GR6,        //!< GR6表示
    SHOW_REG_GR7,        //!< GR7表示
    BREAK_POINT,         //!< ブレークポイントの設定
    CLEAR_BREAK_POINTS,  //!< ブレークポイントのクリア
    LIST_SRC,            //!< ソースリスト表示
    SINGLE_STEP,         //!< シングルステップ
    CONTINUE,            //!< 実行
    RUN,                 //!< 実行
    RESET,               //!< レジスタの初期化
    HELP,                //!< コマンドのヘルプ
    QUIT,                //!< デバッガの終了
};
/**
 * @brief コマンドパラメタ
 */
enum class CmdParam {
    NO_PARAM,  //!< パラメタなし
    NUM1,      //!< 数値パラメタ
    OPT_NUM1,  //!< 数値パラメタ
};
/**
 * @brief コマンド定義
 *
 */
struct CmdDef {
    const char* cmd_name;         //!< コマンド名
    const char* cmd_fullname;     //!< コマンドフル名
    const char* cmd_description;  //!< コマンドフル名

    CmdId cmd_id;        //!< コマンドID
    CmdParam cmd_param;  //!< コマンドパラメタ属性
};
/**
 * @brief カラーエスケープシーケンスクラス
 */
struct ColorChar {
    char escape[6] = "\033[00m";          //!< 色指定のエスケープシーケンス
    const char* reset_escap = "\033[0m";  //!< リセットエスケープシーケンス
    cmn::Color color;                     //!< 色
    const char* s;                        //!< 出力文字列
    /**
     * @brief Construct a new Color Char object
     *
     * @param color 色
     * @param s 出力文字列
     */
    ColorChar(cmn::Color color, const char* s = nullptr) : color(color), s(s) {
        int n = static_cast<int>(color);
        escape[3] = (n / 10) + '0';
        escape[4] = (n % 10) + '0';
    }
};
/**
 * @brief カラーエスケープシーケンス出力ヘルパー関数
 *
 * @param os 出力ストリーム
 * @param cb 色情報
 * @return std::ostream&
 */
inline std::ostream& operator<<(std::ostream& os, ColorChar cb) {
    os << cb.escape << cb.color;
    // 出力文字列が指定されていないときは、色のリセットを行う
    if (cb.s != nullptr) os << cb.s << cb.reset_escap;
    return os;
}
/**
 * @brief デバッガクラス
 *
 */
class Debugger {
    CometII& cii_cpu;          //!< コメットCPU
    ass::DbgInfos& dbg_infos;  //!< デバッグソース情報
    const cii::AssmMem& mem;

   public:
    Debugger(CometII& cii_cpu, ass::DbgInfos& dbg_infos, cii::AssmMem& mem)
        : cii_cpu(cii_cpu), dbg_infos(dbg_infos), mem(mem) {}

    /**
     * @brief デバッガ開始
     */
    void Start();

   private:
    /**
     * @brief 一行文字列を読み取り、デバッガコマンドとしてチェックする
     * @param cmd_string    コマンド文字列
     * @param cmd_def       コマンド定義
     * @param params        コマンドパラメタ
     * @return true         OK
     *         false        NG
     */
    static bool ParseCmd(std::string& cmd_string, CmdDef& cmd_def, std::vector<std::string>& params);
    /**
     * @brief コマンドヘルプを表示する
     *
     */
    static void DisplayHelp();
    /**
     * @brief 実行
     */
    void Run();
    /**
     * @brief 複数のブレークポイントが正しいかどうかチェックし、正しい場合ブレークポイントを設定する
     * @param params ブレークポイント文字列
     */
    bool SetBreakPoint(const std::vector<std::string>& params);
    /**
     * @brief ブレークポイントを設定する
     * @param point ブレークポイントの位置
     */
    bool SetBreakPoint(uint16_t point);
    /**
     * @brief 全レジスタを表示する
     */
    void DisplayRegs() const;
    /**
     * @brief パラメタをチェックし、パラメタの位置のソースリストを表示する
     * @param params パラメタ
     */
    void DisplaySrc(std::vector<std::string>& params) const;
    /**
     * @brief ソースリストを表示する
     * @param start 表示開始位置
     * @param end 表示終了位置
     * @param opt オプション
     */
    void DisplaySrc(uint16_t start, uint16_t end = -1, bool opt = false) const;

    void DisplayMacro(int start, ass::TokenId token_id, const ass::DbgInfo& dbg_info) const;
    void DisplayContents(int start, uint16_t offset, std::string line, int no, uint16_t data1, uint16_t data2) const;
    /**
     * @brief シングルスッテップ
     *
     */
    void SingleStep();
    /**
     * @brief Set the Single Step object
     *
     */
    void SetSingleStep();
    /**
     * @brief レジスタを保存する
     *
     */
    void SaveRegs();
    /**
     * @brief 指定されたレジスタを表示する
     *
     * @param cmd_id レジスタ番号
     */
    void DisplayReg(CmdId cmd_id) const;
    /**
     * @brief レジスタを表示する。前回表示した値と異なる場合は表示色を変更する
     *
     * @param reg_name レジスタ名
     * @param reg レジスタ値
     * @param pre_reg レジスタ値の前回値
     */
    void DisplayOneReg(const char* reg_name, uint16_t reg, uint16_t pre_reg) const;
    /**
     * @brief フラグレジスタを表示する。前回値と異なる場合は表示色を変更する
     *
     * @param reg_name レジスタ名
     * @param flag レジスタフラグ値
     * @param pre_flag レジスタフラグ値の前回値
     */
    void DisplayOneReg(const char* reg_name, bool flag, bool pre_flag) const;
    /**
     * @brief ソースリストの一行を表示する
     *
     * @param dbg_info ソースリスト情報
     */
    int DisplayLine(const ass::DbgInfo dbg_info) const;
    /**
     * @brief 入力されたparamが数値かラベルかチェックし数値に変換する
     *
     * @param param 入力パラメタ
     * @param addr 入力パラメタを数値に変換または、ラベル値のアドレス値
     * @return true 成功
     * @return false 失敗
     */
    bool CheckAddr(std::string param, uint16_t& addr) const;
    /**
     * @brief ブレークポイントをクリアする
     *
     * @param params ブレークポイント
     */
    void ClearBreakPoints(std::vector<std::string>& params);
    /**
     * @brief 1個のブレークポイントを削除する
     *
     * @param point ブレークポイント
     */
    void ClearBreakPoint(uint16_t point);
    /**
     * @brief すべてのブレークポイントを削除する
     *
     */
    void ClearAllBreakPoints();
};

}  // namespace cii

#endif