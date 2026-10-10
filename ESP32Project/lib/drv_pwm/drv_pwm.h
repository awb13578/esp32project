#ifndef DRV_PWM_H_
#define DRV_PWM_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_PWM

typedef struct {
    bool initialized;
    uint32_t duty_cycle;
} pwm_sw_data_t;

typedef struct {
    uint8_t pin;
    uint32_t frequency;
    uint8_t resolution;
} pwm_hw_cfg_t;

typedef struct {
    pwm_id_e id;
    pwm_sw_data_t sw_data;
    pwm_hw_cfg_t hw_cfg;
} pwm_ctx_t;

extern pwm_ctx_t pwm_inst[PWM_ID_MAX];

void drv_pwm_init(pwm_id_e id, uint8_t pin, uint32_t frequency, uint8_t resolution = 8);
void drv_pwm_set_duty(pwm_id_e id, uint8_t percent);
void drv_pwm_disable(pwm_id_e id);

#endif // ENABLE_PWM
#endif // DRV_PWM_H_