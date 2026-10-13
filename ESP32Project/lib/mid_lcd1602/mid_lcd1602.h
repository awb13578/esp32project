#ifndef MID_LCD1602_H_
#define MID_LCD1602_H_

#include "drv_i2c.h"

#ifdef ENABLE_I2C

#define LCD_ADDR 0x27

typedef enum {
    LCD_STATUS_ERROR,
    LCD_STATUS_OK,
} lcd_status_e;

lcd_status_e LCD1602_Init(void);
lcd_status_e LCD1602_SendCmd(uint16_t cmd);
lcd_status_e LCD1602_SendData(uint16_t data);
lcd_status_e LCD1602_SetCursor(uint16_t row, uint16_t col);
lcd_status_e LCD1602_Print(const char *str);
lcd_status_e LCD1602_Clear(void);

#endif // ENABLE_I2C
#endif // MID_LCD1602_H_
