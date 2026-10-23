#include "app_control.h"

#include <Arduino.h>
#include <math.h>

#include "drv_adc.h"
#include "drv_config.h"
#include "drv_pwm.h"
#include "mid_ph.h"
#include "../app_state/app_state.h"

namespace {

// Measured module output voltages before the divider; update after recalibration.
constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;
constexpr float PH_7_00_CALIBRATION_VOLTAGE = 2.50f;
constexpr uint32_t LOAD_SETTLE_TIME_MS = 300;
constexpr float PH_VOLTAGE_FILTER_ALPHA = 0.25f;

constexpr uint8_t PWM_DUTY_LOW = 30;
constexpr uint8_t PWM_DUTY_HIGH = 70;
constexpr uint8_t PWM_DUTY_MOTOR = 80;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t PUMP_PULSE_DURATION_LOW_DUTY_MS = 300;
constexpr uint32_t PUMP_PULSE_DURATION_HIGH_DUTY_MS = 500;
constexpr uint32_t MIXING_DURATION_MS = 3000;
constexpr uint32_t SAFE_MIXING_DURATION_MS = 5000;
constexpr uint8_t MAX_DOSING_CYCLES = 20;

enum control_phase_e {
    CONTROL_PHASE_READY,
    CONTROL_PHASE_DOSING,
    CONTROL_PHASE_WAIT_MIXING,
    CONTROL_PHASE_CONFIRMING_SAFE,
    CONTROL_PHASE_SAFE,
};

app_control_status_e control_status = APP_CONTROL_STATUS_WATER_OK;
control_phase_e control_phase = CONTROL_PHASE_READY;
uint32_t phase_started_ms = 0;
uint32_t safe_ph_started_ms = 0;
uint32_t pump_pulse_duration_ms = PUMP_PULSE_DURATION_HIGH_DUTY_MS;
bool control_initialized = false;
bool motor_running = false;
bool safe_ph_timer_active = false;
uint8_t dosing_cycles_count = 0;
bool pump_error_locked = false;
app_pump_status_e pump_status = APP_PUMP_STATUS_OFF;
float current_ph = NAN;
float filtered_voltage_mv = NAN;
uint8_t previous_load_mask = 0;
uint32_t load_changed_ms = 0;
bool load_state_initialized = false;

bool is_safe_ph(float ph) {
    return ph >= 6.0f && ph <= 8.0f;
}

bool elapsed(uint32_t duration_ms) {
    return static_cast<uint32_t>(millis() - phase_started_ms) >= duration_ms;
}

uint8_t active_load_mask(void) {
    return static_cast<uint8_t>((motor_running ? 0x01 : 0x00) |
                                (app_state.pwm_pump_acid_percent > 0 ? 0x02 : 0x00) |
                                (app_state.pwm_pump_bazo_percent > 0 ? 0x04 : 0x00));
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
    drv_pwm_set_duty(PWM_ID_MOTOR, enabled ? PWM_DUTY_MOTOR : 0);
    app_state.pwm_motor_percent = enabled ? PWM_DUTY_MOTOR : 0;
    motor_running = enabled;
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
    set_motor(true);

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
    safe_ph_timer_active = false;
    control_phase = CONTROL_PHASE_READY;
    current_ph = NAN;
    filtered_voltage_mv = NAN;
    previous_load_mask = 0;
    load_changed_ms = millis();
    load_state_initialized = false;
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

    if (!mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE,
                     PH_7_00_CALIBRATION_VOLTAGE)) {
        stop_pumps();
        drv_pwm_disable(PWM_ID_MOTOR);
        return false;
    }

    stop_pumps();
    set_motor(false);
    control_initialized = true;
    phase_started_ms = millis();
    safe_ph_timer_active = false;
    dosing_cycles_count = 0;
    pump_error_locked = false;
    pump_status = APP_PUMP_STATUS_OFF;

    Serial.printf("pH calibration: pH 4.01=%.3f V, pH 7.00=%.3f V "
                  "(module output before divider)\n",
                  PH_4_01_CALIBRATION_VOLTAGE,
                  PH_7_00_CALIBRATION_VOLTAGE);

    return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
}

bool app_control_update(void) {
    if (!control_initialized) {
        return false;
    }

    if (pump_error_locked) {
        stop_pumps();
        set_motor(false);
        app_control_set_status(APP_CONTROL_STATUS_PUMP_ERROR);
        return false;
    }

    const uint32_t measured_voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
    app_state.adc_ph_raw = analogRead(ADC_PH_PIN);
    const uint8_t load_mask = active_load_mask();
    if (!load_state_initialized) {
        previous_load_mask = load_mask;
        load_changed_ms = millis() - LOAD_SETTLE_TIME_MS;
        load_state_initialized = true;
    } else if (load_mask != previous_load_mask) {
        previous_load_mask = load_mask;
        load_changed_ms = millis();
    }

    const bool settling_after_load_change =
        static_cast<uint32_t>(millis() - load_changed_ms) < LOAD_SETTLE_TIME_MS;
    const bool pump_active = (load_mask & 0x06) != 0;
    const bool can_update_measurement =
        measured_voltage_mv != UINT32_MAX && !pump_active &&
        !settling_after_load_change;

    if (can_update_measurement) {
        const float measured_mv = static_cast<float>(measured_voltage_mv);
        if (!isfinite(filtered_voltage_mv)) {
            filtered_voltage_mv = measured_mv;
        } else {
            filtered_voltage_mv +=
                PH_VOLTAGE_FILTER_ALPHA * (measured_mv - filtered_voltage_mv);
        }
    }

    const bool have_valid_measurement = isfinite(filtered_voltage_mv);
    const float measured_adc_voltage_v =
        measured_voltage_mv == UINT32_MAX
            ? NAN
            : static_cast<float>(measured_voltage_mv) / 1000.0f;
    app_state.adc_ph_mv = have_valid_measurement
                              ? static_cast<uint32_t>(filtered_voltage_mv)
                              : 0;
    app_state.voltage_ph_adc_v =
        have_valid_measurement ? filtered_voltage_mv / 1000.0f : NAN;
    app_state.voltage_ph_v =
        have_valid_measurement ? app_state.voltage_ph_adc_v * 2.0f : NAN;
    if (have_valid_measurement) {
        current_ph = mid_ph_read_ph(app_state.adc_ph_mv);
    }
    app_state.ph = current_ph;
    const float ph = current_ph;

    Serial.printf(
        "pH debug | Raw ADC=%u | Measured ADC=%.3f V | ADC voltage=%.3f V | "
        "Sensor voltage=%.3f V | Calculated pH=%.2f | Sample=%s\n",
        static_cast<unsigned int>(app_state.adc_ph_raw),
        measured_adc_voltage_v, app_state.voltage_ph_adc_v,
        app_state.voltage_ph_v, ph,
        can_update_measurement ? "filtered" : "held");

    if (!have_valid_measurement || !isfinite(ph)) {
        handle_invalid_ph();
        safe_ph_timer_active = false;
        return false;
    }

    if (motor_running && is_safe_ph(ph)) {
        if (!safe_ph_timer_active) {
            safe_ph_started_ms = millis();
            safe_ph_timer_active = true;
        }
    } else {
        safe_ph_timer_active = false;
    }

    if (control_phase == CONTROL_PHASE_DOSING) {
        if (!elapsed(pump_pulse_duration_ms)) {
            return true;
        }

        if (!begin_mixing()) {
            return false;
        }
        return true;
    }

    if (control_phase == CONTROL_PHASE_WAIT_MIXING) {
        if (!elapsed(MIXING_DURATION_MS)) {
            return true;
        }

        control_phase = CONTROL_PHASE_READY;
    }

    if (control_phase == CONTROL_PHASE_CONFIRMING_SAFE) {
        if (!is_safe_ph(ph)) {
            control_phase = CONTROL_PHASE_READY;
        } else if (!safe_ph_timer_active ||
                   static_cast<uint32_t>(millis() - safe_ph_started_ms) <
                       SAFE_MIXING_DURATION_MS) {
            return true;
        } else {
            set_motor(false);
            control_phase = CONTROL_PHASE_SAFE;
            return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
        }
    }

    if (control_phase == CONTROL_PHASE_SAFE && is_safe_ph(ph)) {
        dosing_cycles_count = 0;
        pump_error_locked = false;
        stop_pumps();
        set_motor(false);
        return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
    }

    if (is_safe_ph(ph)) {
        dosing_cycles_count = 0;
        pump_error_locked = false;
        stop_pumps();
        if (motor_running) {
            if (safe_ph_timer_active &&
                static_cast<uint32_t>(millis() - safe_ph_started_ms) >=
                    SAFE_MIXING_DURATION_MS) {
                set_motor(false);
                control_phase = CONTROL_PHASE_SAFE;
                return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
            }

            set_motor(true);
            control_phase = CONTROL_PHASE_CONFIRMING_SAFE;
            return app_control_set_status(APP_CONTROL_STATUS_MOTOR_ON);
        }

        control_phase = CONTROL_PHASE_SAFE;
        return app_control_set_status(APP_CONTROL_STATUS_WATER_OK);
    }

    const bool low_ph = ph < 6.0f;
    const bool high_speed = ph < 4.5f || ph > 9.5f;
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
    stop_pumps();
    set_motor(false);
    pump_status = APP_PUMP_STATUS_OFF;
    motor_running = false;
    control_phase = CONTROL_PHASE_READY;
    dosing_cycles_count = 0;
    pump_error_locked = false;
}