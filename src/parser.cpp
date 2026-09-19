#include "parser.h"
#include <cctype>
#include <functional>
#include <string>

namespace parser {

// 把显示用的 Unicode 运算符 / 撇号 归一为 ASCII，简化后续分词：
//   −(U+2212)->'-'   ×(U+00D7)->'*'   ÷(U+00F7)->'/'   ’(U+2019)->'''
static std::string normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0xE2 && i + 2 < s.size() &&
            static_cast<unsigned char>(s[i + 1]) == 0x88 &&
            static_cast<unsigned char>(s[i + 2]) == 0x92) { out += '-'; i += 2; }
        else if (c == 0xE2 && i + 2 < s.size() &&
                 static_cast<unsigned char>(s[i + 1]) == 0x80 &&
                 static_cast<unsigned char>(s[i + 2]) == 0x99) { out += '\''; i += 2; }
        else if (c == 0xC3 && i + 1 < s.size() &&
                 static_cast<unsigned char>(s[i + 1]) == 0x97) { out += '*'; i += 1; }
        else if (c == 0xC3 && i + 1 < s.size() &&
                 static_cast<unsigned char>(s[i + 1]) == 0xB7) { out += '/'; i += 1; }
        else out += static_cast<char>(c);
    }
    return out;
}

// 解析数值串："5" / "3/5" / "2'3/8" -> 精确 Fraction
static Fraction parseFractionStr(const std::string& s) {
    size_t p = s.find('\'');
    size_t q = s.find('/');
    if (p == std::string::npos && q == std::string::npos) return Fraction(std::stoll(s));
    long long a = 0;
    if (p != std::string::npos) a = std::stoll(s.substr(0, p));
    if (q != std::string::npos) {
        size_t ns = (p != std::string::npos) ? p + 1 : 0;
        long long n = std::stoll(s.substr(ns, q - ns));
        long long d = std::stoll(s.substr(q + 1));
        return Fraction(a * d + n, d);
    }
    return Fraction(a);
}

enum TokType { TNum, TAdd, TSub, TMul, TDiv, TLP, TRP, TEnd };

struct Tok {
    TokType t;
    Fraction v;
};

// 分词：数值串连续吃掉 digit / '/' / '''（因此分数内部的 '/' 不会被误判成除号）；
// 运算符前后有空格，所以孤立的 '/' 才是除法运算符。
static std::vector<Tok> tokenize(const std::string& s) {
    std::vector<Tok> toks;
    size_t i = 0, n = s.size();
    while (i < n) {
        char c = s[i];
        if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }
        if (c >= '0' && c <= '9') {
            size_t j = i;
            while (j < n && (std::isdigit(static_cast<unsigned char>(s[j])) || s[j] == '/' || s[j] == '\'')) ++j;
            Tok tk{ TNum, parseFractionStr(s.substr(i, j - i)) };
            toks.push_back(tk);
            i = j;
        } else if (c == '+') { toks.push_back({ TAdd, Fraction(0) }); ++i; }
        else if (c == '-') { toks.push_back({ TSub, Fraction(0) }); ++i; }
        else if (c == '*') { toks.push_back({ TMul, Fraction(0) }); ++i; }
        else if (c == '/') { toks.push_back({ TDiv, Fraction(0) }); ++i; }
        else if (c == '(') { toks.push_back({ TLP, Fraction(0) }); ++i; }
        else if (c == ')') { toks.push_back({ TRP, Fraction(0) }); ++i; }
        else { ++i; }  // 未知字符跳过
    }
    toks.push_back({ TEnd, Fraction(0) });
    return toks;
}

static Fraction evaluateTokens(const std::vector<Tok>& toks) {
    size_t pos = 0;
    auto peek = [&]() -> TokType { return toks[pos].t; };

    std::function<Fraction()> parseExpr, parseTerm, parseFactor;
    parseExpr = [&]() {
        Fraction v = parseTerm();
        while (peek() == TAdd || peek() == TSub) {
            TokType op = peek();
            ++pos;
            Fraction r = parseTerm();
            v = (op == TAdd) ? v + r : v - r;
        }
        return v;
    };
    parseTerm = [&]() {
        Fraction v = parseFactor();
        while (peek() == TMul || peek() == TDiv) {
            TokType op = peek();
            ++pos;
            Fraction r = parseFactor();
            v = (op == TMul) ? v * r : v / r;
        }
        return v;
    };
    parseFactor = [&]() {
        Tok t = toks[pos];
        if (t.t == TLP) {
            ++pos;
            Fraction v = parseExpr();
            if (peek() == TRP) ++pos;
            return v;
        }
        if (t.t == TNum) { ++pos; return t.v; }
        ++pos;
        return Fraction(0);
    };
    return parseExpr();
}

Fraction evaluate(const std::string& exprLine) {
    std::string s = normalize(exprLine);
    size_t eq = s.find('=');
    if (eq != std::string::npos) s = s.substr(0, eq);  // 只取 '=' 之前的表达式
    try {
        return evaluateTokens(tokenize(s));
    } catch (...) {
        return Fraction(0);
    }
}

}  // namespace parser
