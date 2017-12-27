#ifndef __UART_H__
#define __UART_H__

#include "config.h"
#include "lcd.h"

extern void Init_UART1( void );
extern void usart1_send_byte(char ch);
extern void uart_send_str( uint8 *s);
extern void send_dat(void * p,uint8 len);

#endif


