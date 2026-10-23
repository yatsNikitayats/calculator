// funcln — натуральный логарифм. При x <= 0 выдаёт ошибку, например для ln(-100).
#include <cmath>
#include <stdexcept>

#include "calc/plugin_api.h"

namespace {

double naturalLog(const double* args) {
    if (args[0] <= 0.0) throw std::domain_error("argument must be > 0");
    return std::log(args[0]);
}

const CalcPluginInfo kInfo = {
    CALC_PLUGIN_API_VERSION, "ln", CALC_PLUGIN_FUNCTION, 1, 0, 0,
    &calc_plugin::safeCall<naturalLog>,
};

}  // namespace

CALC_PLUGIN_EXPORT const CalcPluginInfo* calc_get_plugin_info() { return &kInfo; }
