#ifndef MID_PH_H_
#define MID_PH_H_

#include <stdint.h>

// Dien ap module tai pH 4.01, do truoc mach phan ap.
// Tra ve false neu gia tri khong hop le.
bool mid_ph_init(float voltage_ph_4_01);

// Gia tri dien ap ADC (mV) sau mach phan ap 1/2.
// Tra ve NAN neu chua init hoac dau vao khong hop le.
float mid_ph_read_ph(uint32_t voltage_mv);

#endif // MID_PH_H_