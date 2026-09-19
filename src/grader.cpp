#include "grader.h"
#include "parser.h"
#include <fstream>
#include <string>
#include <vector>

// 判分：题目与答案按行号 1..N 一一对应。先规范化再比较，容忍空格/' ’ 差异。
void grade(const std::string& exerciseFile,
           const std::string& answerFile,
           const std::string& outFile) {
    std::ifstream ef(exerciseFile), af(answerFile);
    std::vector<std::string> eqs, anss;
    std::string line;
    while (std::getline(ef, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) eqs.push_back(line);
    }
    while (std::getline(af, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) anss.push_back(line);
    }

    size_t N = std::min(eqs.size(), anss.size());
    std::vector<int> correct, wrong;
    for (size_t i = 0; i < N; ++i) {
        Fraction correctVal = parser::evaluate(eqs[i]);
        Fraction givenVal = parser::evaluate(anss[i]);
        if (correctVal == givenVal)
            correct.push_back(static_cast<int>(i + 1));
        else
            wrong.push_back(static_cast<int>(i + 1));
    }

    std::ofstream of(outFile);
    of << "Correct: " << correct.size() << " (";
    for (size_t i = 0; i < correct.size(); ++i) {
        if (i) of << ", ";
        of << correct[i];
    }
    of << ")\n";

    of << "Wrong: " << wrong.size() << " (";
    for (size_t i = 0; i < wrong.size(); ++i) {
        if (i) of << ", ";
        of << wrong[i];
    }
    of << ")\n";
}
