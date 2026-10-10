#ifndef DRV_PWM_H_
#define DRV_PWM_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_PWM

typedef struct {
    bool initialized;
    uint32_t duty_cycle;
} pwm_sw_data_t;// lưu trạng thái hoạt động của driver và chu kỳ nhiệm vụ

typedef struct {
    uint8_t pin;// chân
    uint32_t frequency;// tần số
    uint8_t resolution;// độ phân giải
} pwm_hw_cfg_t;// thông tin phần cứng của pwm

typedef struct {
    pwm_id_e id;//id
    pwm_sw_data_t sw_data;
    pwm_hw_cfg_t hw_cfg;
} pwm_ctx_t;// gom thông tin của instance pwm

extern pwm_ctx_t pwm_inst[PWM_ID_MAX];// khai báo mảng chứa các instance của pwm

// Mỗi lần gọi khởi tạo một ID PWM; duty của API này tính theo phần trăm.
bool PWM_System_Init(pwm_id_e id, uint8_t pin, uint32_t frequency = 20000,
                     uint8_t resolution = 13);
bool PWM_SetDuty(pwm_id_e id, uint8_t duty_percent);
bool PWM_SetFrequency(pwm_id_e id, uint32_t frequency);
bool PWM_Disable_Channel(pwm_id_e id);

bool drv_pwm_init(pwm_id_e id, uint8_t pin, uint32_t frequency,
                  uint8_t resolution = 8);// khởi tạo pwm
bool drv_pwm_write(pwm_id_e id, uint32_t duty);// ghi dữ liệu vào pwm

#endif // ENABLE_PWM
#endif // DRV_PWM_H_