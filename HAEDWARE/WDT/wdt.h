#ifndef __WDT_H__
#define __WDT_H__
#include "HC89F303.h"

void wdt_init(u8 time);
void clear_wdt(void);
#endif