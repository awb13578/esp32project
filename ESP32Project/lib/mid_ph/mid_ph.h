#ifndef MID_PH_H_
#define MID_PH_H_

#include <stdint.h>

// Khoi tao noi suy tu ba diem hieu chuan pH duoc cau hinh trong mid_ph.cpp.
bool mid_ph_init(void);

// Chuyen dien ap ADC (mV) thanh dien ap module truoc mach phan ap.
float mid_ph_get_module_voltage(uint32_t adc_voltage_mv);

// Gia tri dien ap ADC (mV) sau mach phan ap.
// Tra ve NAN neu chua init hoac dau vao khong hop le.
float mid_ph_read_ph(uint32_t voltage_mv);

#endif // MID_PH_H_