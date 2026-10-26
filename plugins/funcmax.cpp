// funcmax — max(a, b). Пример функции с двумя аргументами.
#include "calc/plugin_api.h"

namespace {

double maximum(const double* args) { return args[0] > args[1] ? args[0] : args[1]; }

const CalcPluginInfo kInfo = {
    CALC_PLUGIN_API_VERSION, "max", CALC_PLUGIN_FUNCTION, 2, 0, 0,
    &calc_plugin::safeCall<maximum>,
};

}  // namespace

CALC_PLUGIN_EXPORT const CalcPluginInfo* calc_get_plugin_info() { return &kInfo; }
