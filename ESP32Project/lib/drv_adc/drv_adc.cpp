#include "drv_adc.h"

#ifdef ENABLE_ADC

adc_ctx_t adc_inst[ADC_ID_MAX];

static bool drv_adc_valid_id(adc_id_e id) {
    return (static_cast<int>(id) >= 0) && (id < ADC_ID_MAX);
}

void drv_adc_init(adc_id_e id, uint8_t pin) {
    if (!drv_adc_valid_id(id)) {
        return;
    }

    adc_inst[id].id = id;
    adc_inst[id].hw_cfg.pin = pin;
    analogReadResolution(ADC_RESOLUTION); // set độ phân giải ADC
}

uint16_t drv_adc_read(adc_id_e id) {
    if (!drv_adc_valid_id(id)) {
        return UINT16_MAX;
    }

    adc_inst[id].sw_data.value = analogRead(adc_inst[id].hw_cfg.pin);
    return adc_inst[id].sw_data.value;
}

#endif // ENABLE_ADC