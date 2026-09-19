#include "generator.h"
#include "grader.h"
#include <fstream>
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
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    bool hasN = false, hasR = false, hasE = false, hasA = false;
    long long n = 10, r = -1;
    std::string eFile, aFile;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-n" && i + 1 < args.size()) { n = std::stoll(args[++i]); hasN = true; }
        else if (a == "-r" && i + 1 < args.size()) { r = std::stoll(args[++i]); hasR = true; }
        else if (a == "-e" && i + 1 < args.size()) { eFile = args[++i]; hasE = true; }
        else if (a == "-a" && i + 1 < args.size()) { aFile = args[++i]; hasA = true; }
        else if (a == "-h" || a == "--help" || a == "-?") { printHelp(); return 0; }
    }

    // 判分模式
    if (hasE && hasA) {
        grade(eFile, aFile, "Grade.txt");
        std::cout << "判分完成，结果已写入 Grade.txt\n";
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
