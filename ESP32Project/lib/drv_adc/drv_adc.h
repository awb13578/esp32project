#ifndef DRV_ADC_H_
#define DRV_ADC_H_

#include "drv_config.h"
#include "config_lib.h"

#ifdef ENABLE_ADC  // chỉ bật code khi ADC được enable

#define ADC_RESOLUTION 12 // set độ phân giải ADC là 12 bit
#define ADC_SAMPLE_COUNT 10 // số mẫu đọc để tính trung bình

typedef struct {
    uint32_t voltage_mv;
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
uint32_t drv_adc_read_millivolts(adc_id_e id); // đọc 10 mẫu trung bình theo mV, trả về UINT32_MAX nếu id không hợp lệ


#endif // ENABLE_ADC
#endif // DRV_ADC_H_