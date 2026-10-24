#include "test_PH.h"

#include <Arduino.h>
#include <math.h>

#include "../../src/app_state/app_state.h"
#include "drv_adc.h"
#include "drv_config.h"
#include "mid_ph.h"

namespace {

constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;

bool initialized = false;

} // namespace

bool test_PH_init(void) {
    initialized = false;
    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);
    initialized = mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE);
    return initialized;
}

bool test_PH_update(void) {
    if (!initialized) {
        return false;
    }

    const uint32_t voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
    app_state.adc_ph_raw = analogRead(ADC_PH_PIN);

    if (voltage_mv == UINT32_MAX) {
        app_state.adc_ph_mv = 0;
        app_state.voltage_ph_adc_v = NAN;
        app_state.voltage_ph_v = NAN;
        app_state.ph = NAN;
        return false;
    }

    app_state.adc_ph_mv = voltage_mv;
    app_state.voltage_ph_adc_v = static_cast<float>(voltage_mv) / 1000.0f;
    app_state.voltage_ph_v = app_state.voltage_ph_adc_v * 2.0f;
    app_state.ph = mid_ph_read_ph(voltage_mv);

    return isfinite(app_state.ph);
}
