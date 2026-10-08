#ifndef DRV_GPIO_H_
#define DRV_GPIO_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_GPIO

typedef struct {
    uint8_t state;
} gpio_sw_data_t;

typedef struct {
    uint8_t pin;
} gpio_hw_cfg_t;

typedef struct {
    gpio_id_e id;
    gpio_sw_data_t sw_data;
    gpio_hw_cfg_t hw_cfg;
} gpio_ctx_t;

extern gpio_ctx_t gpio_inst[GPIO_ID_MAX]; // khai báo mảng chưa các instance của GPIO

void drv_gpio_init(gpio_id_e id, uint8_t pin, uint8_t mode); 
//cấu hình chân nào là OUTPUT hay INPUT, nếu muốn thay đổi thì thêm id trong enum gpio_id_e
void drv_gpio_set_pin(gpio_id_e id); //set chân HIGH
void drv_gpio_clear_pin(gpio_id_e id); //clear chân LOW
void drv_gpio_toggle_pin(gpio_id_e id); //đảo trạng thái chân
uint8_t drv_gpio_read_pin(gpio_id_e id); //đọc trạng thái chân

#endif // ENABLE_GPIO
#endif // DRV_GPIO_H_