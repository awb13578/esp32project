#include "test_PH+motor+LCD.h"

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
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY_HZ = 1000;
constexpr uint32_t MOTOR_TOGGLE_INTERVAL_MS = 5000;
constexpr size_t LCD_LINE_LENGTH = 16;
constexpr size_t LCD_LINE_BUFFER_SIZE = LCD_LINE_LENGTH + 1;

bool ph_initialized = false;
bool motor_initialized = false;
bool lcd_initialized = false;
bool motor_running = false;
uint32_t last_motor_toggle_ms = 0;
bool lcd_line_cached[2] = {false, false};
char lcd_line_cache[2][LCD_LINE_BUFFER_SIZE] = {};

void make_lcd_line(char (&line)[LCD_LINE_BUFFER_SIZE], const char *text) {
    memset(line, ' ', LCD_LINE_LENGTH);
    line[LCD_LINE_LENGTH] = '\0';

    const size_t text_length = strlen(text);
    const size_t copy_length =
        text_length < LCD_LINE_LENGTH ? text_length : LCD_LINE_LENGTH;
    memcpy(line, text, copy_length);
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
    char motor_text[LCD_LINE_BUFFER_SIZE];
    char ph_line[LCD_LINE_BUFFER_SIZE];
    char motor_line[LCD_LINE_BUFFER_SIZE];

    if (isfinite(app_state.ph)) {
        snprintf(ph_text, sizeof(ph_text), "pH: %.2f",
                 static_cast<double>(app_state.ph));
    } else {
        snprintf(ph_text, sizeof(ph_text), "pH: --");
    }
    snprintf(motor_text, sizeof(motor_text), "Motor: %s %u%%",
             motor_running ? "ON" : "OFF",
             static_cast<unsigned int>(app_state.pwm_motor_percent));

    make_lcd_line(ph_line, ph_text);
    make_lcd_line(motor_line, motor_text);

    return write_lcd_line(0, ph_line) && write_lcd_line(1, motor_line);
}

} // namespace

bool test_PH_motor_LCD_init(void) {
    ph_initialized = false;
    motor_initialized = false;
    lcd_initialized = false;
    motor_running = false;
    lcd_line_cached[0] = false;
    lcd_line_cached[1] = false;

    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;
    app_state.pwm_motor_percent = 0;
    app_state.display_ready = false;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);
    ph_initialized = mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE);

    drv_pwm_init(PWM_ID_MOTOR, PWM_MOTOR_PIN, PWM_FREQUENCY_HZ,
                 PWM_RESOLUTION);
    motor_initialized = pwm_inst[PWM_ID_MOTOR].sw_data.initialized;
    if (motor_initialized) {
        drv_pwm_set_duty(PWM_ID_MOTOR, MOTOR_DUTY_PERCENT);
        motor_running = true;
        app_state.pwm_motor_percent = MOTOR_DUTY_PERCENT;
        last_motor_toggle_ms = millis();
    }

    drv_i2c_init(I2C_ID_LCD1602, I2C_SDA_PIN, I2C_SCL_PIN);
    lcd_initialized = LCD1602_Init() == LCD_STATUS_OK;
    app_state.display_ready = lcd_initialized;

    return ph_initialized && motor_initialized && lcd_initialized;
}

bool test_PH_motor_LCD_update(void) {
    bool ph_read_ok = false;

    if (ph_initialized) {
        const uint32_t voltage_mv = drv_adc_read_millivolts(ADC_ID_PH);
        app_state.adc_ph_raw = analogRead(ADC_PH_PIN);

        if (voltage_mv != UINT32_MAX) {
            app_state.adc_ph_mv = voltage_mv;
            app_state.voltage_ph_adc_v =
                static_cast<float>(voltage_mv) / 1000.0f;
            app_state.voltage_ph_v = app_state.voltage_ph_adc_v * 2.0f;
            app_state.ph = mid_ph_read_ph(voltage_mv);
            ph_read_ok = isfinite(app_state.ph);
        }
    }

    if (!ph_read_ok) {
        app_state.adc_ph_mv = 0;
        app_state.voltage_ph_adc_v = NAN;
        app_state.voltage_ph_v = NAN;
        app_state.ph = NAN;
    }

    if (motor_initialized &&
        static_cast<uint32_t>(millis() - last_motor_toggle_ms) >=
            MOTOR_TOGGLE_INTERVAL_MS) {
        motor_running = !motor_running;
        const uint8_t duty = motor_running ? MOTOR_DUTY_PERCENT : 0;
        drv_pwm_set_duty(PWM_ID_MOTOR, duty);
        app_state.pwm_motor_percent = duty;
        last_motor_toggle_ms = millis();
    }

    const bool lcd_update_ok = update_lcd();
    return ph_read_ok && motor_initialized && lcd_update_ok;
}
