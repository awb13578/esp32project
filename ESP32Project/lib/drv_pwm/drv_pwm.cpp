#include "drv_pwm.h"

#ifdef ENABLE_PWM

#include <Arduino.h>
#include <esp_arduino_version.h>

pwm_ctx_t pwm_inst[PWM_ID_MAX];

static bool drv_pwm_valid_id(pwm_id_e id) {
    return (id >= PWM_ID_MOTOR) && (id < PWM_ID_MAX);
}

void drv_pwm_init(pwm_id_e id, uint8_t pin, uint32_t frequency,
                  uint8_t resolution) {
    if (!drv_pwm_valid_id(id)) {
        return;
    }

    pwm_inst[id].id = id;
    pwm_inst[id].hw_cfg.pin = pin;
    pwm_inst[id].hw_cfg.frequency = frequency;
    pwm_inst[id].hw_cfg.resolution = resolution;
    pwm_inst[id].sw_data.initialized = false;
    pwm_inst[id].sw_data.duty_cycle = 0;

    if (frequency == 0 || resolution == 0 || resolution >= 32) {
        return;
    }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcAttach(pin, frequency, resolution)) {
        return;
    }
#else
    const uint8_t channel = static_cast<uint8_t>(id);
    if (ledcSetup(channel, frequency, resolution) == 0) {
        return;
    }
    ledcAttachPin(pin, channel);
#endif

    pwm_inst[id].sw_data.initialized = true;
}

void drv_pwm_set_duty(pwm_id_e id, uint8_t percent) {
    if (!drv_pwm_valid_id(id) || !pwm_inst[id].sw_data.initialized) {
        return;
    }

    uint8_t duty_percent = percent;
    if (duty_percent > 100) {
        duty_percent = 100;
    }

    const uint32_t max_duty =
        (static_cast<uint32_t>(1) << pwm_inst[id].hw_cfg.resolution) - 1;
    const uint32_t duty = static_cast<uint32_t>(
        (static_cast<uint64_t>(duty_percent) * max_duty) / 100u);

    if (duty > max_duty) {
        return;
    }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(pwm_inst[id].hw_cfg.pin, duty);
#else
    ledcWrite(static_cast<uint8_t>(id), duty);
#endif

    pwm_inst[id].sw_data.duty_cycle = duty;
}

void drv_pwm_disable(pwm_id_e id) {
    if (!drv_pwm_valid_id(id) || !pwm_inst[id].sw_data.initialized) {
        return;
    }

    drv_pwm_set_duty(id, 0);
}

#endif // ENABLE_PWM
