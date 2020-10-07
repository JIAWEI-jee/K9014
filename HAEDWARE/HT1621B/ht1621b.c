#include "ht1621b.h"
#include "delay.h"
#include "intrins.h"//nop


void ht1621_init_pin(void)
{
	P1M1 = 0XC1;//设置为推挽输出
	P1M0 = 0XC1;//设置为推挽输出
	P0M7 = 0XC1;//设置为推挽输出
	P0M6 = 0XC1;//设置为推挽输出

	LCD_CS = 1;
	LCD_RD = 1;
	LCD_WR = 1;
	LCD_DATA = 1;
}

//高位在前
void ht1621_sendbit_hight(u8 dat,u8 cnt)
{
	u8 i = 0;

	for (i = 0; i < cnt; ++i)
	{
		LCD_WR = 0;
		if((dat & 0x80) == 0)
			LCD_DATA = 0;
		else 
			LCD_DATA = 1;
		LCD_WR = 1;
		lcd_delay_us(5);
		dat <<= 1;
	}
}
//低位在前
void ht1621_sendbit_low(u8 dat,u8 cnt)
{
	u8 i = 0;

	for (i = 0; i < cnt; ++i)
	{
		LCD_WR = 0;
		if((dat & 0x01) == 0)
			LCD_DATA = 0;
		else	
			LCD_DATA = 1;
		LCD_WR = 1;
		lcd_delay_us(5);
		dat >>= 1;	
	}
}


void ht1621_send_cmd(u8 command)
{
	LCD_CS = 0;
	ht1621_sendbit_hight(0x80,4);  //写入标志位100  写命令  没有更改时钟输出所以command最高位置0
	ht1621_sendbit_hight(command,8);
	LCD_CS = 1;
}

void ht1621_write_byte(u8 addr, u8 dat)
{
	LCD_CS = 0;
	ht1621_sendbit_hight(0xA0,3);//101   写数据标志位
	ht1621_sendbit_hight(addr << 2,6);//ram  高6位  SEG序号
	ht1621_sendbit_low(dat,4);//data 低4位  COM
	LCD_CS = 1;
}

void release_ht1621b_pin(void)
{
	LCD_CS = 1;
	LCD_RD = 1;
	LCD_WR = 1;
	LCD_DATA = 1;
}





