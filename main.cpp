#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include "GLSLminifier.hpp"

namespace Color
{
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string CYAN = "\033[36m";
    const std::string RED = "\033[31m";
}

std::string readFile(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void saveFile(const std::string &filepath, const std::string &content)
{
    std::ofstream file(filepath);
    if (file.is_open())
    {
        file << content;
    }
}

void printHeader()
{
    std::cout << Color::BOLD << Color::CYAN
              << "======================================================\n"
              << "            GLSL Minifier - Performance Test          \n"
              << "======================================================"
              << Color::RESET << "\n\n";
}

int main()
{
    printHeader();

    // List of test files
    std::vector<std::string> testFiles = {
        "test1.glsl",
        "test2.glsl",
        "test3.glsl"};

    size_t totalOriginalSize = 0;
    size_t totalMinifiedSize = 0;
    double totalTimeMs = 0.0;

    for (const auto &filename : testFiles)
    {
        std::cout << Color::BOLD << "[*] Processing: " << filename << Color::RESET << "\n";

        try
        {

            std::string originalCode = readFile(filename);
            size_t originalSize = originalCode.length();

            auto start = std::chrono::high_resolution_clock::now();

            std::string minifiedCode = GLSLminifier(originalCode);

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;

            size_t minifiedSize = minifiedCode.length();
            double savings = 100.0 - ((static_cast<double>(minifiedSize) / originalSize) * 100.0);

            totalOriginalSize += originalSize;
            totalMinifiedSize += minifiedSize;
            totalTimeMs += duration.count();

            std::string outFilename = "min_" + filename;
            saveFile(outFilename, minifiedCode);

            std::cout << "    " << Color::YELLOW << "Original Size: " << Color::RESET << std::setw(6) << originalSize << " bytes\n";
            std::cout << "    " << Color::GREEN << "Minified Size: " << Color::RESET << std::setw(6) << minifiedSize << " bytes\n";
            std::cout << "    " << Color::CYAN << "Compression:   " << Color::RESET << std::fixed << std::setprecision(2) << savings << "%\n";
            std::cout << "    " << Color::YELLOW << "Time taken:    " << Color::RESET << duration.count() << " ms\n";
            std::cout << "    " << Color::GREEN << "Saved to:      " << Color::RESET << outFilename << "\n";
            std::cout << "------------------------------------------------------\n";
        }
        catch (const std::exception &e)
        {
            std::cout << "    " << Color::RED << "Error: " << e.what() << Color::RESET << "\n";
            std::cout << "------------------------------------------------------\n";
        }
    }

    if (totalOriginalSize > 0)
    {
        double totalSavings = 100.0 - ((static_cast<double>(totalMinifiedSize) / totalOriginalSize) * 100.0);
        std::cout << Color::BOLD << Color::CYAN << "\n[=== SUMMARY ===]\n"
                  << Color::RESET;
        std::cout << "Total Original:  " << totalOriginalSize << " bytes\n";
        std::cout << "Total Minified:  " << totalMinifiedSize << " bytes\n";
        std::cout << "Total Savings:   " << Color::GREEN << Color::BOLD << std::fixed << std::setprecision(2) << totalSavings << "% \n"
                  << Color::RESET;
        std::cout << "Total Time:      " << totalTimeMs << " ms\n\n";
    }

    return 0;
}