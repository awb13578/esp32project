#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <stdint.h>

typedef struct {
    volatile bool system_state;
    bool control_ready;
    bool display_ready;

    uint16_t adc_ph_raw;       // One instantaneous ADC sample.
    uint32_t adc_ph_mv;        // Averaged ADC voltage in millivolts.
    float voltage_ph_adc_v;    // Voltage at the ESP32 ADC pin.
    float voltage_ph_v;        // Sensor voltage reconstructed before divider.
    float ph;

    uint8_t pwm_motor_percent;
    uint8_t pwm_pump_acid_percent;
    uint8_t pwm_pump_bazo_percent;
} app_state_t;

extern app_state_t app_state;

#endif // APP_STATE_H_