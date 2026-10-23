// Lexer.h — режет строку выражения на токены: числа, имена, знаки.
#pragma once

#include <string>
#include <vector>

#include "OperationRegistry.h"

enum class TokenType { Number, Identifier, Operator, LParen, RParen, Comma, End };

struct Token {
    TokenType type = TokenType::End;
    std::string text;      // исходный текст токена
    double value = 0.0;    // для Number
    std::size_t pos = 0;   // позиция в строке (для сообщений об ошибках)
};

// Лексеру нужен реестр, чтобы он знал операторы из плагинов, например ^.
std::vector<Token> tokenize(const std::string& expression, const OperationRegistry& registry);
