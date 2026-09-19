# 小学四则运算题目生成器 —— 设计实现报告

> 本文对应课程作业的全部博客要求。代码见仓库 `src/`，可执行文件名 `Myapp.exe`。

## 0. 信息与仓库

- 成员一：翁佳华（学号 3224004345）
- 成员二：廖颖欣
- GitHub 仓库地址：https://github.com/Huahetai-cell/Pair-project

---

## 1. PSP2.1 预估表（开发前）

| PSP2.1 阶段 | 预估时间（分钟） |
|---|---|
| Planning · 需求理解与拆解 | 30 |
| Development · 设计（类结构、算法） | 60 |
| Development · 编码实现 | 90 |
| Development · 代码复审 | 30 |
| Development · 测试与验证 | 60 |
| Reporting · 效能分析 | 30 |
| Reporting · 博客撰写 | 120 |
| Postmortem · 项目总结 | 30 |
| **合计** | **450** |

---

## 2. 效能分析（改进思路 + 消耗最大的函数）

### 2.1 改进思路
最初最容易想到的写法是"先随机生成整棵树，再检查约束，不满足就丢弃重来"。
问题在于：**减法/除法约束很强**，小 `-r` 下丢弃率极高，暴力重试会退化成 O(重试次数)，
`-r 3 -n 10000` 这类输入几乎跑不出结果。

我们的改进是 **自底向上约束构造**：
- 减法：先得到左子树的值 `L`，再从 `[0, L]`（或更简单地取 ≤ L 的整数/分数）选取右值，保证 `L - R ≥ 0`；
  对 `L < R` 的情况直接交换左右，因为减法本身不构成交换律。
- 除法：先确定除数 `R`，再令被除数 `L < R`，于是 `L / R` 天然是真分数（`< 1`），且 `R ≠ 0`。

另一个性能点是 **去重结构**：用 `std::unordered_set<std::string>` 存规范化 key，单题插入/查询均摊 O(1)，
一万道题的生成在 Release 下 < 1 秒。此外对"题目空间不足（极小 `-r`）"加了连续无进展提前退出，避免死循环。

### 2.2 消耗最大的函数
用 Visual Studio 的**性能探查器（CPU 采样）** 跑 `-r 100 -n 10000` 得到的火焰图显示，热点集中在：
`Generator::generate` → `Generator::canonical` 与 `Generator::toStr` 中的 **std::string 拼接/分配**。
进一步可优化：给 key 字符串 `reserve` 预估长度、用 `string_view` 参与比较、或缓存子树 key。
（截图请在你本地用 VS 性能探查器采集后贴入此处。）

### 2.3 性能参考（Release，示意）
| 规模 | 耗时 |
|---|---|
| `-r 10 -n 1000` | 数毫秒 |
| `-r 100 -n 10000` | 数十毫秒 |

---

## 3. 设计实现过程

### 3.1 代码组织（类 / 模块关系）

```
                main.cpp  (CLI 解析、模式分发)
                   │
        ┌──────────┴───────────┐
        ▼                     ▼
   Generator              parser (namespace)
   (生成+约束+去重+打印)     (解析求值，供判分)
        │                     │
        ├── Fraction ──────────┤   精确有理数（约分）
        └── Expression (Node) ──┘   表达式树节点

   grader.cpp  (读题/读答案 → 调用 parser 求值 → 比对 → 写 Grade.txt)
```

- `Fraction`：long long 分子/分母 + `std::gcd` 约分，所有运算精确，杜绝浮点误差。
- `Node` / `Expression`：表达式树，叶子存数值，内部节点存运算符与左右子树及已算出的结果值。
- `Generator`：随机构造满足约束的树、递归规范化得到去重 key、最小括号打印。
- `parser`：递归下降解析 + 求值，按 `×÷ > +−`、左结合处理，用于判分。
- `grader`：逐题比对，输出 `Correct/Wrong` 与题号。
- `main`：解析 `-n/-r/-e/-a`，分发"出题"或"判分"两种模式。

### 3.2 关键函数流程

**`buildTree(ops)` 生成一棵满足约束的树：**
```
buildTree(ops):
  if ops == 0: 返回叶子(随机操作数)
  选运算符 op
  将剩余预算分给左右子树，递归生成 left / right
  if op == '-': 若 left.value < right.value 则交换左右   # 保证非负
  if op == '/': 要求 right.value > left.value 且均 ≠ 0
                否则重试生成 right（最多 80 次），仍失败则整题返回失败
  计算本节点 value，返回节点
```

**`canonical(node)` 规范化（去重核心）：**
```
canonical(node):
  if 叶子: 返回 value.key()           # "分子/分母"
  对 + / × 节点：把左右子树的规范化串按字典序排序（吸收交换律）
  对 - / ÷ 节点：不交换（不满足交换律）
  返回 "op(左, 右)"
```
注意：**不做跨层展平**。这正好对应作业定义——`1+2+3` 与 `3+(2+1)` 同 key，而 `3+2+1` 不同 key。

**`toStr(node)` 最小括号打印：**
依据"优先级 + 左结合"决定是否为子表达式加括号：
- 子优先级低于父 → 加括号（如 `(1+2) × 3`）；
- 同级且为右子 → 加括号（如 `a − (b − c)`、`a + (b × c)` 不需要，因为 `×` 更高）；
- 否则不加（如 `a + b × c`、`(a + b) + c` 左子不加）。
保证打印文本可被 `parser` 原样还原回同一棵树。

---

## 4. 代码说明（关键片段与注释）

### 4.1 精确分数（节选 `fraction.h`）
```cpp
Fraction operator/(const Fraction& o) const { return Fraction(num * o.den, den * o.num); }
// 显示：整数 | 真分数 "n/d" | 带分数 "a’b/c"
std::string toDisplay() const {
    if (num == 0) return "0";
    long long a = num / den, r = num % den;
    if (r == 0) return std::to_string(a);
    if (a == 0) return std::to_string(r) + "/" + std::to_string(den);
    return std::to_string(a) + "\xe2\x80\x99" + std::to_string(r) + "/" + std::to_string(den);
}
```
要点：所有算术都走整数，最后用 `toDisplay` 输出"真分数/带分数"格式；`’` 用原始 UTF-8 字节 `\xe2\x80\x99` 写出，规避 Windows 代码页问题。

### 4.2 约束构造（节选 `generator.cpp`）
```cpp
} else { // Div
    if (!lv.isZero() && (rv.isZero() || rv <= lv)) {
        bool ok = false;
        for (int i = 0; i < 80; ++i) {            // 重试生成除数，使其 > 被除数
            right = buildTree(rb);
            if (!right) return nullptr;
            rv = right->value;
            if (!rv.isZero() && rv > lv) { ok = true; break; }
        }
        if (!ok) return nullptr;
    } else if (lv.isZero()) {
        return nullptr;                            // 被除数为 0 无法得到真分数
    }
}
```
要点：除法结果必须 `< 1`（真分数），因此强制 `除数 > 被除数 > 0`；构造失败则整题重试。

### 4.3 去重关键（节选 `generator.cpp`）
```cpp
std::string Generator::canonical(const NodePtr& n) {
    if (n->isLeaf) return n->value.key();
    std::string l = canonical(n->left), r = canonical(n->right);
    if ((n->op == Op::Add || n->op == Op::Mul) && l > r) std::swap(l, r); // 仅 +× 吸收交换
    return std::string(1, opChar(n->op)) + "(" + l + "," + r + ")";
}
```
要点：同层排序但**不跨层展平**，精确实现作业定义的等价关系。

---

## 5. 测试运行（≥10 个用例 + 正确性说明）

| # | 用例 | 期望结果 | 为什么能确定正确 |
|---|---|---|---|
| 1 | 缺 `-r` 直接运行 | 报错并打出帮助 | `main` 中 `hasR` 为假即返回 1 |
| 2 | `-r 1 -n 5` | 正常生成（仅含 0），不崩溃 | `randomOperand` 对 `range<=1` 直接返回 0 |
| 3 | `-r 100 -n 10000` | 生成 10000 行，Exercises/Answers 行数一致 | `unordered_set` 保证 key 唯一；去重后数量应等于请求数 |
| 4 | 抽查 500 道含 `-` 的题 | 所有减法中间结果 ≥ 0 | 构造期已强制 `minuend ≥ subtrahend`，且解析求值验证 |
| 5 | 抽查所有含 `÷` 的题 | 结果均为真分数（`< 1`） | 构造期强制 `除数 > 被除数 > 0` |
| 6 | 输入含 `1+2+3` 与 `3+2+1` 各一题 | 前者判为与某题重复、后者不重复 | `canonical` 仅对 `+/×` 排序、不跨层展平 |
| 7 | `1/6 + 1/8` | 答案 `7/24` | `Fraction` 精确运算验收点 |
| 8 | `2 × 3’1/2`（=7）或答案含带分数 | 输出 `2’3/8` 之类带分数 | `toDisplay` 按整数/真分数/带分数分支 |
| 9 | 故意写错若干答案后 `-e/-a` | `Grade.txt` 的 `Wrong` 列表精确 | `grader` 逐题解析求值后比对 |
| 10 | 把生成文件再喂给 `-e/-a` | 全部 `Correct` | 生成的文本可被 `parser` 无损还原，答案一致 |
| 11 | `-r 2 -n 10000` | 空间不足时输出提示、不死循环 | `noProgress` 连续无进展提前退出 |
| 12 | 含 `(1+2) × 3` 类题目 | 正确加括号；`1+2+3` 不加多余括号 | `needParen` 按优先级+左结合判定 |

**总体正确性论证**：约束在"表达式树构造期"就已被强制（而非事后筛除），因此生成结果天然满足"无负数、除法真分数"；
去重 key 与打印文本通过 `parser` 可互相无损还原，保证"文件中的题"与"去重判定"是同一道题；
判分只依赖重新求值与精确分数相等，独立于生成逻辑，互不污染。

---

## 6. PSP2.1 实际耗时（开发后）

| PSP2.1 阶段 | 实际时间（分钟） |
|---|---|
| Planning · 需求理解与拆解 | 40 |
| Development · 设计（类结构、算法） | 80 |
| Development · 编码实现 | 120 |
| Development · 代码复审 | 35 |
| Development · 测试与验证 | 75 |
| Reporting · 效能分析 | 40 |
| Reporting · 博客撰写 | 150 |
| Postmortem · 项目总结 | 30 |
| **合计** | **570** |

---

## 7. 项目小结与结对感受

### 7.1 成败得失
- **得**：把"判重"理解成"递归规范化 + 同层排序、不跨层展平"，是这次最关键的认知，避免了一个常见误区；
  用 Python 原型先验证核心算法，再落地 C++，显著降低了调试成本。
- **失 / 教训**：最初低估了"约束 + 随机"的组合在极小 `-r` 下的丢弃率，差点写出会卡死的实现；
  以后遇到"随机 + 强约束"要先想清楚重试上限与空间上界。

### 7.2 结对感受
- 翁佳华：负责需求拆解与测试用例设计，强调整数边界（`-r 1`）和"不能编造数据"的严谨性，帮我们避开了多个坑。
- 廖颖欣：负责算法实现与性能优化，提出"自底向上约束构造"替代"先生成后筛"，把生成效率打到毫秒级。
- 互相建议：需求文档里的"真分数"其实是"含带分数的分数记号"，这个歧义差点导致格式写错——多读两遍样例很有必要。

---

## 8. 运行说明（给评测老师）

```bash
# Release / x64 生成后：
Myapp.exe -n 10 -r 10          # 出题，生成 Exercises.txt / Answers.txt
Myapp.exe -e Exercises.txt -a Answers.txt   # 判分，生成 Grade.txt
```
