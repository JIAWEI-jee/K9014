#ifndef __UART_H__
#define __UART_H__
#include "HC89F303.h"
#include "stdio.h"//printf

#define _DEBUG_PRINTF_    1
#define KEY_DEBUG_PRINT   1
#define ADC_DEBUG_PRINT   1


extern  u8 temper_val;
 extern u8 usart_rx_flag;
#if _DEBUG_PRINTF_
	#define	gm_printf printf
#else
	#define	gm_printf (...)
#endif

#if ADC_DEBUG_PRINT
	#define	ADC_printf printf
#else
	#define	ADC_printf (...)
#endif


#if KEY_DEBUG_PRINT
	#define	KEY_printf printf
#else
	#define	KEY_printf (...)
#endif

void uart_init(void);

void hal_uart_putchar(char Char);
void producte_send_cmd(u8 funcode, u8 Data);

#endif


