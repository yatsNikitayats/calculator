#include "Parser.h"

#include <cmath>

#include "Errors.h"

namespace {

std::string describe(const Token& t) {
    if (t.type == TokenType::End) return "end of expression";
    return "'" + t.text + "' at position " + std::to_string(t.pos + 1);
}

// Результат должен быть обычным числом: inf и nan считаем ошибкой.
double checkFinite(double value, const std::string& operation) {
    if (!std::isfinite(value))
        throw MathError("result of '" + operation + "' is not a finite number (overflow or undefined)");
    return value;
}

constexpr int kMaxDepth = 500;

// Счётчик глубины: сам уменьшается при выходе из функции, даже если было исключение.
class DepthGuard {
public:
    explicit DepthGuard(int& depth) : depth_(depth) {
        if (++depth_ > kMaxDepth) {
            --depth_;
            throw ParseError("expression is too deeply nested");
        }
    }
    ~DepthGuard() { --depth_; }
    DepthGuard(const DepthGuard&) = delete;
    DepthGuard& operator=(const DepthGuard&) = delete;

private:
    int& depth_;
};

}  // namespace

double Parser::evaluate(const std::string& expression) {
    tokens_ = tokenize(expression, registry_);
    pos_ = 0;
    depth_ = 0;

    if (peek().type == TokenType::End) throw ParseError("empty expression");

    const double result = parseExpression(0);
    if (peek().type != TokenType::End) throw ParseError("unexpected " + describe(peek()));
    return result;
}

double Parser::parseExpression(int minPrecedence) {
    DepthGuard guard(depth_);
    double lhs = parseUnary();

    while (peek().type == TokenType::Operator) {
        const OperatorInfo* op = registry_.findOperator(peek().text);
        if (op == nullptr) throw ParseError("unknown operator " + describe(peek()));
        if (op->precedence < minPrecedence) break;

        advance();
        // nextMin — с какого приоритета разбирать правую часть.
        // Для левоассоциативных (- и /) ставим +1: оператор того же приоритета
        // не попадёт в правую часть, его возьмёт цикл, поэтому 2 - 3 - 4 = -5.
        // Для правоассоциативных (^) +1 не ставим, и 2 ^ 3 ^ 2 = 2^(3^2) = 512.
        const int nextMin = op->rightAssociative ? op->precedence : op->precedence + 1;
        const double rhs = parseExpression(nextMin);
        lhs = checkFinite(op->impl(lhs, rhs), op->symbol);
    }
    return lhs;
}

double Parser::parseUnary() {
    const Token& t = peek();
    if (t.type == TokenType::Operator && (t.text == "-" || t.text == "+")) {
        const bool negate = (t.text == "-");
        advance();
        const double value = parseExpression(precedence::kUnary);
        return negate ? -value : value;
    }
    return parsePrimary();
}

double Parser::parsePrimary() {
    const Token& t = advance();
    switch (t.type) {
        case TokenType::Number:
            return t.value;
        case TokenType::LParen: {
            const double value = parseExpression(0);
            expect(TokenType::RParen, "')'");
            return value;
        }
        case TokenType::Identifier:
            return parseFunctionCall(t);
        default:
            throw ParseError("expected number, function or '(' but got " + describe(t));
    }
}

double Parser::parseFunctionCall(const Token& nameToken) {
    const FunctionInfo* fn = registry_.findFunction(nameToken.text);
    if (fn == nullptr)
        throw ParseError("unknown function '" + nameToken.text + "' (is the plugin loaded?)");

    expect(TokenType::LParen, "'(' after function name");

    std::vector<double> args;
    if (peek().type != TokenType::RParen) {
        args.push_back(parseExpression(0));
        while (peek().type == TokenType::Comma) {
            advance();
            args.push_back(parseExpression(0));
        }
    }
    expect(TokenType::RParen, "')'");

    if (static_cast<int>(args.size()) != fn->arity)
        throw ParseError("function '" + fn->name + "' expects " + std::to_string(fn->arity) +
                         " argument(s), got " + std::to_string(args.size()));

    return checkFinite(fn->impl(args), fn->name);
}

void Parser::expect(TokenType type, const char* what) {
    if (peek().type != type) throw ParseError(std::string("expected ") + what + " but got " + describe(peek()));
    advance();
}
