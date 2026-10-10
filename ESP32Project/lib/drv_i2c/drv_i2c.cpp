#include "drv_i2c.h"

#ifdef ENABLE_I2C

#include <Wire.h>

i2c_ctx_t i2c_inst[I2C_ID_MAX] = {};

void drv_i2c_init(i2c_id_e id, uint8_t sda_pin, uint8_t scl_pin) {
    i2c_inst[id].id = id;
    i2c_inst[id].hw_cfg.sda_pin = sda_pin;
    i2c_inst[id].hw_cfg.scl_pin = scl_pin;
    i2c_inst[id].sw_data.initialized = Wire.begin(sda_pin, scl_pin);
}

i2c_status_e drv_i2c_send(i2c_id_e id, uint8_t address, const uint8_t *data) {
    if (!i2c_inst[id].sw_data.initialized ||
        data == nullptr) {
        return I2C_STATUS_ERROR;
    }

    Wire.beginTransmission(address);
    Wire.write(data, TxLength);
    Wire.endTransmission();

    return I2C_STATUS_OK;
}

#endif // ENABLE_I2C