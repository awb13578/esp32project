#ifndef DRV_ADC_H_
#define DRV_ADC_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_ADC  // chỉ bật code khi ADC được enable

#define ADC_RESOLUTION 12 // set độ phân giải ADC là 12 bit

typedef struct {
    uint16_t value;
} adc_sw_data_t;

typedef struct {
    uint8_t pin;
} adc_hw_cfg_t;

typedef struct {
    adc_id_e id;
    adc_sw_data_t sw_data;
    adc_hw_cfg_t hw_cfg;
} adc_ctx_t;

extern adc_ctx_t adc_inst[ADC_ID_MAX]; // khai báo mảng chưa các instance của ADC


void drv_adc_init(adc_id_e id, uint8_t pin); // dùng để cấu hình ADC
uint16_t drv_adc_read(adc_id_e id); // dùng để đọc giá trị ADC trả về


#endif // ENABLE_ADC
#endif // DRV_ADC_H_