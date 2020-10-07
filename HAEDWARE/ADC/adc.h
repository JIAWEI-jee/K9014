#ifndef __ADC_H__
#define __ADC_H__
#include "HC89F303.h"

void adc_init(void);

 void get_voltage(u16 *u1_voltage,u16 *u3_voltage);
u16 get_voltage_val(void);
u16 get_adc_val_ch(void);

#endif
