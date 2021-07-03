#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <memory>
#include <string>

namespace cmn {
/**
 * @brief ANSIエスケープシーケンス
 * 文字色番号の定義
 */
enum class Color {
    RESET,
    LIGHT = 1,
    // 文字色
    F_BLACK = 30,
    F_RED,
    F_GREEN,
    F_YELLOW,
    F_BLUE,
    F_MAGENTA,
    F_CYAN,
    F_WHITE,
    F_BRIGHT_BLACK = 90,
    F_BRIGHT_RED,
    F_BRIGHT_GREEN,
    F_BRIGHT_YELLOW,
    F_BRIGHT_BLUE,
    F_BRIGHT_MAGENTA,
    F_BRIGHT_CYAN,
    F_BRIGHT_WHITE,
    // 背景色
    B_BLACK = 40,
    B_RED,
    B_GREEN,
    B_YELLOW,
    B_BLUE,
    B_MAGENTA,
    B_SYAN,
    B_WHITE,
    B_BRIGHT_BLACK = 100,
    B_BRIGHT_RED,
    B_BRIGHT_GREEN,
    B_BRIGHT_YELLOW,
    B_BRIGHT_BLUE,
    B_BRIGHT_MAGENTA,
    B_BRIGHT_CYAN,
    B_BRIGHT_WHITE,
};

/**
 * @brief 文字列フォーマット
 *
 * @tparam Args
 * @param fmt 文字列フォーマット
 * @param args パラメタ
 * @return std::string フォーマット後の文字列
 */
template <typename... Args>
std::string Format(const std::string& fmt, Args... args) {
    size_t len = std::snprintf(nullptr, 0, fmt.c_str(), args...);
    std::unique_ptr<char[]> buf(new char[len + 1]);
    std::snprintf(buf.get(), len + 1, fmt.c_str(), args...);
    return std::string(buf.get(), buf.get() + len);
}
/**
 * @brief ANSIエスケープシーケンス 色設定の出力
 *
 * @param os 出力ストリーム
 * @param c 色
 * @return std::ostream& 出力ストリーム
 */
inline std::ostream& operator<<(std::ostream& os, cmn::Color c) {
    os << cmn::Format("\033[%dm", static_cast<int>(c));
    return os;
}
/**
 * @brief 標準出力
 *
 */
static std::ostream& C = std::cout;

}  // namespace cmn

#endif
