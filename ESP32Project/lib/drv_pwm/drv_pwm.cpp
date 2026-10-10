#include "drv_pwm.h"

#ifdef ENABLE_PWM

#include <Arduino.h>
#include <esp_arduino_version.h>

namespace {

bool drv_pwm_valid_id(pwm_id_e id) {
    return static_cast<unsigned int>(id) < PWM_ID_MAX;// kiểm tra id
}

bool drv_pwm_valid_pin(uint8_t pin) {
    return pin <= 48;// kiểm tra chân pwm
}

bool drv_pwm_valid_resolution(uint8_t resolution) {
    return resolution > 0 && resolution <= 14;// kiểm tra độ phân giải pwm
}

}

pwm_ctx_t pwm_inst[PWM_ID_MAX];// khai báo mảng chứa các instance của pwm

bool drv_pwm_init(pwm_id_e id, uint8_t pin, uint32_t frequency,
                  uint8_t resolution) {
    if (!drv_pwm_valid_id(id) || !drv_pwm_valid_pin(pin) || frequency == 0 ||
        !drv_pwm_valid_resolution(resolution)) {
        return false;// kiểm tra id, chân, tần số và độ phân giải pwm
    }

    pwm_inst[id].id = id;//gán id
    pwm_inst[id].hw_cfg.pin = pin;//gán chân
    pwm_inst[id].hw_cfg.frequency = frequency;//gán tần số
    pwm_inst[id].hw_cfg.resolution = resolution;//gán độ phân giải
    pwm_inst[id].sw_data.initialized = false;//gán trạng thái hoạt động của driver
    pwm_inst[id].sw_data.duty_cycle = 0;//gán chu kỳ nhiệm vụ

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcAttach(pin, frequency, resolution)) {
        return false;// kiểm tra trạng thái hoạt động của driver
    }
#else
    const uint8_t channel = static_cast<uint8_t>(id);
    if (ledcSetup(channel, frequency, resolution) == 0) {
        return false;// kiểm tra trạng thái hoạt động của driver
    }
    ledcAttachPin(pin, channel);// gán chân cho kênh pwm
#endif

    pwm_inst[id].sw_data.initialized = true;
    return true;// gán trạng thái hoạt động của driver cho instance của pwm
}

bool drv_pwm_write(pwm_id_e id, uint32_t duty) {
    if (!drv_pwm_valid_id(id) || !pwm_inst[id].sw_data.initialized) {
        return false;// kiểm tra id và trạng thái hoạt động của driver
    }

    const uint32_t max_duty =
        (static_cast<uint32_t>(1) << pwm_inst[id].hw_cfg.resolution) - 1;
    if (duty > max_duty) {
        duty = max_duty;// kiểm tra chu kỳ nhiệm vụ
    }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcWrite(pwm_inst[id].hw_cfg.pin, duty)) {
        return false;// kiểm tra trạng thái hoạt động của driver
    }
#else
    ledcWrite(static_cast<uint8_t>(id), duty);// gán chu kỳ nhiệm vụ cho kênh pwm
#endif

    pwm_inst[id].sw_data.duty_cycle = duty;// gán chu kỳ nhiệm vụ cho instance của pwm
    return true;
}

bool PWM_System_Init(pwm_id_e id, uint8_t pin, uint32_t frequency,
                     uint8_t resolution) {
    return drv_pwm_init(id, pin, frequency, resolution);
}

bool PWM_SetDuty(pwm_id_e id, uint8_t duty_percent) {
    if (!drv_pwm_valid_id(id) || !pwm_inst[id].sw_data.initialized) {
        return false;
    }

    if (duty_percent > 100) {
        duty_percent = 100;
    }

    const uint32_t max_duty =
        (static_cast<uint32_t>(1) << pwm_inst[id].hw_cfg.resolution) - 1;
    const uint32_t duty =
        (static_cast<uint32_t>(duty_percent) * max_duty) / 100;
    return drv_pwm_write(id, duty);
}

bool PWM_SetFrequency(pwm_id_e id, uint32_t frequency) {
    if (!drv_pwm_valid_id(id) || !pwm_inst[id].sw_data.initialized ||
        frequency == 0) {
        return false;
    }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (ledcChangeFrequency(pwm_inst[id].hw_cfg.pin, frequency,
                            pwm_inst[id].hw_cfg.resolution) == 0) {
        return false;
    }
#else
    const uint8_t channel = static_cast<uint8_t>(id);
    if (ledcSetup(channel, frequency, pwm_inst[id].hw_cfg.resolution) == 0) {
        return false;
    }
#endif

    pwm_inst[id].hw_cfg.frequency = frequency;
    return true;
}

bool PWM_Disable_Channel(pwm_id_e id) {
    if (!drv_pwm_valid_id(id) || !pwm_inst[id].sw_data.initialized) {
        return false;
    }

    return drv_pwm_write(id, 0);
}

#endif // ENABLE_PWM
