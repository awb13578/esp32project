#include "mid_ph.h"

#include <math.h>

namespace {

constexpr float PH_NEUTRAL = 7.0f;
constexpr float PH_ACID = 4.01f;
constexpr float PH_BASE = 9.18f;

constexpr float VOLTAGE_PH7 = 2.5f;

constexpr float DIVIDER_RATIO = 2.0f;

constexpr float EPSILON = 1e-6f;

float acid_slope = 0.0f;
float base_slope = 0.0f;
bool initialized = false;

} // namespace

bool mid_ph_init(float voltage_ph_4_01, float voltage_ph_9_18) {
    initialized = false;

    if (!isfinite(voltage_ph_4_01) ||
        voltage_ph_4_01 < 0.0f ||
        !isfinite(voltage_ph_9_18) ||
        voltage_ph_9_18 < 0.0f) {
        return false;
    }

    const float acid_voltage_diff = voltage_ph_4_01 - VOLTAGE_PH7;
    const float base_voltage_diff = voltage_ph_9_18 - VOLTAGE_PH7;

    if (fabsf(acid_voltage_diff) <= EPSILON ||
        fabsf(base_voltage_diff) <= EPSILON) {
        return false;
    }

    acid_slope = acid_voltage_diff / (PH_ACID - PH_NEUTRAL);
    base_slope = base_voltage_diff / (PH_BASE - PH_NEUTRAL);

    if (acid_slope * base_slope <= 0.0f) {
        return false;
    }

    initialized = true;

    return true;
}

float mid_ph_read_ph(uint32_t voltage_mv) {
    if (!initialized || voltage_mv == UINT32_MAX) {
        return NAN;
    }

    const float adc_voltage = static_cast<float>(voltage_mv) / 1000.0f;

    const float module_voltage = adc_voltage * DIVIDER_RATIO;
    const float voltage_from_neutral = module_voltage - VOLTAGE_PH7;

    const float slope = voltage_from_neutral * base_slope > 0.0f
                            ? base_slope
                            : acid_slope;
    const float ph = PH_NEUTRAL + voltage_from_neutral / slope;

    return isfinite(ph) ? ph : NAN;
}