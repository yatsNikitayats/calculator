// OperationRegistry.h — список всех операций: встроенных и из плагинов.
//
// Парсер не спрашивает, откуда взялась операция, он просто смотрит в реестр.
// Парсер и загрузчик плагинов работают с одним реестром, а не друг с другом.
#pragma once

#include <functional>
#include <map>
#include <ostream>
#include <string>
#include <vector>

using FunctionImpl = std::function<double(const std::vector<double>&)>;
using BinaryImpl = std::function<double(double, double)>;

struct FunctionInfo {
    std::string name;
    int arity = 1;
    FunctionImpl impl;
    std::string origin;  // откуда взялась: "builtin" или имя dll
};

struct OperatorInfo {
    std::string symbol;
    int precedence = 0;
    bool rightAssociative = false;
    BinaryImpl impl;
    std::string origin;
};

class OperationRegistry {
public:
    // Бросают PluginError, если имя плохое или уже занято.
    void addFunction(FunctionInfo info);
    void addOperator(OperatorInfo info);

    // nullptr, если не нашли.
    const FunctionInfo* findFunction(const std::string& name) const;
    const OperatorInfo* findOperator(const std::string& symbol) const;

    // Длина самого длинного оператора, который начинается в позиции pos.
    // 0, если такого нет. Нужно лексеру.
    std::size_t matchOperator(const std::string& text, std::size_t pos) const;

    void printSummary(std::ostream& out) const;

private:
    std::map<std::string, FunctionInfo> functions_;
    std::map<std::string, OperatorInfo> operators_;
};

// Приоритеты встроенных операций. Плагины берут числа из этой же шкалы,
// например ^ = 40 — выше умножения и унарного минуса.
namespace precedence {
constexpr int kAdditive = 10;        // + -
constexpr int kMultiplicative = 20;  // * /
constexpr int kUnary = 30;           // -x, +x
}  // namespace precedence

void registerBuiltins(OperationRegistry& registry);
