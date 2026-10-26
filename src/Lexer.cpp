#include "Lexer.h"

#include <cctype>
#include <cerrno>
#include <cstdlib>

#include "Errors.h"

std::vector<Token> tokenize(const std::string& expression, const OperationRegistry& registry) {
    const std::string& s = expression;
    std::vector<Token> tokens;
    std::size_t i = 0;

    while (i < s.size()) {
        const auto c = static_cast<unsigned char>(s[i]);

        if (std::isspace(c)) {
            ++i;
            continue;
        }

        Token tok;
        tok.pos = i;

        if (std::isdigit(c) || c == '.') {
            const char* begin = s.c_str() + i;
            char* end = nullptr;
            errno = 0;
            tok.value = std::strtod(begin, &end);
            if (end == begin) throw ParseError("invalid number at position " + std::to_string(i + 1));
            if (errno == ERANGE) throw ParseError("number is out of range at position " + std::to_string(i + 1));
            tok.type = TokenType::Number;
            tok.text.assign(begin, static_cast<std::size_t>(end - begin));
            i += static_cast<std::size_t>(end - begin);
        } else if (std::isalpha(c) || c == '_') {
            std::size_t j = i;
            while (j < s.size() && (std::isalnum(static_cast<unsigned char>(s[j])) || s[j] == '_')) ++j;
            tok.type = TokenType::Identifier;
            tok.text = s.substr(i, j - i);
            i = j;
        } else if (c == '(') {
            tok.type = TokenType::LParen;
            tok.text = "(";
            ++i;
        } else if (c == ')') {
            tok.type = TokenType::RParen;
            tok.text = ")";
            ++i;
        } else if (c == ',') {
            tok.type = TokenType::Comma;
            tok.text = ",";
            ++i;
        } else if (std::size_t len = registry.matchOperator(s, i); len > 0) {
            tok.type = TokenType::Operator;
            tok.text = s.substr(i, len);
            i += len;
        } else {
            throw ParseError("unknown symbol '" + std::string(1, s[i]) + "' at position " +
                             std::to_string(i + 1));
        }
        tokens.push_back(std::move(tok));
    }

    Token end;
    end.type = TokenType::End;
    end.pos = s.size();
    tokens.push_back(end);
    return tokens;
}
