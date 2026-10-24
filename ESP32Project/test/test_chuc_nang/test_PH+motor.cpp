#include "test_PH+motor.h"

#include <Arduino.h>
#include <math.h>

#include "../../src/app_state/app_state.h"
#include "drv_adc.h"
#include "drv_config.h"
#include "drv_pwm.h"
#include "mid_ph.h"

namespace {

constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;
constexpr uint8_t MOTOR_DUTY_PERCENT = 50;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t MOTOR_TOGGLE_INTERVAL_MS = 3000;

bool initialized = false;
bool motor_running = false;
uint32_t last_motor_toggle_ms = 0;

} // namespace

bool test_PH_motor_init(void) {
    initialized = false;
    motor_running = false;
    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;
    app_state.pwm_motor_percent = 0;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);
    const bool ph_ready = mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE);

    drv_pwm_init(PWM_ID_MOTOR, PWM_MOTOR_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    const bool motor_ready = pwm_inst[PWM_ID_MOTOR].sw_data.initialized;

    if (!ph_ready || !motor_ready) {
        if (motor_ready) {
            drv_pwm_disable(PWM_ID_MOTOR);
        }
        return false;
    }

    initialized = true;
    motor_running = true;
    drv_pwm_set_duty(PWM_ID_MOTOR, MOTOR_DUTY_PERCENT);
    app_state.pwm_motor_percent = MOTOR_DUTY_PERCENT;
    last_motor_toggle_ms = millis();
    return true;
}

bool test_PH_motor_update(void) {
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
    if (static_cast<uint32_t>(now - last_motor_toggle_ms) >=
        MOTOR_TOGGLE_INTERVAL_MS) {
        motor_running = !motor_running;
        drv_pwm_set_duty(PWM_ID_MOTOR,
                         motor_running ? MOTOR_DUTY_PERCENT : 0);
        app_state.pwm_motor_percent =
            motor_running ? MOTOR_DUTY_PERCENT : 0;
        last_motor_toggle_ms = now;
    }

    return ph_read_ok;
}
