#include "app_control.h"

#include <Arduino.h>
#include <math.h>

#include "drv_adc.h"
#include "drv_config.h"
#include "drv_pwm.h"
#include "mid_ph.h"

namespace {

// Replace with the measured module output voltage in pH 4.01 buffer.
constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;

constexpr uint8_t PWM_DUTY_LOW = 30;
constexpr uint8_t PWM_DUTY_HIGH = 80;
constexpr uint8_t PWM_DUTY_MOTOR = 100;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t PUMP_PULSE_DURATION_MS = 500;
constexpr uint32_t MIXING_DURATION_MS = 3000;
constexpr uint32_t SAFE_MIXING_DURATION_MS = 3000;
constexpr uint8_t MAX_DOSING_CYCLES = 10;

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
bool control_initialized = false;
bool motor_running = false;
bool safe_ph_timer_active = false;
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

void stop_pumps(void) {
    drv_pwm_disable(PWM_ID_PUMP1);
    drv_pwm_disable(PWM_ID_PUMP2);
    if (!pump_error_locked) {
        pump_status = APP_PUMP_STATUS_OFF;
    }
}

void set_motor(bool enabled) {
    drv_pwm_set_duty(PWM_ID_MOTOR, enabled ? PWM_DUTY_MOTOR : 0);
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
    stop_pumps();
    drv_pwm_set_duty(pump, duty);
    if (pump == PWM_ID_PUMP1) {
        pump_status = APP_PUMP_STATUS_AXIT_ON;
    } else if (pump == PWM_ID_PUMP2) {
        pump_status = APP_PUMP_STATUS_BAZO_ON;
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

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);

    drv_pwm_init(PWM_ID_MOTOR, PWM_MOTOR_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    drv_pwm_init(PWM_ID_PUMP1, PWM_PUMP1_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    drv_pwm_init(PWM_ID_PUMP2, PWM_PUMP2_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);

    if (!pwm_inst[PWM_ID_MOTOR].sw_data.initialized ||
        !pwm_inst[PWM_ID_PUMP1].sw_data.initialized ||
        !pwm_inst[PWM_ID_PUMP2].sw_data.initialized) {
        stop_pumps();
        drv_pwm_disable(PWM_ID_MOTOR);
        return false;
    }

    if (!mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE)) {
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

    Serial.println(
        "WARNING: pH 4.01 calibration voltage is an estimate (3.00 V). "
        "Measure and update it before operating the pumps.");

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

    const uint32_t voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
    current_ph = (voltage_mv == UINT32_MAX) ? NAN : mid_ph_read_ph(voltage_mv);
    const float ph = current_ph;

    if (!isfinite(ph)) {
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
        if (!elapsed(PUMP_PULSE_DURATION_MS)) {
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
    const bool high_speed = ph < 5.5f || ph > 8.5f;
    const uint8_t duty = high_speed ? PWM_DUTY_HIGH : PWM_DUTY_LOW;

    if (low_ph) {
        return start_dosing(PWM_ID_PUMP2, duty,
                            APP_CONTROL_STATUS_BAZO_PUMP_ON);
    }

    return start_dosing(PWM_ID_PUMP1, duty,
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