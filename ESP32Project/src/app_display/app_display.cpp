#include "app_display.h"

#include "../app_control/app_control.h"

namespace {

constexpr size_t LCD_LINE_LENGTH = 16;
constexpr size_t LCD_LINE_BUFFER_SIZE = LCD_LINE_LENGTH + 1;

bool display_initialized = false;
bool line_cached[2] = {false, false};
char line_cache[2][LCD_LINE_BUFFER_SIZE] = {};

void make_line(char (&line)[LCD_LINE_BUFFER_SIZE], const char *text) {
    memset(line, ' ', LCD_LINE_LENGTH);
    line[LCD_LINE_LENGTH] = '\0';

    if (text != nullptr) {
        const size_t text_length = strlen(text);
        const size_t copy_length =
            text_length < LCD_LINE_LENGTH ? text_length : LCD_LINE_LENGTH;
        memcpy(line, text, copy_length);
    }
}

bool write_line(uint16_t row, const char (&line)[LCD_LINE_BUFFER_SIZE]) {
    if (line_cached[row] &&
        memcmp(line_cache[row], line, LCD_LINE_LENGTH) == 0) {
        return true;
    }

    if (LCD1602_SetCursor(row, 0) != LCD_STATUS_OK ||
        LCD1602_Print(line) != LCD_STATUS_OK) {
        return false;
    }

    memcpy(line_cache[row], line, LCD_LINE_BUFFER_SIZE);
    line_cached[row] = true;
    return true;
}

} // namespace

bool app_display_init(void) {
    display_initialized = false;
    line_cached[0] = false;
    line_cached[1] = false;

    drv_i2c_init(I2C_ID_LCD1602, I2C_SDA_PIN, I2C_SCL_PIN);
    if (LCD1602_Init() != LCD_STATUS_OK) {
        return false;
    }

    display_initialized = true;
    return true;
}

bool app_display_update(float ph) {
    if (!display_initialized) {
        return false;
    }

    char ph_text[LCD_LINE_BUFFER_SIZE];
    if (isfinite(ph)) {
        const float display_ph =
            ph <= 0.0f ? 0.0f : (ph >= 14.0f ? 14.0f : ph);
        snprintf(ph_text, sizeof(ph_text), "pH: %.2f",
                 static_cast<double>(display_ph));
    } else {
        snprintf(ph_text, sizeof(ph_text), "pH: --");
    }

    char raw_status[32];
    if (!isfinite(ph)) {
        snprintf(raw_status, sizeof(raw_status), "SENSOR ERROR");
    } else {
        const app_pump_status_e pump_st = app_control_get_pump_status();
        const bool motor_on = app_control_is_motor_running();

        const char *pump_str = "P:OFF     ";
        switch (pump_st) {
            case APP_PUMP_STATUS_AXIT_ON:
                pump_str = "P:AXIT ON ";
                break;
            case APP_PUMP_STATUS_BAZO_ON:
                pump_str = "P:BAZO ON ";
                break;
            case APP_PUMP_STATUS_ERROR:
                pump_str = "P:ERROR   ";
                break;
            case APP_PUMP_STATUS_OFF:
            default:
                pump_str = "P:OFF     ";
                break;
        }

        const char *motor_str = motor_on ? " M:ON " : " M:OFF";
        snprintf(raw_status, sizeof(raw_status), "%s%s", pump_str, motor_str);
    }

    char ph_line[LCD_LINE_BUFFER_SIZE];
    char status_line[LCD_LINE_BUFFER_SIZE];

    make_line(ph_line, ph_text);
    make_line(status_line, raw_status);

    return write_line(0, ph_line) && write_line(1, status_line);
}