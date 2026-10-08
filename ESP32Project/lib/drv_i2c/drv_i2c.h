#ifndef DRV_I2C_H_
#define DRV_I2C_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_I2C

typedef struct {
    bool initialized;
} i2c_sw_data_t;

typedef struct {
    uint8_t sda_pin;
    uint8_t scl_pin;
    uint32_t frequency;
} i2c_hw_cfg_t;

typedef struct {
    i2c_id_e id;
    i2c_sw_data_t sw_data;
    i2c_hw_cfg_t hw_cfg;
} i2c_ctx_t;

extern i2c_ctx_t i2c_inst[I2C_ID_MAX];

bool drv_i2c_init(i2c_id_e id, uint8_t sda_pin, uint8_t scl_pin,
                  uint32_t frequency = 100000UL);
bool drv_i2c_write(i2c_id_e id, uint8_t address, const uint8_t *data,
                   size_t length);
bool drv_i2c_read(i2c_id_e id, uint8_t address, uint8_t *data,
                  size_t length);
bool drv_i2c_write_read(i2c_id_e id, uint8_t address,
                        const uint8_t *write_data, size_t write_length,
                        uint8_t *read_data, size_t read_length);

#endif // ENABLE_I2C
#endif // DRV_I2C_H_