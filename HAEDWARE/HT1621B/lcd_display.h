#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__
#include "HC89F303.h"
#include "ht1621b.h"
#include "flash.h"

#define BIAS		0X52 	//偏置电压
#define	SYSDIS		0X00	//关闭系统时钟和偏压发生器
#define SYSEN   	0X02	//打开系统时钟
#define LCD_OFF		0X04	//关闭偏压发生器
#define LCD_ON		0X06	//打开偏压发生器
#define RC256 		0X30	//内部时钟256K
#define TONEON 		0X12 	//打开声音输出
#define TONEOFF 	0X10  	//关闭声音输出
#define WDTDIS 		0X0A 	//禁止看门狗
#define XTAL_32K	0X28	//外部接时钟32k  
#define TMDIS		0x08

#define Error_STD   0xff

extern u8 cali_display_std;

void lcd_display_time(u8 time_type);
void lcd_display_gap(u8 gap);

void init_lcd_ht1621b(void);
void ht1621_all_display(void);
void ht1621_all_clear(void);
void lcd_turn_off_time(void);
void flick(void);

#endif
