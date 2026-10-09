#ifndef DRV_I2C_H_
#define DRV_I2C_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_I2C

typedef struct {
    bool initialized;// lưu trạng thái hoạt động của driver
} i2c_sw_data_t;

typedef struct {
    uint8_t sda_pin;// chân sda
    uint8_t scl_pin;// chân sdl
    uint32_t frequency;// tần số truyền dữ liệu
} i2c_hw_cfg_t;// thông tin phần cứng của i2c

typedef struct {
    i2c_id_e id;
    i2c_sw_data_t sw_data;
    i2c_hw_cfg_t hw_cfg;
} i2c_ctx_t; // gom thông tin của instance i2c

extern i2c_ctx_t i2c_inst[I2C_ID_MAX]; // khai báo mảng chứa các instance của i2c

bool drv_i2c_init(i2c_id_e id, uint8_t sda_pin, uint8_t scl_pin,
                  uint32_t frequency = 100000UL);// khởi tạo i2c
bool drv_i2c_write(i2c_id_e id, uint8_t address, const uint8_t *data,
                   size_t length);// ghi dữ liệu vào i2c
bool drv_i2c_read(i2c_id_e id, uint8_t address, uint8_t *data,
                  size_t length);//đọc dữ liệu từ i2c
bool drv_i2c_write_read(i2c_id_e id, uint8_t address,
                        const uint8_t *write_data, size_t write_length,
                        uint8_t *read_data, size_t read_length);// ghi dữ liệu vào i2c

#endif // ENABLE_I2C
#endif // DRV_I2C_H_