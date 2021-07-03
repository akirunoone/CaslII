#ifndef READER_H_
#define READER_H_

#include <cctype>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace ass {

enum OperandType {
    REG_EADR,
    REG_MEM,
    REG_REGorMEM,
    //
    EADR,
    REG,
    ADR_ADR,
    DS_CONST,
    DC_CONST,
    NONE,
};

/**
 * TokenIdの分類Class
 */
enum TokenClass {
    TOKEN_CLASS_MASK = 0xff00,
    REG_CLASS = 0x0100,
    OPE_CLASS = 0x0200,
    ASEM_CLASS = 0x0300,
    MACRO_CLASS = 0x0400,
    COMMENT_CLASS = 0x0800,
};

enum class TokenId {
    GR0 = REG_CLASS,          //!< 汎用レジスタ 0
    GR1,                      //!< 汎用レジスタ 1
    GR2,                      //!< 汎用レジスタ 2
    GR3,                      //!< 汎用レジスタ 3
    GR4,                      //!< 汎用レジスタ 4
    GR5,                      //!< 汎用レジスタ 5
    GR6,                      //!< 汎用レジスタ 6
    GR7,                      //!< 汎用レジスタ 7
    ST = OPE_CLASS,           //!< ストア
    LD,                       //!< ロードレジスタ
    LAD,                      //!< ロードアドレス
    ADDA,                     //!< 算術加算
    ADDL,                     //!< 論理加算
    SUBA,                     //!< 算術減算
    SUBL,                     //!< 論理OpType(減算, )
    AND,                      //!< 論理積
    OR,                       //!< RI_LOGI | 2,    //! 排他的論理和
    XOR,                      //!< 排他的論理和
    CPA,                      //!< 算術比較
    CPL,                      //!< 論理比較
    SLA,                      //!< 算術左シフト
    SRA,                      //!< 算術右シフト
    SLL,                      //!< 論理左シフト
    SRL,                      //!< 論理右シフト
    JPL,                      //!< 正分岐
    JMI,                      //!< 負分岐
    JNZ,                      //!< 非零分岐
    JZE,                      //!< 零分岐
    JOV,                      //!< オーバーフロー分岐
    JUMP,                     //!< 無条件分岐
    PUSH,                     //!< プッシュ
    POP,                      //!< ポップ
    CALL,                     //!< コール
    RET,                      //!< リターン
    SVC,                      //!< スーパーバイザコール
    NOP,                      //!< NOP
    HLT,                      //!< HALT
    START = ASEM_CLASS,       //!< START
    END,                      //!< END
    DC,                       //!< DC
    DS,                       //!< DS
    IN = MACRO_CLASS,         //!< IN
    OUT,                      //!< OUT
    COMMENT = COMMENT_CLASS,  //!< コメント
    LABEL,                    //!< ラベル
    COMMA,                    //!< カンマ
    DIGIT,                    //!< 数値
    CONST,                    //!< 定数
    STRING,                   //!< 文字列
    OTHER,                    //!< その他
    // SP_MACRO_EXPAND           //!< とりあえずのマクロの展開用
};

/**
 * TokenIdからTokenClassを取り出す
 * @param tid
 * @return TokenClass
 */
inline TokenClass GetTokenClass(TokenId tid) { return (TokenClass)((uint16_t)TOKEN_CLASS_MASK & (uint16_t)tid); }

struct TokenInfo {
    TokenId token_id;
    OperandType operand_type;
    uint16_t digit;
    std::string label;
    TokenInfo(TokenId id, OperandType ope_type = NONE) : token_id(id), operand_type(ope_type) {}
    TokenInfo(TokenId id, uint16_t digit) : token_id(id), digit(digit) {}
    TokenInfo(TokenId id, std::string label) : token_id(id), label(label) {}
    TokenInfo(TokenId id, uint16_t digit, std::string label) : token_id(id), digit(digit), label(label) {}
};

using Tokens = std::vector<TokenInfo>;

/**
 * リーダクラス
 * 一行の文字列を読込トークン情報に分割する
 */
class Reader {
   protected:
    //! 一行をトークンに分割するための正規表現
    const std::regex reg = std::regex(R"(\s*([^,;'\s]+|,|;|')\s*)");
    //! 解析した一行分のトークン
    Tokens tokenes;

   public:
    //! キーワードテーブル
    static std::map<std::string, TokenInfo> key_words;
    /**
     * 1行を解析しトークンを生成する
     * @param line パースする1行の文字列
     * @return パースしたトークンを返す
     */
    const Tokens& Parse(const std::string& line);

    /**
     * 文字列が10進数かどうかチェックする
     * @param s 文字列
     * @retval true 10進数文字列
     */
    static bool IsDigit(const std::string& s);
    /**
     * 文字列が16進数かどうかチェックする
     * @param s 文字列
     * @retval true 16進数文字列
     */
    static bool IsHex(const std::string& s);
    /**
     * 文字列がラベルかどうかチェックする
     * @param s 文字列
     * @retval true ラベル
     */
    static bool IsLabel(const std::string& s);
};
}  // namespace ass

#endif