#ifndef MID_PH_H_
#define MID_PH_H_

#include <stdint.h>

// Calibration voltages at pH 4.01 and pH 7.00, measured before the divider.
bool mid_ph_init(float voltage_ph_4_01, float voltage_ph_7_00);

// ADC voltage after the 1:2 divider, in mV.
// Returns a pH in 0..14, or NAN if uninitialized/input is invalid or out of range.
float mid_ph_read_ph(uint32_t voltage_mv);

#endif // MID_PH_H_