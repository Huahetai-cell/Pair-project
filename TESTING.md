# 本地测试指南（手把手）

本指南面向"第一次在本机跑这个项目"的同学，按步骤走就能验证程序是否正确。

---

## 一、准备环境

你需要一个能编译 C++17 + CMake 的工具链，二选一：

### 方案 A：Visual Studio（最推荐，作业语境就是 Windows）
1. 下载安装 **Visual Studio 2022 社区版**（免费）：https://visualstudio.microsoft.com/zh-hans/downloads/
2. 安装时勾选 **"使用 C++ 的桌面开发"** 工作负载（会自动带上 MSVC、CMake、Windows SDK）。
3. 安装完即可，不需要额外配置。

### 方案 B：只装构建工具（轻量）
1. 安装 **Visual Studio Build Tools 2022**，同样勾选 "使用 C++ 的桌面开发"。
2. 安装 **CMake**（https://cmake.org/download/，下 Windows x64 安装包，勾选加入 PATH）。
3. 打开"开始菜单 → x64 Native Tools Command Prompt for VS 2022"（这是带编译环境的命令行）。

> 验证是否装好：在命令行输入 `cmake --version` 和 `cl`（或 `gcc --version`/`g++ --version`），能打印版本号即 OK。

---

## 二、拿到代码

把仓库克隆到本地，或把本目录整体复制下来：

```bash
git clone https://github.com/Huahetai-cell/Pair-project.git
cd Pair-project
```

---

## 三、编译（生成 Myapp.exe）

### 用 VS 打开（图形界面，最简单）
1. 打开 VS → "打开本地文件夹"，选择本项目根目录（含 `CMakeLists.txt` 的那个）。
2. 顶部工具栏把配置改成 **x64 + Release**（务必 Release，Debug 会慢很多）。
3. 菜单 "生成 → 生成全部" / 或按 `Ctrl+Shift+B`。
4. 产物在 `out/build/x64-Release/Myapp.exe`（VS 打开文件夹模式），或在你新建工程时的输出目录。

### 用 CMake 命令行（更通用）
在"x64 Native Tools Command Prompt"里：

```bash
cmake -S . -B build
cmake --build build --config Release
```

产物位置：
- 用 VS 生成：`out/build/x64-Release/Myapp.exe`
- 用上面的命令：`build/Release/Myapp.exe`

> 找不到 exe？在资源管理器里搜索 `Myapp.exe` 即可。

---

## 四、运行与验证（重点）

**关键原则：所有命令都要在 `Myapp.exe` 所在的目录里运行**，因为输出文件（`Exercises.txt` 等）是写到"当前目录"的。

打开命令行，`cd` 到 exe 所在目录，然后一条条试：

### 1) 缺参数应报错（验收点 1）
```bash
Myapp.exe -n 10
```
期望：屏幕打印帮助信息并退出（不生成任何文件）。这证明 `-r` 是必填的。

### 2) 正常出题（最核心）
```bash
Myapp.exe -n 10 -r 10
```
期望：当前目录多出 `Exercises.txt` 和 `Answers.txt`，各 10 行。
- `Exercises.txt` 每行形如 `1 + 2 + 3 =`
- `Answers.txt` 每行是答案，真分数如 `3/5`，带分数如 `2’3/8`

用记事本打开核对，重点看：
- 有没有出现负数（减法结果不会是负的）；
- 除法的结果是不是都是真分数（&lt; 1，即没有"整数除以整数得整数"的情况）。

### 3) 验收点：分数精确运算
自己手造一个最小题集验证 `1/6 + 1/8 = 7/24`：
在 `Exercises.txt` 里临时写一行 `1/6 + 1/8 =`，对应的 `Answers.txt` 写 `7/24`，
然后跑判分（见下），看它是否判为 Correct。

### 4) 判分（验收点：批改）
先造一对"故意有对有错"的文件来测判分逻辑：

`Exercises.txt`（示例）：
```
1 + 2 =
3 ÷ 2 =
4 - 1 =
2 × 3 =
```
`Answers.txt`（示例，第 2 题故意写错）：
```
3
2
3
5
```
运行：
```bash
Myapp.exe -e Exercises.txt -a Answers.txt
```
期望：生成 `Grade.txt`，内容类似：
```
Correct: 3 (1, 3, 4)
Wrong: 1 (2)
```
（第 2 题 `3 ÷ 2 = 1’1/2`，我们写的答案是 `2`，所以进 Wrong。）

### 5) 自检闭环（验收点 10）
直接用程序自己生成的文件去判分，应该全对：
```bash
Myapp.exe -n 100 -r 10
Myapp.exe -e Exercises.txt -a Answers.txt
```
期望：`Grade.txt` 里 `Correct: 100 (1, 2, …, 100)`，`Wrong: 0 ()`。
这证明"生成的题目 → 解析求值 → 答案"能无损往返，是正确性的最强证据。

### 6) 压力/边界测试（附加分相关）
```bash
Myapp.exe -r 100 -n 10000      # 一万道题，Release 下应 < 1 秒
Myapp.exe -r 1 -n 5            # 极小范围，程序不应崩溃
Myapp.exe -r 2 -n 10000        # 空间不足，应优雅停止并提示，不死循环
```

---

## 五、常见坑

| 现象 | 原因 / 解决 |
|---|---|
| 输出中文/符号 `÷ × − ’` 乱码 | 确保用 **Release + /utf-8**（CMakeLists 已加）。若仍乱码，把终端字体设为支持中文的字体，或用记事本打开文件看（文件本身是对的）。 |
| 提示"不是内部或外部命令" | 没 `cd` 到 exe 目录，或 exe 没生成成功。先去目录里确认 `Myapp.exe` 存在。 |
| `cmake` 找不到编译器 | 没开 "x64 Native Tools Command Prompt"，而是普通 cmd。用开始菜单里的 VS 专用命令行。 |
| 判分结果数量不对 | `Exercises.txt` 和 `Answers.txt` 行数必须一致；题目需按行顺序编号、格式规范。 |

---

## 六、给评测老师的一句话

> 用 x64 + Release 编译出 `Myapp.exe` 后，在当前目录依次运行
> `Myapp.exe -n 10 -r 10` 与 `Myapp.exe -e Exercises.txt -a Answers.txt`，
> 即可分别得到题目、答案与批改结果三个文件。
