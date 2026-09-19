#pragma once
#include "fraction.h"
#include <string>
#include <vector>

// 表达式解析与求值：用于判分时重新计算每道题的正确答案。
// 支持整数、真分数 "n/d"、带分数 "a'b/c"，以及 + − × ÷ 与括号，
// 优先级 ×÷ > +−，左结合。运算符前后允许空格，与出题格式一致。
namespace parser {

// 解析一行表达式（到 '=' 为止），返回计算得到的精确 Fraction
Fraction evaluate(const std::string& exprLine);

}  // namespace parser
