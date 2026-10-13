#include "mid_lcd1602.h"

#ifdef ENABLE_I2C

#include <Arduino.h>

namespace {

constexpr uint8_t LCD_BACKLIGHT = 0x08;
constexpr uint8_t LCD_ENABLE = 0x04;
constexpr uint8_t LCD_RS_COMMAND = 0x00;
constexpr uint8_t LCD_RS_DATA = 0x01;

bool lcd_initialized = false;

lcd_status_e lcd_i2c_write(uint8_t value) {
    return drv_i2c_send(I2C_ID_A, LCD_ADDR, &value) == I2C_STATUS_OK
        ? LCD_STATUS_OK
        : LCD_STATUS_ERROR;
}

lcd_status_e lcd_send_nibble(uint8_t nibble, uint8_t rs_mode) {
    const uint8_t data_en_high = static_cast<uint8_t>(
        (nibble & 0xF0) | rs_mode | LCD_BACKLIGHT | LCD_ENABLE);
    const uint8_t data_en_low = static_cast<uint8_t>(
        (nibble & 0xF0) | rs_mode | LCD_BACKLIGHT);

    if (lcd_i2c_write(data_en_high) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }
    delayMicroseconds(10);

    if (lcd_i2c_write(data_en_low) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }
    delayMicroseconds(50);
    return LCD_STATUS_OK;
}

lcd_status_e lcd_send_byte(uint8_t value, uint8_t rs_mode) {
    if (lcd_send_nibble(value, rs_mode) != LCD_STATUS_OK ||
        lcd_send_nibble(static_cast<uint8_t>(value << 4), rs_mode) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }

    if (rs_mode == LCD_RS_COMMAND && (value == 0x01 || value == 0x02)) {
        delayMicroseconds(2000);
    }
    return LCD_STATUS_OK;
}

} // namespace

lcd_status_e LCD1602_Init(void) {
    lcd_initialized = false;
    delay(50);

    if (lcd_send_nibble(0x30, LCD_RS_COMMAND) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }
    delay(5);
    if (lcd_send_nibble(0x30, LCD_RS_COMMAND) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }
    delayMicroseconds(200);
    if (lcd_send_nibble(0x30, LCD_RS_COMMAND) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }
    delayMicroseconds(200);

    if (lcd_send_nibble(0x20, LCD_RS_COMMAND) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }
    delayMicroseconds(1000);

    if (lcd_send_byte(0x28, LCD_RS_COMMAND) != LCD_STATUS_OK ||
        lcd_send_byte(0x0C, LCD_RS_COMMAND) != LCD_STATUS_OK ||
        lcd_send_byte(0x06, LCD_RS_COMMAND) != LCD_STATUS_OK ||
        lcd_send_byte(0x01, LCD_RS_COMMAND) != LCD_STATUS_OK) {
        return LCD_STATUS_ERROR;
    }

    lcd_initialized = true;
    return LCD_STATUS_OK;
}

lcd_status_e LCD1602_SendCmd(uint16_t cmd) {
    if (!lcd_initialized || cmd > 0xFF) {
        return LCD_STATUS_ERROR;
    }
    return lcd_send_byte(static_cast<uint8_t>(cmd), LCD_RS_COMMAND);
}

lcd_status_e LCD1602_SendData(uint16_t data) {
    if (!lcd_initialized || data > 0xFF) {
        return LCD_STATUS_ERROR;
    }
    return lcd_send_byte(static_cast<uint8_t>(data), LCD_RS_DATA);
}

lcd_status_e LCD1602_SetCursor(uint16_t row, uint16_t col) {
    static constexpr uint8_t row_offsets[] = {0x80, 0xC0};
    if (!lcd_initialized) {
        return LCD_STATUS_ERROR;
    }

    if (row > 1) {
        row = 1;
    }
    if (col > 15) {
        col = 15;
    }
    return LCD1602_SendCmd(static_cast<uint16_t>(row_offsets[row] + col));
}

lcd_status_e LCD1602_Print(const char *str) {
    if (!lcd_initialized || str == nullptr) {
        return LCD_STATUS_ERROR;
    }

    while (*str != '\0') {
        if (LCD1602_SendData(static_cast<uint8_t>(*str++)) != LCD_STATUS_OK) {
            return LCD_STATUS_ERROR;
        }
    }
    return LCD_STATUS_OK;
}

lcd_status_e LCD1602_Clear(void) {
    return LCD1602_SendCmd(0x01);
}

#endif // ENABLE_I2C
