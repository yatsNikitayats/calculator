// funcsqrt — квадратный корень sqrt(x), x >= 0.
#include <cmath>
#include <stdexcept>

#include "calc/plugin_api.h"

namespace {

double squareRoot(const double* args) {
    if (args[0] < 0.0) throw std::domain_error("argument must be >= 0");
    return std::sqrt(args[0]);
}

const CalcPluginInfo kInfo = {
    CALC_PLUGIN_API_VERSION, "sqrt", CALC_PLUGIN_FUNCTION, 1, 0, 0,
    &calc_plugin::safeCall<squareRoot>,
};

}  // namespace

CALC_PLUGIN_EXPORT const CalcPluginInfo* calc_get_plugin_info() { return &kInfo; }
