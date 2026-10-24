#include "test_PH+LCD.h"

#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../src/app_state/app_state.h"
#include "drv_adc.h"
#include "drv_config.h"
#include "mid_ph.h"
#include "mid_lcd1602.h"
#include "drv_i2c.h"

namespace {

constexpr float PH_4_01_CALIBRATION_VOLTAGE = 3.00f;
constexpr size_t LCD_LINE_LENGTH = 16;
constexpr size_t LCD_LINE_BUFFER_SIZE = LCD_LINE_LENGTH + 1;

bool ph_initialized = false;
bool lcd_initialized = false;

void make_lcd_line(char (&line)[LCD_LINE_BUFFER_SIZE], const char *text) {
    memset(line, ' ', LCD_LINE_LENGTH);
    line[LCD_LINE_LENGTH] = '\0';
    snprintf(line, LCD_LINE_BUFFER_SIZE, "%-16.16s", text);
}

} // namespace

bool test_PH_LCD_init(void) {
    ph_initialized = false;
    lcd_initialized = false;

    app_state.adc_ph_raw = 0;
    app_state.adc_ph_mv = 0;
    app_state.voltage_ph_adc_v = NAN;
    app_state.voltage_ph_v = NAN;
    app_state.ph = NAN;

    drv_adc_init(ADC_ID_PH, ADC_PH_PIN);
    ph_initialized = mid_ph_init(PH_4_01_CALIBRATION_VOLTAGE);

    drv_i2c_init(I2C_ID_LCD1602, I2C_SDA_PIN, I2C_SCL_PIN);
    lcd_initialized = LCD1602_Init() == LCD_STATUS_OK;
    app_state.display_ready = lcd_initialized;

    return ph_initialized && lcd_initialized;
}

bool test_PH_LCD_update(void) {
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
        } else {
            app_state.adc_ph_mv = 0;
            app_state.voltage_ph_adc_v = NAN;
            app_state.voltage_ph_v = NAN;
            app_state.ph = NAN;
        }
    } else {
        app_state.ph = NAN;
    }

    bool lcd_update_ok = false;
    if (lcd_initialized) {
        char ph_text[LCD_LINE_BUFFER_SIZE];
        char adc_text[LCD_LINE_BUFFER_SIZE];
        char ph_line[LCD_LINE_BUFFER_SIZE];
        char adc_line[LCD_LINE_BUFFER_SIZE];

        if (isfinite(app_state.ph)) {
            snprintf(ph_text, sizeof(ph_text), "pH: %.2f",
                     static_cast<double>(app_state.ph));
        } else {
            snprintf(ph_text, sizeof(ph_text), "pH: --");
        }
        if (app_state.adc_ph_mv > 0) {
            snprintf(adc_text, sizeof(adc_text), "ADC: %lu mV",
                     static_cast<unsigned long>(app_state.adc_ph_mv));
        } else {
            snprintf(adc_text, sizeof(adc_text), "ADC: ERROR");
        }

        make_lcd_line(ph_line, ph_text);
        make_lcd_line(adc_line, adc_text);
        lcd_update_ok =
            LCD1602_SetCursor(0, 0) == LCD_STATUS_OK &&
            LCD1602_Print(ph_line) == LCD_STATUS_OK &&
            LCD1602_SetCursor(1, 0) == LCD_STATUS_OK &&
            LCD1602_Print(adc_line) == LCD_STATUS_OK;
    }

    return ph_read_ok && lcd_update_ok;
}
