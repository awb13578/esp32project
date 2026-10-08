#include <Arduino.h>
#include "drv_i2c.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    if (!drv_i2c_init(I2C_ID_A, 21, 22, 100000UL)) {
        Serial.println("I2C initialization failed");
        return;
    }

    Serial.println("Scanning I2C addresses...");
    for (uint8_t address = 0x01; address <= 0x7F; ++address) {
        if (drv_i2c_write(I2C_ID_A, address, nullptr, 0)) {
            Serial.print("I2C device found at 0x");
            if (address < 0x10) {
                Serial.print('0');
            }
            Serial.println(address, HEX);
        }
    }
    Serial.println("I2C scan complete");
}

void loop() {
    delay(1000);
}