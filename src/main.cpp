// main.cpp — консольная программа-калькулятор.
//
// Сначала регистрирует встроенные + - * /, потом загружает плагины из папки plugins,
// затем читает строки и печатает результат или ошибку.
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "DynamicLibrary.h"
#include "Errors.h"
#include "OperationRegistry.h"
#include "Parser.h"
#include "PluginLoader.h"

namespace {

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::string formatNumber(double value) {
    if (value == 0.0) value = 0.0;  // чтобы не печатать минус перед нулём
    std::ostringstream out;
    out << std::setprecision(12) << value;
    return out.str();
}

void printHelp(const OperationRegistry& registry) {
    std::cout << "Enter an expression, e.g. 16 + 4 * (3 - 1)\n"
                 "Commands: help, exit\n";
    registry.printSummary(std::cout);
}

}  // namespace

int main() {
    try {
        OperationRegistry registry;
        registerBuiltins(registry);

        const auto pluginDir = platform::executableDir() / "plugins";
        const PluginLoadReport report = loadPlugins(pluginDir, registry);
        for (const auto& line : report.loaded) std::cout << "[plugin] loaded " << line << '\n';
        for (const auto& line : report.errors) std::cerr << "[plugin] warning: " << line << '\n';
        if (report.loaded.empty())
            std::cerr << "[plugin] warning: no plugins loaded, only + - * / ( ) are available\n";

        std::cout << "Calculator. Type 'help' for the list of functions, 'exit' to quit.\n";

        Parser parser(registry);
        std::string line;
        while (std::cout << "> " << std::flush, std::getline(std::cin, line)) {
            const std::string input = trim(line);
            if (input.empty()) continue;
            if (input == "exit" || input == "quit") break;
            if (input == "help") {
                printHelp(registry);
                continue;
            }

            try {
                std::cout << formatNumber(parser.evaluate(input)) << '\n';
            } catch (const CalcError& e) {
                // Ошибка в одной строке не должна закрывать программу.
                std::cout << "Error: " << e.what() << '\n';
            }
        }
        std::cout << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: unknown exception\n";
        return 1;
    }
    return 0;
}
