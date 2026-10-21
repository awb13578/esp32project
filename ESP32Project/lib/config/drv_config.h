#ifndef DRV_CONFIG_H_
#define DRV_CONFIG_H_

#include "Arduino.h"

#define GPIO_TEST_PIN 

#define ADC_PH_PIN 2

#define PWM_MOTOR_PIN 5
#define PWM_PUMP_ACID_PIN 6
#define PWM_PUMP_BAZO_PIN 7

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

typedef enum {
    GPIO_ID_TEST, //tên id gpio cần test
    GPIO_ID_MAX,
} gpio_id_e;

typedef enum {
    ADC_ID_PH,
    ADC_ID_MAX,
} adc_id_e;

typedef enum {
    PWM_ID_MOTOR,
    PWM_ID_PUMP_ACID,
    PWM_ID_PUMP_BAZO,
    PWM_ID_MAX,
} pwm_id_e;

typedef enum {
    I2C_ID_LCD1602,
    I2C_ID_MAX,
} i2c_id_e;

#endif // DRV_CONFIG_H_