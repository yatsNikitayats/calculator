// Errors.h — иерархия исключений приложения.
#pragma once

#include <stdexcept>

// Общий предок всех ошибок калькулятора.
// Конструкторы берём у std::runtime_error.
class CalcError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Ошибка в записи выражения, например "2 + * 3" или "(1 + 2".
class ParseError : public CalcError {
public:
    using CalcError::CalcError;
};

// Ошибка вычисления: деление на ноль, ln(-100), переполнение.
class MathError : public CalcError {
public:
    using CalcError::CalcError;
};

// Плагин не загрузился: битая dll, нет нужной функции, имя уже занято.
class PluginError : public CalcError {
public:
    using CalcError::CalcError;
};
