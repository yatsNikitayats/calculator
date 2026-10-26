// PluginLoader.h — ищет dll в папке plugins и регистрирует их функции.
#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "OperationRegistry.h"

struct PluginLoadReport {
    std::vector<std::string> loaded;  // "funcsin.dll -> sin"
    std::vector<std::string> errors;  // "broken.dll: cannot load library ..."
};

// Загружает все dll из папки dir и добавляет их функции в registry.
// Сломанный плагин не мешает остальным: ошибка просто попадает в отчёт.
PluginLoadReport loadPlugins(const std::filesystem::path& dir, OperationRegistry& registry);
