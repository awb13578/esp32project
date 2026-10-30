#include "app_control.h"

#include <Arduino.h>
#include <math.h>

#include "drv_adc.h"
#include "drv_config.h"
#include "drv_pwm.h"
#include "mid_ph.h"
#include "../app_state/app_state.h"

namespace {

constexpr uint8_t PWM_DUTY_LOW = 50;
constexpr uint8_t PWM_DUTY_HIGH = 80;
constexpr uint8_t PWM_DUTY_MOTOR = 50;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t PUMP_PULSE_DURATION_LOW_DUTY_MS = 1000;
constexpr uint32_t PUMP_PULSE_DURATION_HIGH_DUTY_MS = 2000;
constexpr uint32_t MIXING_DURATION_MS = 5000;
constexpr uint32_t ADC_SETTLING_DELAY_MS = 2000;
constexpr uint8_t MAX_DOSING_CYCLES = 20;

enum control_phase_e {
    CONTROL_PHASE_READY,
    CONTROL_PHASE_DOSING,
    CONTROL_PHASE_WAIT_MIXING,
};

app_control_status_e control_status = APP_CONTROL_STATUS_WATER_OK;
control_phase_e control_phase = CONTROL_PHASE_READY;
uint32_t phase_started_ms = 0;
uint32_t outputs_stopped_at_ms = 0;
bool outputs_stopped_time_valid = false;
uint32_t pump_pulse_duration_ms = PUMP_PULSE_DURATION_HIGH_DUTY_MS;
bool control_initialized = false;
bool motor_running = false;
uint8_t dosing_cycles_count = 0;
bool pump_error_locked = false;
app_pump_status_e pump_status = APP_PUMP_STATUS_OFF;
float current_ph = NAN;

bool is_safe_ph(float ph) {
    return ph >= 6.0f && ph <= 8.0f;
}

bool elapsed(uint32_t duration_ms) {
    return static_cast<uint32_t>(millis() - phase_started_ms) >= duration_ms;
}

bool outputs_stopped(void) {
    return !motor_running && app_state.pwm_motor_percent == 0 &&
           app_state.pwm_pump_acid_percent == 0 &&
           app_state.pwm_pump_bazo_percent == 0;
}

bool adc_settling_elapsed(void) {
    return outputs_stopped_time_valid &&
           static_cast<uint32_t>(millis() - outputs_stopped_at_ms) >=
               ADC_SETTLING_DELAY_MS;
}

void stop_pumps(void) {
    drv_pwm_disable(PWM_ID_PUMP_ACID);
    drv_pwm_disable(PWM_ID_PUMP_BAZO);
    app_state.pwm_pump_acid_percent = 0;
    app_state.pwm_pump_bazo_percent = 0;
    if (!pump_error_locked) {
        pump_status = APP_PUMP_STATUS_OFF;
    }
}

void set_motor(bool enabled) {
    const bool was_running =
        motor_running || app_state.pwm_motor_percent != 0;
    drv_pwm_set_duty(PWM_ID_MOTOR, enabled ? PWM_DUTY_MOTOR : 0);
    app_state.pwm_motor_percent = enabled ? PWM_DUTY_MOTOR : 0;
    motor_running = enabled;
    if (!enabled && was_running &&
        app_state.pwm_pump_acid_percent == 0 &&
        app_state.pwm_pump_bazo_percent == 0) {
        outputs_stopped_at_ms = millis();
        outputs_stopped_time_valid = true;
    }
}

bool start_dosing(pwm_id_e pump, uint8_t duty,
                  app_control_status_e status) {
    if (dosing_cycles_count >= MAX_DOSING_CYCLES) {
        stop_pumps();
        set_motor(false);
        pump_error_locked = true;
        pump_status = APP_PUMP_STATUS_ERROR;
        control_phase = CONTROL_PHASE_READY;
        app_control_set_status(APP_CONTROL_STATUS_PUMP_ERROR);
        Serial.println("ERROR: Max dosing cycles reached! Pump locked for safety.");
        return false;
    }

    dosing_cycles_count++;
    pump_pulse_duration_ms =
        duty == PWM_DUTY_LOW ? PUMP_PULSE_DURATION_LOW_DUTY_MS
                             : PUMP_PULSE_DURATION_HIGH_DUTY_MS;
    stop_pumps();
    drv_pwm_set_duty(pump, duty);
    if (pump == PWM_ID_PUMP_ACID) {
        pump_status = APP_PUMP_STATUS_AXIT_ON;
        app_state.pwm_pump_acid_percent = duty;
    } else if (pump == PWM_ID_PUMP_BAZO) {
        pump_status = APP_PUMP_STATUS_BAZO_ON;
        app_state.pwm_pump_bazo_percent = duty;
    }

    if (!app_control_set_status(status)) {
        stop_pumps();
        return false;
    }

    control_phase = CONTROL_PHASE_DOSING;
    phase_started_ms = millis();
    return true;
}

bool begin_mixing(void) {
    stop_pumps();
    set_motor(true);
    control_phase = CONTROL_PHASE_WAIT_MIXING;
    phase_started_ms = millis();
    return app_control_set_status(APP_CONTROL_STATUS_MOTOR_ON);
}

void handle_invalid_ph(void) {
    stop_pumps();
    set_motor(false);
    control_phase = CONTROL_PHASE_READY;
    app_control_set_status(APP_CONTROL_STATUS_MOTOR_OFF);
}

} // namespace

app_pump_status_e app_control_get_pump_status(void) {
    return pump_status;
}

bool app_control_is_motor_running(void) {
    return motor_running;
}

bool app_control_set_status(app_control_status_e status) {
    if (status < APP_CONTROL_STATUS_MOTOR_ON ||
        status > APP_CONTROL_STATUS_PUMP_ERROR) {
        return false;
    }

    control_status = status;
    return true;
}

app_control_status_e app_control_get_status(void) {
    return control_status;
}

bool app_control_init(void) {
    control_initialized = false;
    motor_running = false;
    control_phase = CONTROL_PHASE_READY;
    current_ph = NAN;
    app_state.ph_updated = false;
    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;
    app_state.pwm_motor_percent = 0;
    app_state.pwm_pump_acid_percent = 0;
    app_state.pwm_pump_bazo_percent = 0;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);

    drv_pwm_init(PWM_ID_MOTOR, PWM_MOTOR_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    drv_pwm_init(PWM_ID_PUMP_ACID, PWM_PUMP_ACID_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    drv_pwm_init(PWM_ID_PUMP_BAZO, PWM_PUMP_BAZO_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);

    if (!pwm_inst[PWM_ID_MOTOR].sw_data.initialized ||
        !pwm_inst[PWM_ID_PUMP_ACID].sw_data.initialized ||
        !pwm_inst[PWM_ID_PUMP_BAZO].sw_data.initialized) {
        stop_pumps();
        drv_pwm_disable(PWM_ID_MOTOR);
        return false;
    }

    if (!mid_ph_init()) {
        stop_pumps();
        drv_pwm_disable(PWM_ID_MOTOR);
        return false;
    }

    stop_pumps();
    set_motor(false);
    outputs_stopped_at_ms = millis();
    outputs_stopped_time_valid = true;
    control_initialized = true;
    phase_started_ms = millis();
    dosing_cycles_count = 0;
    pump_error_locked = false;
    pump_status = APP_PUMP_STATUS_OFF;

    Serial.println(
        "WARNING: pH 4.01 calibration voltage is an estimate. "
        "Measure and update it before operating the pumps.");

    return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
}

bool app_control_update(void) {
    app_state.ph_updated = false;

    if (!control_initialized) {
        return false;
    }

    if (pump_error_locked) {
        stop_pumps();
        set_motor(false);
        app_control_set_status(APP_CONTROL_STATUS_PUMP_ERROR);
        return false;
    }

    if (control_phase == CONTROL_PHASE_DOSING) {
        if (!elapsed(pump_pulse_duration_ms)) {
            return true;
        }

        return begin_mixing();
    }

    if (control_phase == CONTROL_PHASE_WAIT_MIXING) {
        if (!elapsed(MIXING_DURATION_MS)) {
            return true;
        }

        set_motor(false);
        control_phase = CONTROL_PHASE_READY;
        phase_started_ms = millis();
        return true;
    }

    stop_pumps();
    set_motor(false);
    if (!outputs_stopped()) {
        return false;
    }
    if (!adc_settling_elapsed()) {
        return true;
    }

    const uint32_t voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
    app_state.adc_ph_raw = analogRead(ADC_PH_PIN);
    app_state.adc_ph_mv = voltage_mv == UINT32_MAX ? 0 : voltage_mv;
    app_state.voltage_ph_adc_v =
        voltage_mv == UINT32_MAX ? NAN : static_cast<float>(voltage_mv) / 1000.0f;
    app_state.voltage_ph_v = mid_ph_get_module_voltage(voltage_mv);
    current_ph = (voltage_mv == UINT32_MAX) ? NAN : mid_ph_read_ph(voltage_mv);
    app_state.ph = current_ph;
    app_state.ph_updated = true;
    Serial.printf("ADC PH raw=%u, pin=%lu mV, Po=%.3f V\n",
                  static_cast<unsigned int>(app_state.adc_ph_raw),
                  static_cast<unsigned long>(app_state.adc_ph_mv),
                  static_cast<double>(app_state.voltage_ph_v));
    const float ph = current_ph;

    if (!isfinite(ph)) {
        handle_invalid_ph();
        return false;
    }

    if (is_safe_ph(ph)) {
        dosing_cycles_count = 0;
        return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
    }

    const bool low_ph = ph < 6.0f;
    const bool high_speed = ph < 4.0f || ph > 10.0f;
    const uint8_t duty = high_speed ? PWM_DUTY_HIGH : PWM_DUTY_LOW;

    if (low_ph) {
        return start_dosing(PWM_ID_PUMP_BAZO, duty,
                            APP_CONTROL_STATUS_BAZO_PUMP_ON);
    }

    return start_dosing(PWM_ID_PUMP_ACID, duty,
                        APP_CONTROL_STATUS_AXIT_PUMP_ON);
}

float app_control_get_ph(void) {
    return current_ph;
}

void app_control_stop_all(void) {
    const bool outputs_were_running = !outputs_stopped();
    stop_pumps();
    set_motor(false);
    if (outputs_were_running && outputs_stopped()) {
        outputs_stopped_at_ms = millis();
        outputs_stopped_time_valid = true;
    }
    pump_status = APP_PUMP_STATUS_OFF;
    motor_running = false;
    control_phase = CONTROL_PHASE_READY;
    dosing_cycles_count = 0;
    pump_error_locked = false;
}