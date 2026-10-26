// Parser.h — разбор выражений: рекурсивный спуск с учётом приоритетов.
//
// Грамматика (упрощённо):
//   expression := unary { binop expression }      — с учётом приоритетов
//   unary      := ('-' | '+') unary | primary
//   primary    := number | '(' expression ')' | name '(' args ')'
//
// Приоритеты берём из реестра, поэтому оператор из плагина, например ^,
// работает без правок парсера.
#pragma once

#include <string>
#include <vector>

#include "Lexer.h"
#include "OperationRegistry.h"

class Parser {
public:
    explicit Parser(const OperationRegistry& registry) : registry_(registry) {}

    // Считает выражение. При ошибке бросает ParseError или MathError.
    double evaluate(const std::string& expression);

private:
    double parseExpression(int minPrecedence);
    double parseUnary();
    double parsePrimary();
    double parseFunctionCall(const Token& nameToken);

    const Token& peek() const { return tokens_[pos_]; }
    const Token& advance() {  // на конце (End) позицию не двигаем
        const Token& t = tokens_[pos_];
        if (pos_ + 1 < tokens_.size()) ++pos_;
        return t;
    }
    void expect(TokenType type, const char* what);

    const OperationRegistry& registry_;
    std::vector<Token> tokens_;
    std::size_t pos_ = 0;
    int depth_ = 0;  // сколько раз уже вложились; нужно, чтобы не переполнить стек
};
