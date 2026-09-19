#pragma once
#include <string>
#include <numeric>

// 精确有理数：用 long long 存分子/分母并自动约分，避免浮点误差。
// 所有参与运算的数值在本项目中均非负，故无需处理负号细节（除防御外）。
class Fraction {
public:
    long long num = 0;  // 分子（恒 >= 0）
    long long den = 1;  // 分母（恒 > 0）

    Fraction() = default;
    Fraction(long long n, long long d) { set(n, d); }
    explicit Fraction(long long n) { set(n, 1); }

    void set(long long n, long long d) {
        if (d == 0) d = 1;
        if (d < 0) { n = -n; d = -d; }
        long long g = std::gcd(n < 0 ? -n : n, d);
        if (g == 0) g = 1;
        num = n / g;
        den = d / g;
    }

    Fraction operator+(const Fraction& o) const { return Fraction(num * o.den + o.num * den, den * o.den); }
    Fraction operator-(const Fraction& o) const { return Fraction(num * o.den - o.num * den, den * o.den); }
    Fraction operator*(const Fraction& o) const { return Fraction(num * o.num, den * o.den); }
    Fraction operator/(const Fraction& o) const { return Fraction(num * o.den, den * o.num); }

    bool operator<(const Fraction& o) const { return num * o.den < o.num * den; }
    bool operator<=(const Fraction& o) const { return num * o.den <= o.num * den; }
    bool operator==(const Fraction& o) const { return num * o.den == o.num * den; }
    bool isZero() const { return num == 0; }

    // 规范化字符串：用"分子/分母"的既约假分数形式，供判重 key 使用（与显示格式无关）
    std::string key() const { return std::to_string(num) + "/" + std::to_string(den); }

    // 显示格式：整数 | 真分数 "n/d" | 带分数 "a'b/c"（' 为 U+2019）
    std::string toDisplay() const {
        if (num == 0) return "0";
        long long a = num / den;       // 整数部分
        long long r = num % den;       // 余数
        if (r == 0) return std::to_string(a);
        if (a == 0) return std::to_string(r) + "/" + std::to_string(den);
        // 带分数：U+2019 用原始 UTF-8 字节，规避编译器代码页问题
        return std::to_string(a) + "\xe2\x80\x99" + std::to_string(r) + "/" + std::to_string(den);
    }
};
