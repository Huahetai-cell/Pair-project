#pragma once
#include "fraction.h"
#include <memory>
#include <string>

// 运算符枚举
enum class Op { Add, Sub, Mul, Div };

// 表达式树节点：叶子存数值，内部节点存运算符与左右子树
struct Node {
    bool isLeaf = true;
    Op op = Op::Add;
    Fraction value;                       // 该子树的计算结果（构造时自底向上算出）
    std::shared_ptr<Node> left, right;
};

using NodePtr = std::shared_ptr<Node>;
