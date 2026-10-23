// plugin_api.h — договор между calc.exe и плагинами.
//
// Этот файл подключают и программа, и каждый плагин.
// Здесь только простой C: без std::string, без исключений и без искажения имён.
// Поэтому dll можно собрать другим компилятором, и всё будет работать.
#pragma once

#include <exception>

#define CALC_PLUGIN_API_VERSION 1

// Имя функции, которую программа ищет в каждой dll. Имя файла значения не имеет.
#define CALC_PLUGIN_ENTRY_NAME "calc_get_plugin_info"

// extern "C" — без этого имя функции в dll изменится, и GetProcAddress её не найдёт.
// __declspec(dllexport) — чтобы функция была видна снаружи dll.
#define CALC_PLUGIN_EXPORT extern "C" __declspec(dllexport)

extern "C" {

enum CalcPluginKind {
    CALC_PLUGIN_FUNCTION = 0,        // вызывается как name(a, b, ...)
    CALC_PLUGIN_BINARY_OPERATOR = 1  // вызывается как a <name> b, например 2^4
};

// Функция плагина.
//   args, argc — аргументы;
//   result — сюда кладём ответ;
//   errorBuf — буфер, который выделила программа; сюда плагин пишет текст ошибки.
// Возвращает 0, если всё хорошо, иначе ненулевой код.
typedef int (*CalcPluginFn)(const double* args, int argc, double* result,
                            char* errorBuf, int errorBufSize);

struct CalcPluginInfo {
    int apiVersion;        // должен быть равен CALC_PLUGIN_API_VERSION
    const char* name;      // "sin", "ln", "^" ...
    int kind;              // CalcPluginKind
    int arity;             // число аргументов (для оператора всегда 2)
    int precedence;        // приоритет (только для оператора)
    int rightAssociative;  // 1 — правоассоциативный (2^3^2 = 2^(3^2))
    CalcPluginFn function;
};

typedef const CalcPluginInfo* (*CalcGetPluginInfoFn)();

}  // extern "C"

// ---------------------------------------------------------------------------
// Помощь авторам плагинов. Внутри dll можно писать обычный C++ и бросать исключения.
// Наружу уходит только код возврата и текст ошибки, исключение dll не покидает.
namespace calc_plugin {

inline void copyMessage(const char* message, char* buf, int bufSize) noexcept {
    if (buf == nullptr || bufSize <= 0) return;
    int i = 0;
    for (; message != nullptr && message[i] != '\0' && i < bufSize - 1; ++i) buf[i] = message[i];
    buf[i] = '\0';  // строка всегда кончается нулём и не выходит за буфер
}

template <double (*Impl)(const double* args)>
int safeCall(const double* args, int /*argc*/, double* result, char* errorBuf,
             int errorBufSize) noexcept {
    try {
        *result = Impl(args);
        return 0;
    } catch (const std::exception& e) {
        copyMessage(e.what(), errorBuf, errorBufSize);
    } catch (...) {
        copyMessage("unknown error inside plugin", errorBuf, errorBufSize);
    }
    return 1;
}

}  // namespace calc_plugin
