#include "test_PH+Pump.h"

#include <Arduino.h>
#include <math.h>

#include "../../src/app_state/app_state.h"
#include "drv_adc.h"
#include "drv_config.h"
#include "drv_pwm.h"
#include "mid_ph.h"

namespace {

constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;
constexpr uint8_t PUMP_DUTY_LOW_PERCENT = 30;
constexpr uint8_t PUMP_DUTY_HIGH_PERCENT = 70;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t PUMP_STEP_INTERVAL_MS = 5000;

enum pump_test_step_e {
    PUMP_TEST_STEP_LOW,
    PUMP_TEST_STEP_HIGH,
    PUMP_TEST_STEP_OFF,
};

bool initialized = false;
pump_test_step_e pump_step = PUMP_TEST_STEP_LOW;
uint32_t last_pump_step_ms = 0;

uint8_t duty_for_step(pump_test_step_e step) {
    switch (step) {
        case PUMP_TEST_STEP_LOW:
            return PUMP_DUTY_LOW_PERCENT;
        case PUMP_TEST_STEP_HIGH:
            return PUMP_DUTY_HIGH_PERCENT;
        case PUMP_TEST_STEP_OFF:
        default:
            return 0;
    }
}

} // namespace

bool test_PH_pump_init(void) {
    initialized = false;
    pump_step = PUMP_TEST_STEP_LOW;
    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;
    app_state.pwm_pump_acid_percent = 0;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);
    const bool ph_ready = mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE);

    drv_pwm_init(PWM_ID_PUMP_ACID, PWM_PUMP_ACID_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    const bool pump_ready =
        pwm_inst[PWM_ID_PUMP_ACID].sw_data.initialized;

    if (!ph_ready || !pump_ready) {
        if (pump_ready) {
            drv_pwm_disable(PWM_ID_PUMP_ACID);
        }
        return false;
    }

    initialized = true;
    drv_pwm_set_duty(PWM_ID_PUMP_ACID, PUMP_DUTY_LOW_PERCENT);
    app_state.pwm_pump_acid_percent = PUMP_DUTY_LOW_PERCENT;
    last_pump_step_ms = millis();
    return true;
}

bool test_PH_pump_update(void) {
    if (!initialized) {
        return false;
    }

    const uint32_t voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
    app_state.adc_ph_raw = analogRead(ADC_PH_PIN);

    bool ph_read_ok = voltage_mv != UINT32_MAX;
    if (ph_read_ok) {
        app_state.adc_ph_mv = voltage_mv;
        app_state.voltage_ph_adc_v =
            static_cast<float>(voltage_mv) / 1000.0f;
        app_state.voltage_ph_v = app_state.voltage_ph_adc_v * 2.0f;
        app_state.ph = mid_ph_read_ph(voltage_mv);
        ph_read_ok = isfinite(app_state.ph);
    }

    if (!ph_read_ok) {
        app_state.adc_ph_mv = 0;
        app_state.voltage_ph_adc_v = NAN;
        app_state.voltage_ph_v = NAN;
        app_state.ph = NAN;
    }

    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - last_pump_step_ms) >=
        PUMP_STEP_INTERVAL_MS) {
        pump_step = static_cast<pump_test_step_e>(
            (static_cast<uint8_t>(pump_step) + 1) % 3);
        const uint8_t duty = duty_for_step(pump_step);
        drv_pwm_set_duty(PWM_ID_PUMP_ACID, duty);
        app_state.pwm_pump_acid_percent = duty;
        last_pump_step_ms = now;
    }

    return ph_read_ok;
}
