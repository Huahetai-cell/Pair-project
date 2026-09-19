#include "generator.h"
#include <algorithm>

Generator::Generator(long long range, unsigned int seed) : range_(range), rng_(seed) {}

Op Generator::pickOp() {
    std::uniform_int_distribution<int> d(0, 3);
    switch (d(rng_)) {
        case 0: return Op::Add;
        case 1: return Op::Sub;
        case 2: return Op::Mul;
        default: return Op::Div;
    }
}

// 按 -r 约束随机生成操作数：
//  - 自然数：0 .. r-1
//  - 真分数/带分数：a + n/d，分量 a, n, d 均 < r
Fraction Generator::randomOperand() {
    if (range_ <= 1) return Fraction(0);            // -r 1 合法：只有 0
    if (range_ < 3) {                              // 分母需 >=2，故 r<3 时只能是整数
        std::uniform_int_distribution<long long> di(0, range_ - 1);
        return Fraction(di(rng_));
    }
    std::uniform_int_distribution<int> t(0, 1);
    if (t(rng_) == 0) {
        std::uniform_int_distribution<long long> di(0, range_ - 1);
        return Fraction(di(rng_));
    }
    std::uniform_int_distribution<long long> da(0, range_ - 1); // 整数部分 a < r
    std::uniform_int_distribution<long long> dd(2, range_ - 1);  // 分母 d in [2, r)
    long long a = da(rng_);
    long long d = dd(rng_);
    std::uniform_int_distribution<long long> dn(1, d - 1);        // 分子 n in [1, d)
    long long n = dn(rng_);
    return Fraction(a * d + n, d);
}

// 递归构造表达式树，opsRemaining = 本子树（含自身）还能放置的运算符个数
// 约束：减法 minuend >= subtrahend；除法 divisor > dividend 且不为 0（结果恒为真分数）
// 任一约束无法满足则返回 nullptr，由上层重试整道题
NodePtr Generator::buildTree(int opsRemaining) {
    if (opsRemaining <= 0) {
        auto leaf = std::make_shared<Node>();
        leaf->isLeaf = true;
        leaf->value = randomOperand();
        return leaf;
    }
    Op op = pickOp();
    int budget = opsRemaining - 1;
    std::uniform_int_distribution<int> db(0, budget);
    int lb = db(rng_);
    int rb = budget - lb;
    NodePtr left = buildTree(lb);
    NodePtr right = buildTree(rb);
    if (!left || !right) return nullptr;

    Fraction lv = left->value;
    Fraction rv = right->value;

    if (op == Op::Add) {
        // 无约束
    } else if (op == Op::Sub) {
        // 减法出负数 -> 交换左右，使 minuend >= subtrahend
        if (lv < rv) {
            std::swap(left, right);
            lv = left->value;
            rv = right->value;
        }
    } else if (op == Op::Mul) {
        // 无约束（操作数非负，积非负）
    } else { // Div
        // 要求 divisor(rv) > dividend(lv) 且均不为 0，结果方为真分数 (<1)
        if (!lv.isZero() && (rv.isZero() || rv <= lv)) {
            bool ok = false;
            for (int i = 0; i < 80; ++i) {
                right = buildTree(rb);
                if (!right) return nullptr;
                rv = right->value;
                if (!rv.isZero() && rv > lv) { ok = true; break; }
            }
            if (!ok) return nullptr;
        } else if (lv.isZero()) {
            return nullptr;  // dividend 为 0 无法得到真分数，放弃本题
        }
    }

    Fraction v;
    switch (op) {
        case Op::Add: v = lv + rv; break;
        case Op::Sub: v = lv - rv; break;
        case Op::Mul: v = lv * rv; break;
        case Op::Div: v = lv / rv; break;
    }

    auto node = std::make_shared<Node>();
    node->isLeaf = false;
    node->op = op;
    node->left = left;
    node->right = right;
    node->value = v;
    return node;
}

// 规范化：对 + / × 节点，把两个子树的规范化串按字典序排序（吸收交换律）；
// 对 - / ÷ 节点不交换（不满足交换律）。不做跨层展平——这与作业定义完全吻合：
//   1+2+3 与 3+(2+1) 同一 key；3+2+1 不同 key。
std::string Generator::canonical(const NodePtr& n) {
    if (n->isLeaf) return n->value.key();
    std::string l = canonical(n->left);
    std::string r = canonical(n->right);
    if ((n->op == Op::Add || n->op == Op::Mul) && l > r) std::swap(l, r);
    char c = opChar(n->op);
    return std::string(1, c) + "(" + l + "," + r + ")";
}

int Generator::prec(Op o) { return (o == Op::Mul || o == Op::Div) ? 2 : 1; }

// 是否需要在 child 外层加括号：依据优先级与左结合性
bool Generator::needParen(const NodePtr& child, Op parentOp, bool isRight) const {
    if (child->isLeaf) return false;
    int cp = prec(child->op), pp = prec(parentOp);
    if (cp < pp) return true;            // 子优先级低，必须加括号（如 (1+2)*3）
    if (cp == pp) return isRight;        // 同级：右子加括号以保留左结合结构，左子不加
    return false;                         // 子优先级高，无需括号（如 a + b*c）
}

// 最小括号打印：保证打印文本能按标准优先级/左结合完整还原回原树
std::string Generator::toStr(const NodePtr& n, bool isRight) {
    if (n->isLeaf) return n->value.toDisplay();
    std::string ls = toStr(n->left, false);
    std::string rs = toStr(n->right, true);
    if (needParen(n->left, n->op, false)) ls = "(" + ls + ")";
    if (needParen(n->right, n->op, true)) rs = "(" + rs + ")";
    return ls + " " + opSymbol(n->op) + " " + rs;
}

char Generator::opChar(Op o) {
    switch (o) {
        case Op::Add: return '+';
        case Op::Sub: return '-';
        case Op::Mul: return '*';
        case Op::Div: return '/';
    }
    return '?';
}

std::string Generator::opSymbol(Op o) {
    switch (o) {
        case Op::Add: return "+";
        case Op::Sub: return "\xe2\x88\x92";   // −  U+2212
        case Op::Mul: return "\xc3\x97";        // ×  U+00D7
        case Op::Div: return "\xc3\xb7";        // ÷  U+00F7
    }
    return "?";
}

size_t Generator::generate(size_t n, std::vector<Problem>& out) {
    out.clear();
    std::unordered_set<std::string> seen;
    size_t attempts = 0;
    size_t noProgress = 0;                 // 连续未新增题目的次数，用于空间不足时提前退出
    const size_t cap = n * 300 + 100000;
    std::uniform_int_distribution<int> topOps(1, 3);  // 运算符个数 1..3

    while (out.size() < n && attempts < cap && noProgress < 200000) {
        ++attempts;
        int t = topOps(rng_);
        NodePtr root = buildTree(t);
        if (!root) { ++noProgress; continue; }
        std::string k = canonical(root);
        if (seen.count(k)) { ++noProgress; continue; }
        seen.insert(k);
        noProgress = 0;
        Problem p;
        p.key = k;
        p.question = toStr(root, false) + " =";
        p.answer = root->value.toDisplay();
        out.push_back(p);
    }
    return out.size();
}
