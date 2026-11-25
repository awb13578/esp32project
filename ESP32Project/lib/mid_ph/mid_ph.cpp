#include "mid_ph.h"

#include <math.h>

#include "drv_config.h"

namespace {

constexpr float PH_NEUTRAL = 6.8f;
constexpr float PH_ACID = 1.85f;
constexpr float PH_BASE = 8.29f;

constexpr float VOLTAGE_PH_ACID = 3.37f;
constexpr float VOLTAGE_PH7 = 2.67f;
constexpr float VOLTAGE_PH_BASE = 2.3f;
constexpr float ADC_DIVIDER_RATIO = 2.0f;

constexpr float EPSILON = 1e-6f;

float acid_slope = 0.0f;
float base_slope = 0.0f;
bool initialized = false;

} // namespace

bool mid_ph_init(void) {
    initialized = false;

    const float acid_voltage_diff = VOLTAGE_PH_ACID - VOLTAGE_PH7;
    const float base_voltage_diff = VOLTAGE_PH_BASE - VOLTAGE_PH7;
    if (fabsf(acid_voltage_diff) <= EPSILON ||
        fabsf(base_voltage_diff) <= EPSILON) {
        return false;
    }

    acid_slope = (PH_ACID - PH_NEUTRAL) / acid_voltage_diff;
    base_slope = (PH_BASE - PH_NEUTRAL) / base_voltage_diff;
    if (!isfinite(acid_slope) || !isfinite(base_slope)) {
        return false;
    }

    initialized = true;

    return true;
}

float mid_ph_get_module_voltage(uint32_t adc_voltage_mv) {
    if (adc_voltage_mv == UINT32_MAX) {
        return NAN;
    }

    const float adc_voltage = static_cast<float>(adc_voltage_mv) / 1000.0f;
    const float module_voltage = adc_voltage * ADC_DIVIDER_RATIO;
    return isfinite(module_voltage) ? module_voltage : NAN;
}

float mid_ph_read_ph(uint32_t voltage_mv) {
    if (!initialized || voltage_mv == UINT32_MAX) {
        return NAN;
    }

    const float module_voltage = mid_ph_get_module_voltage(voltage_mv);
    if (!isfinite(module_voltage)) {
        return NAN;
    }

    float ph;
    if (module_voltage >= VOLTAGE_PH7) {
        ph = PH_NEUTRAL + (module_voltage - VOLTAGE_PH7) * acid_slope;
    } else {
        ph = PH_NEUTRAL + (module_voltage - VOLTAGE_PH7) * base_slope;
    }

    return isfinite(ph) ? ph : NAN;
}