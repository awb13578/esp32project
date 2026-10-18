#include "mid_ph.h"

#include <math.h>

namespace {

constexpr float PH_NEUTRAL = 7.0f;
constexpr float PH_ACID = 4.01f;

constexpr float VOLTAGE_PH7 = 2.5f;

constexpr uint16_t ADC_MAX = 4095;
constexpr float ADC_FULL_SCALE_V = 3.3f;
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

float mid_ph_read_ph(uint16_t raw_ph) {
    if (!initialized || raw_ph > ADC_MAX) {
        return NAN;
    }

    const float adc_voltage =
        static_cast<float>(raw_ph) * ADC_FULL_SCALE_V / ADC_MAX;

    const float module_voltage = adc_voltage * DIVIDER_RATIO;

    const float ph =
        PH_NEUTRAL + (module_voltage - VOLTAGE_PH7) / slope;

    return isfinite(ph) ? ph : NAN;
}