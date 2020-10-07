#ifndef __HT1621B_H__
#define __HT1621B_H__
#include "HC89F303.h"

#define LCD_CS		P1_1
#define LCD_RD		P1_0
#define LCD_WR		P0_7
#define LCD_DATA	P0_6

void ht1621_init_pin(void);
void ht1621_send_cmd(u8 command);
void ht1621_sendbit_hight(u8 dat,u8 cnt);
void ht1621_sendbit_low(u8 dat,u8 cnt);
void ht1621_write_byte(u8 addr,u8 dat);
void release_ht1621b_pin(void);
#endif 

