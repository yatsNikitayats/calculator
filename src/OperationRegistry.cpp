#include "OperationRegistry.h"

#include <cctype>
#include <utility>

#include "Errors.h"

namespace {

bool isValidFunctionName(const std::string& name) {
    if (name.empty()) return false;
    const auto first = static_cast<unsigned char>(name[0]);
    if (!std::isalpha(first) && first != '_') return false;
    for (char c : name) {
        const auto uc = static_cast<unsigned char>(c);
        if (!std::isalnum(uc) && uc != '_') return false;
    }
    return true;
}

bool isValidOperatorSymbol(const std::string& symbol) {
    if (symbol.empty()) return false;
    for (char c : symbol) {
        const auto uc = static_cast<unsigned char>(c);
        // Скобки, запятую и точку нельзя использовать: в выражениях они уже значат своё.
        if (!std::ispunct(uc) || c == '(' || c == ')' || c == ',' || c == '.' || c == '_')
            return false;
    }
    return true;
}

}  // namespace

void OperationRegistry::addFunction(FunctionInfo info) {
    if (!isValidFunctionName(info.name))
        throw PluginError("invalid function name '" + info.name + "'");
    if (info.arity < 1 || info.arity > 16)
        throw PluginError("function '" + info.name + "' has invalid arity");
    if (!info.impl)
        throw PluginError("function '" + info.name + "' has no implementation");

    auto existing = functions_.find(info.name);
    if (existing != functions_.end())
        throw PluginError("function '" + info.name + "' is already defined by " +
                          existing->second.origin);

    std::string key = info.name;
    functions_.emplace(std::move(key), std::move(info));
}

void OperationRegistry::addOperator(OperatorInfo info) {
    if (!isValidOperatorSymbol(info.symbol))
        throw PluginError("invalid operator symbol '" + info.symbol + "'");
    if (info.precedence <= 0)
        throw PluginError("operator '" + info.symbol + "' has invalid precedence");
    if (!info.impl)
        throw PluginError("operator '" + info.symbol + "' has no implementation");

    auto existing = operators_.find(info.symbol);
    if (existing != operators_.end())
        throw PluginError("operator '" + info.symbol + "' is already defined by " +
                          existing->second.origin);

    std::string key = info.symbol;
    operators_.emplace(std::move(key), std::move(info));
}

const FunctionInfo* OperationRegistry::findFunction(const std::string& name) const {
    auto it = functions_.find(name);
    return it == functions_.end() ? nullptr : &it->second;
}

const OperatorInfo* OperationRegistry::findOperator(const std::string& symbol) const {
    auto it = operators_.find(symbol);
    return it == operators_.end() ? nullptr : &it->second;
}

std::size_t OperationRegistry::matchOperator(const std::string& text, std::size_t pos) const {
    std::size_t best = 0;
    for (const auto& [symbol, info] : operators_) {
        if (symbol.size() > best && text.compare(pos, symbol.size(), symbol) == 0)
            best = symbol.size();
    }
    return best;
}

void OperationRegistry::printSummary(std::ostream& out) const {
    out << "Operators:";
    for (const auto& [symbol, info] : operators_) out << ' ' << symbol;
    out << "\nFunctions:";
    if (functions_.empty()) out << " (none)";
    for (const auto& [name, info] : functions_) {
        out << "\n  " << name << '(' << info.arity << " arg" << (info.arity > 1 ? "s" : "")
            << ")  <- " << info.origin;
    }
    out << '\n';
}

void registerBuiltins(OperationRegistry& registry) {
    using namespace precedence;
    registry.addOperator({"+", kAdditive, false, [](double a, double b) { return a + b; }, "builtin"});
    registry.addOperator({"-", kAdditive, false, [](double a, double b) { return a - b; }, "builtin"});
    registry.addOperator({"*", kMultiplicative, false, [](double a, double b) { return a * b; }, "builtin"});
    registry.addOperator({"/", kMultiplicative, false,
                          [](double a, double b) {
                              if (b == 0.0) throw MathError("division by zero");
                              return a / b;
                          },
                          "builtin"});
}
