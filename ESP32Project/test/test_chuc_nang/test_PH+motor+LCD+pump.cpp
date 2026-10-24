#include "test_PH+motor+LCD+pump.h"

#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../src/app_state/app_state.h"
#include "drv_adc.h"
#include "drv_config.h"
#include "drv_i2c.h"
#include "drv_pwm.h"
#include "mid_lcd1602.h"
#include "mid_ph.h"

namespace {

constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;
constexpr uint8_t MOTOR_DUTY_PERCENT = 50;
constexpr uint8_t PUMP_DUTY_LOW_PERCENT = 30;
constexpr uint8_t PUMP_DUTY_HIGH_PERCENT = 70;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t MOTOR_START_DELAY_MS = 5000;
constexpr uint32_t PUMP_STEP_INTERVAL_MS = 5000;
constexpr size_t LCD_LINE_LENGTH = 16;
constexpr size_t LCD_LINE_BUFFER_SIZE = LCD_LINE_LENGTH + 1;

bool ph_initialized = false;
bool motor_initialized = false;
bool pump_initialized = false;
bool lcd_initialized = false;
bool motor_running = false;
uint32_t last_motor_start_ms = 0;
uint32_t last_pump_step_ms = 0;
uint8_t pump_step_index = 0;
bool lcd_line_cached[2] = {false, false};
char lcd_line_cache[2][LCD_LINE_BUFFER_SIZE] = {};

void make_lcd_line(char (&line)[LCD_LINE_BUFFER_SIZE], const char *text) {
    memset(line, ' ', LCD_LINE_LENGTH);
    line[LCD_LINE_LENGTH] = '\0';

    if (text != nullptr) {
        const size_t text_length = strlen(text);
        const size_t copy_length =
            text_length < LCD_LINE_LENGTH ? text_length : LCD_LINE_LENGTH;
        memcpy(line, text, copy_length);
    }
}

bool write_lcd_line(uint16_t row,
                    const char (&line)[LCD_LINE_BUFFER_SIZE]) {
    if (lcd_line_cached[row] &&
        memcmp(lcd_line_cache[row], line, LCD_LINE_LENGTH) == 0) {
        return true;
    }

    if (LCD1602_SetCursor(row, 0) != LCD_STATUS_OK ||
        LCD1602_Print(line) != LCD_STATUS_OK) {
        return false;
    }

    memcpy(lcd_line_cache[row], line, LCD_LINE_BUFFER_SIZE);
    lcd_line_cached[row] = true;
    return true;
}

bool update_lcd(void) {
    if (!lcd_initialized) {
        return false;
    }

    char ph_text[LCD_LINE_BUFFER_SIZE];
    char status_text[LCD_LINE_BUFFER_SIZE];
    char ph_line[LCD_LINE_BUFFER_SIZE];
    char status_line[LCD_LINE_BUFFER_SIZE];

    if (isfinite(app_state.ph)) {
        snprintf(ph_text, sizeof(ph_text), "pH: %.2f",
                 static_cast<double>(app_state.ph));
    } else {
        snprintf(ph_text, sizeof(ph_text), "pH: --");
    }

    if (motor_running) {
        snprintf(status_text, sizeof(status_text), "M:%d%%, P:%d%%",
                 app_state.pwm_motor_percent,
                 app_state.pwm_pump_acid_percent);
    } else {
        snprintf(status_text, sizeof(status_text), "M:OFF, P:%d%%",
                 app_state.pwm_pump_acid_percent);
    }

    make_lcd_line(ph_line, ph_text);
    make_lcd_line(status_line, status_text);
    return write_lcd_line(0, ph_line) && write_lcd_line(1, status_line);
}

uint8_t pump_step_duty(uint8_t step_index) {
    switch (step_index % 3) {
        case 0:
            return PUMP_DUTY_LOW_PERCENT;
        case 1:
            return PUMP_DUTY_HIGH_PERCENT;
        case 2:
        default:
            return 0;
    }
}

} // namespace

bool test_PH_motor_LCD_pump_init(void) {
    ph_initialized = false;
    motor_initialized = false;
    pump_initialized = false;
    lcd_initialized = false;
    motor_running = false;
    last_motor_start_ms = 0;
    last_pump_step_ms = 0;
    pump_step_index = 0;
    lcd_line_cached[0] = false;
    lcd_line_cached[1] = false;

    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;
    app_state.pwm_motor_percent = 0;
    app_state.pwm_pump_acid_percent = 0;
    app_state.display_ready = false;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);
    ph_initialized = mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE);

    drv_pwm_init(PWM_ID_MOTOR, PWM_MOTOR_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    motor_initialized = pwm_inst[PWM_ID_MOTOR].sw_data.initialized;

    drv_pwm_init(PWM_ID_PUMP_ACID, PWM_PUMP_ACID_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    pump_initialized = pwm_inst[PWM_ID_PUMP_ACID].sw_data.initialized;

    drv_i2c_init(I2C_ID_LCD1602, I2C_SDA_PIN, I2C_SCL_PIN);
    lcd_initialized = LCD1602_Init() == LCD_STATUS_OK;
    app_state.display_ready = lcd_initialized;

    if (motor_initialized) {
        drv_pwm_set_duty(PWM_ID_MOTOR, 0);
        app_state.pwm_motor_percent = 0;
    }
    if (pump_initialized) {
        drv_pwm_set_duty(PWM_ID_PUMP_ACID, 0);
        app_state.pwm_pump_acid_percent = 0;
    }

    return ph_initialized && motor_initialized && pump_initialized && lcd_initialized;
}

bool test_PH_motor_LCD_pump_update(void) {
    if (!ph_initialized || !motor_initialized || !pump_initialized || !lcd_initialized) {
        return false;
    }

    const uint32_t voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
    app_state.adc_ph_raw = analogRead(ADC_PH_PIN);

    bool ph_read_ok = (voltage_mv != UINT32_MAX);
    if (ph_read_ok) {
        app_state.adc_ph_mv = voltage_mv;
        app_state.voltage_ph_adc_v = static_cast<float>(voltage_mv) / 1000.0f;
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
    if (last_motor_start_ms == 0) {
        last_motor_start_ms = now;
    }

    if (static_cast<uint32_t>(now - last_motor_start_ms) >= MOTOR_START_DELAY_MS) {
        motor_running = true;
        drv_pwm_set_duty(PWM_ID_MOTOR, MOTOR_DUTY_PERCENT);
        app_state.pwm_motor_percent = MOTOR_DUTY_PERCENT;
    } else {
        motor_running = false;
        drv_pwm_set_duty(PWM_ID_MOTOR, 0);
        app_state.pwm_motor_percent = 0;
    }

    if (static_cast<uint32_t>(now - last_pump_step_ms) >= PUMP_STEP_INTERVAL_MS) {
        pump_step_index = (pump_step_index + 1) % 3;
        const uint8_t pump_duty = pump_step_duty(pump_step_index);
        drv_pwm_set_duty(PWM_ID_PUMP_ACID, pump_duty);
        app_state.pwm_pump_acid_percent = pump_duty;
        last_pump_step_ms = now;
    }

    const bool lcd_ok = update_lcd();
    return ph_read_ok && lcd_ok;
}
