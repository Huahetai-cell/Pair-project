#pragma once
#include "expression.h"
#include <random>
#include <string>
#include <vector>
#include <unordered_set>

struct Problem {
    std::string question;  // 形如 "1 + 2 + 3 ="
    std::string answer;    // 已格式化的答案
    std::string key;       // 规范化去重 key
};

// 题目生成器：负责随机构造满足约束的表达式树、规范化去重、最小括号打印
class Generator {
public:
    Generator(long long range, unsigned int seed);

    // 生成至多 n 道不重复题目；返回实际生成数量（空间不足时可能 < n）
    size_t generate(size_t n, std::vector<Problem>& out);

    // 性能分析：开启后 generate() 会累计三个阶段（构造树 / 规范化 / 格式化）的耗时（微秒）
    void enableTiming(bool on) { timing_ = on; }
    void getTiming(long long& build, long long& canon, long long& str) const {
        build = tBuild_; canon = tCanon_; str = tStr_;
    }

private:
    long long range_;
    std::mt19937 rng_;
    bool timing_ = false;
    long long tBuild_ = 0, tCanon_ = 0, tStr_ = 0;  // 累计耗时（微秒）

    Fraction randomOperand();                       // 按 -r 约束随机生成一个操作数
    Op pickOp();                                    // 随机选一个运算符
    NodePtr buildTree(int opsRemaining);             // 递归构造（约束不满足时返回 nullptr，由上层重试）
    std::string canonical(const NodePtr& n);         // 递归规范化 -> 去重 key
    std::string toStr(const NodePtr& n, bool isRight); // 最小括号打印
    bool needParen(const NodePtr& child, Op parentOp, bool isRight) const;
    static char opChar(Op o);                        // key 用的 ASCII 运算符字符
    static std::string opSymbol(Op o);               // 显示用的 Unicode 运算符
    static int prec(Op o);                           // 优先级：×÷ > +−
};
