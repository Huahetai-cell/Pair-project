#include "generator.h"
#include "grader.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

static void printHelp() {
    std::cout << "Myapp - 小学四则运算题目生成与判分\n";
    std::cout << "出题: Myapp.exe -n <题目数> -r <数值范围>\n";
    std::cout << "  -n  题目个数（可选，默认 10）\n";
    std::cout << "  -r  数值范围 [0, r)，必填；缺失则报错\n";
    std::cout << "判分: Myapp.exe -e <题目文件> -a <答案文件>\n";
    std::cout << "性能: Myapp.exe -b -r <数值范围> [-n <题目数>]\n";
    std::cout << "      （内置计时，输出 Benchmark.txt 供画图）\n";
}

// 控制台文本柱状图：展示三个阶段耗时占比，并指出消耗最大的函数
static void printBench(size_t got, double total_ms,
                       double build_ms, double canon_ms, double str_ms) {
    double pctB = total_ms > 0 ? build_ms / total_ms * 100 : 0;
    double pctC = total_ms > 0 ? canon_ms / total_ms * 100 : 0;
    double pctS = total_ms > 0 ? str_ms  / total_ms * 100 : 0;
    auto bar = [](double pct) {
        int n = static_cast<int>(pct + 0.5);          // 每 1% 一个 '#'
        if (n < 1 && pct > 0) n = 1;
        if (n > 60) n = 60;
        return std::string(n, '#');
    };
    std::cout << "\n性能分析（累计耗时，单位 ms，共 " << got << " 题）:\n";
    std::cout << "  构造表达式树 buildTree : " << bar(pctB) << "  "
              << std::fixed << std::setprecision(3) << build_ms << " (" << std::setprecision(1) << pctB << "%)\n";
    std::cout << "  规范化去重   canonical : " << bar(pctC) << "  "
              << std::setprecision(3) << canon_ms << " (" << std::setprecision(1) << pctC << "%)\n";
    std::cout << "  格式化输出   toStr     : " << bar(pctS) << "  "
              << std::setprecision(3) << str_ms << " (" << std::setprecision(1) << pctS << "%)\n";
    std::cout << "  --------------------------------------------\n";
    std::cout << "  总计                      " << std::setprecision(3) << total_ms << " ms\n";

    const char* hot = (build_ms >= canon_ms && build_ms >= str_ms) ? "buildTree（构造表达式树）"
                    : (canon_ms >= str_ms) ? "canonical（规范化去重）"
                                           : "toStr（格式化输出 / 最小括号打印）";
    std::cout << "消耗最大的函数阶段: " << hot << "\n";
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    bool hasN = false, hasR = false, hasE = false, hasA = false, hasB = false;
    long long n = 10, r = -1;
    std::string eFile, aFile;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-n" && i + 1 < args.size()) { n = std::stoll(args[++i]); hasN = true; }
        else if (a == "-r" && i + 1 < args.size()) { r = std::stoll(args[++i]); hasR = true; }
        else if (a == "-e" && i + 1 < args.size()) { eFile = args[++i]; hasE = true; }
        else if (a == "-a" && i + 1 < args.size()) { aFile = args[++i]; hasA = true; }
        else if (a == "-b") { hasB = true; }
        else if (a == "-h" || a == "--help" || a == "-?") { printHelp(); return 0; }
    }

    // 判分模式
    if (hasE && hasA) {
        grade(eFile, aFile, "Grade.txt");
        std::cout << "判分完成，结果已写入 Grade.txt\n";
        return 0;
    }

    // 性能基准模式（-b）
    if (hasB) {
        if (!hasR) {
            std::cerr << "错误：性能基准需要 -r（数值范围）。\n";
            printHelp();
            return 1;
        }
        if (r < 1) {
            std::cerr << "错误：-r 必须 >= 1。\n";
            printHelp();
            return 1;
        }
        if (!hasN) n = 10000;  // 未指定 -n 时默认一百万道，保证采样充分
        std::random_device rd;
        Generator gen(r, rd());
        gen.enableTiming(true);
        std::vector<Problem> probs;
        auto ts = std::chrono::high_resolution_clock::now();
        size_t got = gen.generate(static_cast<size_t>(n), probs);
        auto te = std::chrono::high_resolution_clock::now();
        long long build = 0, canon = 0, str = 0;
        gen.getTiming(build, canon, str);
        double total_ms = std::chrono::duration<double, std::milli>(te - ts).count();
        double b_ms = build / 1000.0, c_ms = canon / 1000.0, s_ms = str / 1000.0;

        std::ofstream bf("Benchmark.txt");
        bf << "# Myapp 性能基准 (Benchmark)\n";
        bf << "total_problems: " << got << "\n";
        bf << "total_time_ms: " << std::fixed << std::setprecision(3) << total_ms << "\n";
        bf << "build_ms: " << b_ms << "\n";
        bf << "canonical_ms: " << c_ms << "\n";
        bf << "tostr_ms: " << s_ms << "\n";
        bf.close();

        std::cout << "性能基准完成，共生成 " << got << " 道题目，结果已写入 Benchmark.txt\n";
        printBench(got, total_ms, b_ms, c_ms, s_ms);
        return 0;
    }

    // 出题模式
    if (!hasR) {
        std::cerr << "错误：缺少必填参数 -r（数值范围）。\n";
        printHelp();
        return 1;
    }
    if (r < 1) {
        std::cerr << "错误：-r 必须 >= 1。\n";
        printHelp();
        return 1;
    }
    if (n < 1) {
        std::cerr << "错误：-n 必须 >= 1。\n";
        return 1;
    }

    std::random_device rd;
    Generator gen(r, rd());
    std::vector<Problem> probs;
    size_t got = gen.generate(static_cast<size_t>(n), probs);
    if (got < static_cast<size_t>(n)) {
        std::cerr << "提示：-r " << r << " 下题目空间不足，仅生成 " << got
                  << " 道不重复题目。\n";
    }

    std::ofstream exo("Exercises.txt"), ans("Answers.txt");
    for (size_t i = 0; i < probs.size(); ++i) {
        exo << probs[i].question << "\n";
        ans << probs[i].answer << "\n";
    }
    std::cout << "已生成 " << probs.size()
              << " 道题目，分别写入 Exercises.txt 与 Answers.txt。\n";
    return 0;
}
