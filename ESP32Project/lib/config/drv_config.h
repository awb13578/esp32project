#ifndef DRV_CONFIG_H_
#define DRV_CONFIG_H_

#include "Arduino.h"

typedef enum {
    GPIO_ID_A, //tên id gpio cần test
    GPIO_ID_MAX,
} gpio_id_e;

typedef enum {
    ADC_ID_PH,
    ADC_ID_MAX,
} adc_id_e;

typedef enum {
    PWM_ID_MOTOR,
    PWM_ID_PUMP1,
    PWM_ID_PUMP2,
    PWM_ID_MAX,
} pwm_id_e;

typedef enum {
    I2C_ID_LCD1602,
    I2C_ID_MAX,
} i2c_id_e;

#endif // DRV_CONFIG_H_