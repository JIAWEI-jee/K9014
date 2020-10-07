#define	ALLOCATE_EXTERN
#include "HC89F303.h"
#include "intrins.h"//nop
#include "uart.h"
#include "delay.h"
#include "key.h"
#include "flash.h"
#include "timer.h"
#include "pwm.h"
#include "lcd_display.h"
#include "adc.h"
#include "wdt.h"

#define SKU 9014
#define SOFT_VER "1.00.00"

u16 adc_cnt = 0;
u8  first_heat_std = 0,fault_std = 0;

void Set_Temp ( u16 temp );
void Controll_Heat ( u16 temp_set,u16 temp_now );
void Protect ( void );
void Error ( void );

void device_init ( void )
{
	/************************************系统初始化****************************************/
	// CLKSWR = 0x51;						//选择内部高频RC为系统时钟，内部高频RC 2分频，Fosc=16MHz
	CLKSWR = 0x53;  //8分频 32/8 = 4M
	CLKDIV = 0x01;						//Fosc 1分频得到Fcpu，Fcpu=16MHz
	P0M2 = 0x10; 						//P02设置为非施密特数字输入
}


static void key_handle ( void )
{
	u8 key_val = 0;
	if ( fault_std == 0 )
	{
		key_val = key_scan();
	}
	else
	{
		key_val = 0;
	}
	if ( ( key_val == KEY_1_PRES ) && ( calibration_std == 0 ) )
	{
		KEY_printf ( " key_scan\r\n" );

		if ( get_device_state() == ON )
		{

			set_device_state ( OFF );
			ht1621_all_clear();
//			set_pwm ( 0 );
			HEAT_STD = 0;
			led_set_off();
		}
		else
		{
			led_set_on();
			set_device_state ( ON );
			set_time_sec();
			if ( flash_info.gap > GAP_3 )
			{
				if ( flash_info.timer == TIMER_ON )
				{
					On_stay = 2;
				}
				else if ( flash_info.timer > TIMER_2H )
				{
					Gap_protect_std = 2;
				}
			}
			first_heat_std = 1;
			set_correct_time ( flash_info.gap );
			lcd_display_gap ( flash_info.gap );
			lcd_display_time ( flash_info.timer );

		}
	}

	else if ( get_device_state() == ON )
	{

		if ( key_val == LED_STAY_ON )
		{
//		 	KEY_printf ( " KEY_led_PRES\r\n" );
			led_set_on();
		}

		if ( key_val == KEY_2_PRES ) 
		{
			led_set_on();
//			KEY_printf ( " KEY_2_PRES\r\n" );
			if ( flash_info.gap < GAP_9 )
			{
				flash_info.gap++;

			}
			else
			{
				flash_info.gap = GAP_1;
			}
			if ( flash_info.gap > GAP_3 )
			{
				if ( flash_info.timer == TIMER_ON )
				{
					On_stay = 2;
				}
				else if ( flash_info.timer > TIMER_2H )
				{
					Gap_protect_std = 2;
				}
			}

			first_heat_std = 1;
			set_correct_time ( flash_info.gap );
			lcd_display_gap ( flash_info.gap );
			//set_time_sec();
			flah_save_data();
		}
		else if ( key_val == KEY_3_PRES ) //定时
		{
			led_set_on();
			if ( flash_info.timer < 0x05 )
			{
				flash_info.timer++;
			}
			else
			{
				flash_info.timer = TIMER_ON;
			}
			if ( flash_info.gap > GAP_3 )
			{
				if ( flash_info.timer == TIMER_ON )
				{
					On_stay = 2;
				}
				else if ( flash_info.timer > TIMER_2H )
				{
					Gap_protect_std = 2;
				}
			}
			//KEY_printf ( "timer:%d \r\n", ( u16 ) flash_info.timer );
			lcd_display_time ( flash_info.timer );
			set_time_sec();
			flah_save_data();
		}
	}
}


u16 calibration_temperature ( u16 temper )
{
	u8 temp1 = 0;
	if ( ( usart_rx_flag == 2 ) && ( calibration_std == 1 ) )
	{
		temp1 = ( u8 ) temper;
		usart_rx_flag = 0;
		flash_info.correct_std = 1;

		if ( temper_val > 15 )
		{
			if ( temper_val > temp1 )
			{
				flash_info.correct_value = temper_val - temp1;
				flash_info.correct_sign = 1; //为正公差
			}
			else
			{
				flash_info.correct_value = temp1 - temper_val;

				hal_uart_putchar ( flash_info.correct_value );
				flash_info.correct_sign = 2; //为负公差
			}
			if ( ( flash_info.correct_value < 2 ) || ( flash_info.correct_value > 20 ) )
			{
				flash_info.correct_value = 0;
				flash_info.correct_sign = 0;
			}
			flah_save_data();
			producte_send_cmd ( 0x02, 0x02 );
		}
	}
	if ( flash_info.correct_sign == 1 )
	{
		return ( ( u16 ) ( temper = temper + flash_info.correct_value ) );
	}
	else if ( flash_info.correct_sign == 2 )
	{
		return ( ( u16 ) ( temper = temper - flash_info.correct_value ) );
	}
	return ( ( u16 ) temper );

}



u16 temp_calc ( u16 uR510,u16 uRw )
{
	u8 i = 0;
	u8 basi_tmp = 50;
	u8 k = 10;
	float u1 = 0;

	if ( ( uR510 < 50 ) || ( uR510 > uRw ) )
	{
		return 0xff;
	}


	u1 = (  ( ( float ) uRw /1000 ) - ( ( float ) uR510/1000 ) ) / ( ( ( float ) uR510/1000 ) /510 );
	if ( u1 < 0 )
	{
		return 0xff;
	}
//	gm_printf ( "R = %f  \r\n",u1 );
	u1 = u1 / Length;
	if ( u1 < 73 )
	{
		return 0;
	}
	// gm_printf("average R = %f \r\n",u1);
	u1 = u1 / Temperature_Value;
	//gm_printf("cmp = %f \r\n",u1);
	if ( u1*Temperature_Value > Temperature_Value )
	{
		while ( 1 )
		{
			u1 = u1 / ( 1 + 0.0048 );
			i++;
			if ( u1 <= 1 )
			{
				break;
			}

		}
		// gm_printf("basi_tmp_u1 = %d \r\n",basi_tmp);
		basi_tmp = basi_tmp + i;
		//gm_printf("basi_tmp_u1f = %d \r\n",basi_tmp);
	}
	else
	{
		while ( 1 )
		{
			u1 = u1 / ( 1 - 0.0048 );
			i++;
			if ( u1 >= 1 )
			{
				break;
			}


		}
		// gm_printf("i = %d \r\n",i);
		basi_tmp = basi_tmp - i;
	}
	//gm_printf("basi_tmpF:%d \r\n",basi_tmp);

	return ( basi_tmp );
}



void temperature_handle ( void )
{
	u16 temp = 0;
	u16 adc_val1 = 0,adc_val3 = 0;
	static u8 error_std = 0;
	adc_cnt++;

	if ( adc_cnt > 10000 )
	{
		adc_cnt = 0;
		get_voltage ( &adc_val1,&adc_val3 );

		//	KEY_printf ( "adv1 = %d adv3 =%d \r\n",adc_val1,adc_val3 );  //pjw set
		temp = temp_calc ( adc_val1, adc_val3 );
//			KEY_printf ( "temp val:%d \r\n",temp );
		temp =	calibration_temperature ( temp );
		//	KEY_printf ( "%d \r\n",temp );

		if ( adc_val1 >50 )
		{
			if ( get_device_state() == ON )
			{
				if ( first_heat_std == 1 )
				{
					first_heat_std = 0;
					if ( temp > 50 )
					{
						Heat_start_std = 2;
						Open_Heat_Value = corrected_value_GAP_6_temp;
					}
					else
					{
						Heat_start_std = 2;
						AC_POWER_CNT = first_heat_power;
						one_heat = 1;
						Open_Heat_Value = corrected_value_warm_temp	;
					}
				}

				lcd_display_time ( flash_info.timer );
				lcd_display_gap ( flash_info.gap );
				Set_Temp ( temp );
			}
			else
			{
				if ( calibration_std == 1 )
				{
//						set_pwm ( 0 );
					HEAT_STD = 0;
					ht1621_all_clear();
					lcd_display_gap ( flash_info.gap );
					cali_display_std = 1;
				}
				else
				{
//				set_pwm ( 0 );
					HEAT_STD = 0;
					ht1621_send_cmd ( LCD_OFF );
					cali_display_std = 0;
				}
			}
			fault_std = 0;
		}
		else
		{
			calibration_std = 0;
			cali_display_std = 0;
			ht1621_all_clear();
			HEAT_STD = 0;
			error_std = Error_STD;
			lcd_display_gap ( error_std );
			fault_std = 1;
		}

	}


}




void main ( void )
{
	device_init();

	uart_init();
	adc_init();
	EA = 1;//开启总中断
	key_init();
	time0_init();
	flash_init();

	led_set_on();
	init_lcd_ht1621b();
	delay_ms ( 800 );
//	ht1621_all_clear();
//	lcd_display_gap ( GAP_8 );
	led_set_off();
	wdt_init ( 2 );
//	set_pwm ( 0 );
	HEAT_STD = 0;

	gm_printf ( "\r\n==================================\r\n" );
	gm_printf ( "sku:K%d \r\n", ( u16 ) SKU );
	gm_printf ( "soft version:%s \r\n",SOFT_VER );
	gm_printf ( "gap %d \r\n", ( u16 ) flash_info.gap );      //挡位
	gm_printf ( "timer %d \r\n", ( u16 ) flash_info.timer );  // 时间长度
	gm_printf ( "==================================\r\n" );

	while ( 1 )
	{
		key_handle();
		temperature_handle();
		AC_TEST();
		PWM_out();
		// Protect();
		clear_wdt();

	}
}


void Controll_Heat ( u16 temp_set,u16 temp_now )
{
//KEY_printf ( "temp_set val:%d \r\n",temp_set );
	if ( temp_now >  temp_set   )
	{
//		set_pwm ( 0 ); // 关闭加热丝
		HEAT_STD = 0;
	}
	else if ( temp_now < ( temp_set - Open_Heat_Value ) )
	{
//		set_pwm ( 10 ); //打开加热�
		HEAT_STD = 1;
	}
}

void Set_Temp ( u16 temp )
{
	if ( one_heat == 1 )
	{
		
		Controll_Heat ( One_Heat_Temp,temp );
	}
	else
	{
		
		switch ( flash_info.gap )
		{
			case GAP_WARM:
				Controll_Heat ( GAP_WARM_temp,temp );
				break;
			case GAP_1:
				Controll_Heat ( GAP_1_temp,temp );
				break;
			case GAP_2:
				Controll_Heat ( GAP_2_temp,temp );
				break;
			case GAP_3:
				Controll_Heat ( GAP_3_temp,temp );
				break;
			case GAP_4:
				Controll_Heat ( GAP_4_temp,temp );
				break;
			case GAP_5:
				Controll_Heat ( GAP_5_temp,temp );
				break;
			case GAP_6:
				Controll_Heat ( GAP_6_temp,temp );
				break;
			case GAP_7:
				Controll_Heat ( GAP_7_temp,temp );
				break;
			case GAP_8:
				Controll_Heat ( GAP_8_temp,temp );
				break;
			case GAP_9:
				Controll_Heat ( GAP_9_temp,temp );
				break;

		}
	}
}




void Protect ( void )
{
	if ( over_rang_time_std == 1 )
	{
		if ( flash_info.gap > GAP_3 )
		{
			flash_info.gap = GAP_3;
			lcd_display_gap ( flash_info.gap );
			flah_save_data();
			over_rang_time_std = 0;
		}
	}
}


