#ifndef DRV_CONFIG_H_
#define DRV_CONFIG_H_

#include "Arduino.h"

typedef enum {
    PIN_GPIO_TEST, //tên id gpio cần test
    GPIO_ID_MAX,
} gpio_id_e;

typedef enum {
    ADC_ID_A,
    ADC_ID_MAX
} adc_id_e;

typedef enum {
    I2C_ID_A,
    I2C_ID_MAX
} i2c_id_e;

#endif // DRV_CONFIG_H_