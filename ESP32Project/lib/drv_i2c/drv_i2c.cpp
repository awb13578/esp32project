#include "drv_i2c.h"

#ifdef ENABLE_I2C

#include <Wire.h>

namespace {

bool drv_i2c_valid_id(i2c_id_e id) {
    return static_cast<unsigned int>(id) < I2C_ID_MAX; // kiểm tra id
}

bool drv_i2c_valid_address(uint8_t address) {
    return address >= 0x08 && address <= 0x77;// kiểm tra địa chỉ i2c
}

}

i2c_ctx_t i2c_inst[I2C_ID_MAX];// khai báo mảng chứa các instance của i2c

bool drv_i2c_init(i2c_id_e id, uint8_t sda_pin, uint8_t scl_pin,
                  uint32_t frequency) {
    if (!drv_i2c_valid_id(id) || sda_pin == scl_pin || sda_pin > 48 ||
        scl_pin > 48 || frequency == 0 || frequency > 1000000UL) {
        return false;// kiểm tra id, chân và tần số truyền dữ liệu
    }

    i2c_inst[id].id = id;// gán id cho instance của i2c
    i2c_inst[id].hw_cfg.sda_pin = sda_pin;// gán chân sda 
    i2c_inst[id].hw_cfg.scl_pin = scl_pin;//   gán chân scl 
    i2c_inst[id].hw_cfg.frequency = frequency;// gán tần số truyền dữ liệu 
    i2c_inst[id].sw_data.initialized = false;// gán trạng thái hoạt động của driver 

    if (!Wire.begin(sda_pin, scl_pin)) {
        return false;
    }

    Wire.setClock(frequency);// gán tần số truyền dữ liệu
    i2c_inst[id].sw_data.initialized = true;// gán trạng thái hoạt động của driver cho instance của i2c
    return true;
}

bool drv_i2c_write(i2c_id_e id, uint8_t address, const uint8_t *data,
                   size_t length) {
    if (!drv_i2c_valid_id(id) || !i2c_inst[id].sw_data.initialized ||
        !drv_i2c_valid_address(address) || (length > 0 && data == nullptr)) {
        return false;// kiểm tra id, trạng thái hoạt động của driver, địa chỉ i2c và dữ liệu
    }

    Wire.beginTransmission(address);// bắt đầu truyền dữ liệu đến địa chỉ i2c
    if (length > 0 && Wire.write(data, length) != length) {
        Wire.endTransmission(true);// kết thúc truyền dữ liệu đến địa chỉ i2c
        return false;
    }

    return Wire.endTransmission(true) == 0;// kết thúc truyền dữ liệu đến địa chỉ i2c và kiểm tra trạng thái truyền dữ liệu
}

bool drv_i2c_read(i2c_id_e id, uint8_t address, uint8_t *data,
                  size_t length) {
    if (!drv_i2c_valid_id(id) || !i2c_inst[id].sw_data.initialized ||
        !drv_i2c_valid_address(address) || data == nullptr || length == 0) {
        return false;// kiểm tra id, trạng thái hoạt động của driver, địa chỉ i2c và dữ liệu
    }

    size_t received = Wire.requestFrom(address, length, static_cast<uint8_t>(true));// yêu cầu dữ liệu từ địa chỉ i2c
    if (received != length) {
        while (Wire.available()) {
            Wire.read();// đọc dữ liệu từ địa chỉ i2c
        }
        return false;
    }

    for (size_t i = 0; i < length; ++i) {
        if (!Wire.available()) {
            return false;// kiểm tra dữ liệu từ địa chỉ i2c
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
        return false;// kiểm tra id, trạng thái hoạt động của driver, địa chỉ i2c và dữ liệu
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