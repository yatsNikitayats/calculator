#include "PluginLoader.h"

#include <algorithm>
#include <cctype>
#include <memory>

#include "DynamicLibrary.h"
#include "Errors.h"
#include "calc/plugin_api.h"

namespace fs = std::filesystem;

namespace {

bool hasLibraryExtension(const fs::path& file) {
    std::string ext = file.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext == platform::libraryExtension();
}

// Берём описание функции из dll и проверяем, что оно в порядке.
const CalcPluginInfo& readPluginInfo(const DynamicLibrary& lib) {
    auto getInfo = reinterpret_cast<CalcGetPluginInfoFn>(lib.symbol(CALC_PLUGIN_ENTRY_NAME));
    if (getInfo == nullptr)
        throw PluginError(std::string("entry point '") + CALC_PLUGIN_ENTRY_NAME + "' not found");

    const CalcPluginInfo* info = nullptr;
    try {
        info = getInfo();
    } catch (...) {
        throw PluginError("entry point threw an exception");
    }

    if (info == nullptr) throw PluginError("entry point returned null");
    if (info->apiVersion != CALC_PLUGIN_API_VERSION)
        throw PluginError("unsupported plugin API version " + std::to_string(info->apiVersion));
    if (info->name == nullptr) throw PluginError("plugin has no name");
    if (info->function == nullptr) throw PluginError("plugin has no function pointer");
    return *info;
}

// Вызывает функцию плагина и превращает код ошибки обратно в исключение MathError.
double callPlugin(CalcPluginFn fn, const std::string& name, const double* args, int argc) {
    constexpr int kErrorBufSize = 256;
    char error[kErrorBufSize] = {};
    double result = 0.0;

    int rc = 0;
    try {
        rc = fn(args, argc, &result, error, kErrorBufSize);
    } catch (...) {  // на всякий случай: вдруг плагин всё же бросил исключение
        throw MathError(name + ": plugin threw an unexpected exception");
    }
    if (rc != 0) {
        error[kErrorBufSize - 1] = '\0';
        throw MathError(name + ": " + (error[0] ? error : "plugin reported an error"));
    }
    return result;
}

std::string registerPlugin(const fs::path& file, OperationRegistry& registry) {
    auto lib = std::make_shared<DynamicLibrary>(file);  // если ниже будет ошибка, dll выгрузится сама
    const CalcPluginInfo& info = readPluginInfo(*lib);

    const std::string name = info.name;
    const std::string origin = file.filename().string();
    const CalcPluginFn fn = info.function;

    // Лямбды ниже держат shared_ptr на dll. Пока функция в реестре, библиотека
    // не выгрузится. Когда реестр удалится, последний shared_ptr вызовет FreeLibrary.

    switch (info.kind) {
        case CALC_PLUGIN_FUNCTION: {
            const int arity = info.arity;
            registry.addFunction({name, arity,
                                  [lib, fn, name, arity](const std::vector<double>& args) {
                                      (void)lib;
                                      return callPlugin(fn, name, args.data(), arity);
                                  },
                                  origin});
            break;
        }
        case CALC_PLUGIN_BINARY_OPERATOR: {
            if (info.arity != 2) throw PluginError("binary operator must have arity 2");
            registry.addOperator({name, info.precedence, info.rightAssociative != 0,
                                  [lib, fn, name](double a, double b) {
                                      (void)lib;
                                      const double args[2] = {a, b};
                                      return callPlugin(fn, name, args, 2);
                                  },
                                  origin});
            break;
        }
        default:
            throw PluginError("unknown plugin kind " + std::to_string(info.kind));
    }
    return name;
}

}  // namespace

PluginLoadReport loadPlugins(const fs::path& dir, OperationRegistry& registry) {
    PluginLoadReport report;

    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        report.errors.push_back("plugin directory not found: " + dir.string());
        return report;
    }

    std::error_code iterEc;
    fs::directory_iterator it(dir, iterEc);
    if (iterEc) {
        report.errors.push_back("cannot read plugin directory " + dir.string() + ": " +
                                iterEc.message());
        return report;
    }

    std::vector<fs::path> files;
    for (const auto& entry : it) {
        std::error_code fileEc;
        if (entry.is_regular_file(fileEc) && hasLibraryExtension(entry.path()))
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());  // чтобы порядок загрузки был одинаковым при каждом запуске

    if (files.empty()) {
        report.errors.push_back(std::string("no plugins (*") + platform::libraryExtension() +
                                ") found in " + dir.string());
        return report;
    }

    for (const auto& file : files) {
        try {
            const std::string name = registerPlugin(file, registry);
            report.loaded.push_back(file.filename().string() + " -> " + name);
        } catch (const std::exception& e) {
            report.errors.push_back(file.filename().string() + ": " + e.what());
        }
    }
    return report;
}
