# 小学四则运算题目生成器（Myapp）

结对项目 · 成员：翁佳华、廖颖欣
GitHub：https://github.com/Huahetai-cell/Pair-project

命令行程序，自动生成小学四则运算题目（自然数 / 真分数 / 带分数，±×÷，括号，每题运算符 ≤ 3），
并能对已有题目与答案进行判分统计。

## 功能

- 出题：`Myapp.exe -n <题目数> -r <数值范围>`
  - `-n`：题目个数（可选，默认 10）
  - `-r`：**必填**，数值范围 `[0, r)`；缺失则报错并打印帮助
- 判分：`Myapp.exe -e <题目文件> -a <答案文件>`
- 性能基准：`Myapp.exe -b -r <数值范围> [-n <题目数>]`
  - 内置计时三个热点阶段（构造树 / 规范化 / 格式化），输出 `Benchmark.txt`；
  - 再用 `python perf_visualize.py` 生成 `perf_chart.svg` 性能分析图（零依赖，无需 matplotlib）。
- 输出文件（写到程序当前目录）：
  - `Exercises.txt`：题目，每行形如 `1 + 2 + 3 =`
  - `Answers.txt`：对应答案（真分数 `3/5`，带分数 `2’3/8`）
  - `Grade.txt`：`Correct: k (...)` / `Wrong: m (...)`

## 约束（均已实现）

1. 计算过程不产生负数（减法 `e1 ≥ e2`）。
2. 除法结果恒为真分数（`< 1`）。
3. 每题运算符个数不超过 3 个。
4. 生成的题目互不重复：在 **交换律（+×）与同层子节点排序** 下判定等价，不跨层展平。
   - `1+2+3` 与 `3+(2+1)` 视为同一题；`3+2+1` 是不同题。
5. 支持一次生成一万道题（`unordered_set` 去重，单题 O(1)）。
6. 真分数分隔符为 `’`（U+2019），解析时兼容 ASCII `'`。

## 编译

### Visual Studio（推荐，作业语境）
用 VS 打开本目录的 `CMakeLists.txt`（或把 `src/` 下文件新建为 `Myapp` 工程），
以 **x64 + Release** 生成，产物为 `Myapp.exe`。已加 `/utf-8`，保证 `÷ × − ’` 正确输出。

### CMake 命令行
```bash
cmake -S . -B build
cmake --build build --config Release
# 产物：build/Release/Myapp.exe（或 build/Myapp）
```

## 用法示例

```bash
# 生成 10 道、范围 10 以内
Myapp.exe -n 10 -r 10

# 生成 10000 道、范围 100 以内
Myapp.exe -n 10000 -r 100

# 判分（与生成的文件或手造文件对比）
Myapp.exe -e Exercises.txt -a Answers.txt

# 性能基准（生成 Benchmark.txt，再用 perf_visualize.py 出图）
Myapp.exe -b -r 100 -n 200000
```

## 边界说明

- `-r 1` 合法：此时可用数值仅 `0`，程序不会崩溃（只会生成极少量题目）。
- `-r` 很小（如 2、3）且 `-n` 很大时，题目空间不足，程序输出它能产出的全部不重复题目并给出提示，
  **不会死循环**。
- 判分假设输入题目均规范、按行顺序编号。

## 目录结构

```
src/
  fraction.h      精确有理数（约分）
  expression.h    表达式树节点 / 运算符定义
  generator.h/.cpp 题目生成、约束构造、规范化去重、最小括号打印
  parser.h/.cpp   表达式解析与求值（用于判分）
  grader.h/.cpp   判分统计
  main.cpp        CLI 参数解析与模式分发
CMakeLists.txt
design.md         设计实现报告（含 PSP 表、流程图、测试用例）
```
