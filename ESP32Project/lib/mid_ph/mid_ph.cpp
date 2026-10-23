#include "mid_ph.h"

#include <math.h>

namespace {

constexpr float PH_ACID = 4.01f;
constexpr float PH_NEUTRAL = 7.0f;
constexpr float PH_MIN = 0.0f;
constexpr float PH_MAX = 14.0f;

constexpr float DIVIDER_RATIO = 2.0f;

constexpr float EPSILON = 1e-6f;

float ph_per_volt = 0.0f;
float voltage_at_ph7 = 0.0f;
bool initialized = false;

} // namespace

bool mid_ph_init(float voltage_ph_4_01, float voltage_ph_7_00) {
    initialized = false;

    if (!isfinite(voltage_ph_4_01) || !isfinite(voltage_ph_7_00) ||
        voltage_ph_4_01 < 0.0f || voltage_ph_7_00 < 0.0f) {
        return false;
    }

    const float voltage_diff = voltage_ph_7_00 - voltage_ph_4_01;
    if (fabsf(voltage_diff) <= EPSILON) {
        return false;
    }

    ph_per_volt = (PH_NEUTRAL - PH_ACID) / voltage_diff;
    voltage_at_ph7 = voltage_ph_7_00;
    if (!isfinite(ph_per_volt)) {
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

    const float calculated_ph =
        PH_NEUTRAL + (module_voltage - voltage_at_ph7) * ph_per_volt;

    if (!isfinite(calculated_ph)) {
        return NAN;
    }

    if (calculated_ph < PH_MIN) {
        return PH_MIN;
    }
    if (calculated_ph > PH_MAX) {
        return PH_MAX;
    }
    return calculated_ph;
}