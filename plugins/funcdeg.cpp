// funcdeg — оператор степени: 2^4 = 16.
// Считается справа налево: 2^3^2 = 512. Сильнее умножения и унарного минуса,
// поэтому -2^2 = -4.
#include <cmath>
#include <stdexcept>

#include "calc/plugin_api.h"

namespace {

double power(const double* args) {
    const double base = args[0];
    const double exponent = args[1];
    if (base == 0.0 && exponent < 0.0) throw std::domain_error("zero cannot be raised to a negative power");
    if (base < 0.0 && std::floor(exponent) != exponent)
        throw std::domain_error("negative base with fractional exponent");
    return std::pow(base, exponent);
}

const CalcPluginInfo kInfo = {
    CALC_PLUGIN_API_VERSION,
    "^",
    CALC_PLUGIN_BINARY_OPERATOR,
    2,   // оператор всегда бинарный
    40,  // приоритет: + - = 10, * / = 20, унарный минус = 30
    1,   // правоассоциативный
    &calc_plugin::safeCall<power>,
};

}  // namespace

CALC_PLUGIN_EXPORT const CalcPluginInfo* calc_get_plugin_info() { return &kInfo; }
