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
    adc_inst[id].sw_data.voltage_mv = 0;
    analogReadResolution(ADC_RESOLUTION); // set độ phân giải ADC
}

uint32_t drv_adc_read_millivolts(adc_id_e id) {
    if (!drv_adc_valid_id(id)) {
        return UINT32_MAX;
    }

    uint32_t samples[ADC_SAMPLE_COUNT];
    for (uint8_t i = 0; i < ADC_SAMPLE_COUNT; ++i) {
        samples[i] = analogReadMilliVolts(adc_inst[id].hw_cfg.pin);
        delay(1);
    }

    for (uint8_t i = 1; i < ADC_SAMPLE_COUNT; ++i) {
        const uint32_t sample = samples[i];
        uint8_t j = i;
        while (j > 0 && samples[j - 1] > sample) {
            samples[j] = samples[j - 1];
            --j;
        }
        samples[j] = sample;
    }

    uint32_t sum = 0;
    for (uint8_t i = 1; i < ADC_SAMPLE_COUNT - 1; ++i) {
        sum += samples[i];
    }

    adc_inst[id].sw_data.voltage_mv = sum / (ADC_SAMPLE_COUNT - 2);
    return adc_inst[id].sw_data.voltage_mv;
}

#endif // ENABLE_ADC