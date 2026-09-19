#pragma once
#include <string>

// 判分：读取题目文件与答案文件，逐题比对，统计 Correct / Wrong 并写入 Grade.txt
void grade(const std::string& exerciseFile,
           const std::string& answerFile,
           const std::string& outFile);
