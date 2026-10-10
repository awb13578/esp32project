#ifndef DRV_I2C_H_
#define DRV_I2C_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_I2C

#define TxLength 1

typedef enum {
    I2C_STATUS_ERROR,
    I2C_STATUS_OK,
} i2c_status_e;

typedef struct {
    bool initialized;
} i2c_sw_data_t;

typedef struct {
    uint8_t sda_pin;
    uint8_t scl_pin;
} i2c_hw_cfg_t;

typedef struct {
    i2c_id_e id;
    i2c_sw_data_t sw_data;
    i2c_hw_cfg_t hw_cfg;
} i2c_ctx_t;

extern i2c_ctx_t i2c_inst[I2C_ID_MAX];

void drv_i2c_init(i2c_id_e id, uint8_t sda_pin, uint8_t scl_pin);
i2c_status_e drv_i2c_send(i2c_id_e id, uint8_t address, const uint8_t *data);

#endif // ENABLE_I2C
#endif // DRV_I2C_H_