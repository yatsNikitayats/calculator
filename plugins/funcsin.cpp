// funcsin — функция sin(x), x в градусах: sin(90) = 1.
#include <cmath>

#include "calc/plugin_api.h"

namespace {

double sinDegrees(const double* args) {
    const double pi = std::acos(-1.0);
    const double value = std::sin(args[0] * pi / 180.0);
    return std::fabs(value) < 1e-12 ? 0.0 : value;  // иначе sin(180) показал бы 1.2e-16, а не ноль
}

const CalcPluginInfo kInfo = {
    CALC_PLUGIN_API_VERSION,
    "sin",                 // имя в выражении; имя файла dll здесь не важно
    CALC_PLUGIN_FUNCTION,
    1,                     // один аргумент
    0, 0,                  // приоритет/ассоциативность — только для операторов
    &calc_plugin::safeCall<sinDegrees>,
};

}  // namespace

CALC_PLUGIN_EXPORT const CalcPluginInfo* calc_get_plugin_info() { return &kInfo; }
