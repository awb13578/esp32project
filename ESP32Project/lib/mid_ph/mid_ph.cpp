#include "mid_ph.h"

#include <math.h>

namespace {

constexpr float PH_NEUTRAL = 7.0f;
constexpr float PH_ACID = 3.95f;

constexpr float VOLTAGE_PH7 = 2.5f;

constexpr float DIVIDER_RATIO = 2.0f;

constexpr float EPSILON = 1e-6f;

float slope = 0.0f;
bool initialized = false;

} // namespace

bool mid_ph_init(float voltage_ph_4_01) {
    initialized = false;

    if (!isfinite(voltage_ph_4_01) ||
        voltage_ph_4_01 < 0.0f) {
        return false;
    }

    const float voltage_diff = voltage_ph_4_01 - VOLTAGE_PH7;

    if (fabsf(voltage_diff) <= EPSILON) {
        return false;
    }

    slope = voltage_diff / (PH_ACID - PH_NEUTRAL);
    initialized = true;

    return true;
}

float mid_ph_read_ph(uint32_t voltage_mv) {
    if (!initialized || voltage_mv == UINT32_MAX) {
        return NAN;
    }

    const float adc_voltage = static_cast<float>(voltage_mv) / 1000.0f;

    const float module_voltage = adc_voltage * DIVIDER_RATIO;

    const float ph =
        PH_NEUTRAL + (module_voltage - VOLTAGE_PH7) / slope;

    return isfinite(ph) ? ph : NAN;
}