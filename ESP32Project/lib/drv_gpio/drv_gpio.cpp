#include "drv_gpio.h"

#ifdef ENABLE_GPIO

gpio_ctx_t gpio_inst[GPIO_ID_MAX];

static bool drv_gpio_valid_id(gpio_id_e id) {
    return (static_cast<int>(id) >= 0) && (id < GPIO_ID_MAX);
}

void drv_gpio_init(gpio_id_e id, uint8_t pin, uint8_t mode) {
    if (!drv_gpio_valid_id(id)) {
        return;
    }

    gpio_inst[id].id = id;
    gpio_inst[id].hw_cfg.pin = pin;
    pinMode(gpio_inst[id].hw_cfg.pin, mode);
}

void drv_gpio_set_pin(gpio_id_e id) {
    if (!drv_gpio_valid_id(id)) {
        return;
    }

    digitalWrite(gpio_inst[id].hw_cfg.pin, HIGH);
    gpio_inst[id].sw_data.state = 1;
}

void drv_gpio_clear_pin(gpio_id_e id) {
    if (!drv_gpio_valid_id(id)) {
        return;
    }

    digitalWrite(gpio_inst[id].hw_cfg.pin, LOW);
    gpio_inst[id].sw_data.state = 0;
}

void drv_gpio_toggle_pin(gpio_id_e id) {
    if (!drv_gpio_valid_id(id)) {
        return;
    }

    digitalWrite(gpio_inst[id].hw_cfg.pin, !digitalRead(gpio_inst[id].hw_cfg.pin));
    gpio_inst[id].sw_data.state = !gpio_inst[id].sw_data.state;
}

uint8_t drv_gpio_read_pin(gpio_id_e id) {
    if (!drv_gpio_valid_id(id)) {
        return UINT8_MAX;
    }

    return digitalRead(gpio_inst[id].hw_cfg.pin);
}

#endif // ENABLE_GPIO