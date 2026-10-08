#include "drv_i2c.h"

#ifdef ENABLE_I2C

#include <Wire.h>

namespace {

bool drv_i2c_valid_id(i2c_id_e id) {
    return static_cast<unsigned int>(id) < I2C_ID_MAX;
}

bool drv_i2c_valid_address(uint8_t address) {
    return address >= 0x08 && address <= 0x77;
}

}

i2c_ctx_t i2c_inst[I2C_ID_MAX];

bool drv_i2c_init(i2c_id_e id, uint8_t sda_pin, uint8_t scl_pin,
                  uint32_t frequency) {
    if (!drv_i2c_valid_id(id) || frequency == 0) {
        return false;
    }

    i2c_inst[id].id = id;
    i2c_inst[id].hw_cfg.sda_pin = sda_pin;
    i2c_inst[id].hw_cfg.scl_pin = scl_pin;
    i2c_inst[id].hw_cfg.frequency = frequency;
    i2c_inst[id].sw_data.initialized = false;

    if (!Wire.begin(sda_pin, scl_pin, frequency)) {
        return false;
    }

    Wire.setClock(frequency);
    i2c_inst[id].sw_data.initialized = true;
    return true;
}

bool drv_i2c_write(i2c_id_e id, uint8_t address, const uint8_t *data,
                   size_t length) {
    if (!drv_i2c_valid_id(id) || !i2c_inst[id].sw_data.initialized ||
        !drv_i2c_valid_address(address) || (length > 0 && data == nullptr)) {
        return false;
    }

    Wire.beginTransmission(address);
    if (length > 0 && Wire.write(data, length) != length) {
        Wire.endTransmission(true);
        return false;
    }

    return Wire.endTransmission(true) == 0;
}

bool drv_i2c_read(i2c_id_e id, uint8_t address, uint8_t *data,
                  size_t length) {
    if (!drv_i2c_valid_id(id) || !i2c_inst[id].sw_data.initialized ||
        !drv_i2c_valid_address(address) || data == nullptr || length == 0) {
        return false;
    }

    size_t received = Wire.requestFrom(address, length, static_cast<uint8_t>(true));
    if (received != length) {
        while (Wire.available()) {
            Wire.read();
        }
        return false;
    }

    for (size_t i = 0; i < length; ++i) {
        if (!Wire.available()) {
            return false;
        }
        data[i] = static_cast<uint8_t>(Wire.read());
    }

    return true;
}

bool drv_i2c_write_read(i2c_id_e id, uint8_t address,
                        const uint8_t *write_data, size_t write_length,
                        uint8_t *read_data, size_t read_length) {
    if (!drv_i2c_valid_id(id) || !i2c_inst[id].sw_data.initialized ||
        !drv_i2c_valid_address(address) || write_data == nullptr ||
        write_length == 0 || read_data == nullptr || read_length == 0) {
        return false;
    }

    Wire.beginTransmission(address);
    if (Wire.write(write_data, write_length) != write_length) {
        Wire.endTransmission(true);
        return false;
    }

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    size_t received = Wire.requestFrom(address, read_length, static_cast<uint8_t>(true));
    if (received != read_length) {
        while (Wire.available()) {
            Wire.read();
        }
        return false;
    }

    for (size_t i = 0; i < read_length; ++i) {
        if (!Wire.available()) {
            return false;
        }
        read_data[i] = static_cast<uint8_t>(Wire.read());
    }

    return true;
}

#endif // ENABLE_I2C