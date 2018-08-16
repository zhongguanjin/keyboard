#ifndef __UART_H__
#define __UART_H__

#include "config.h"
#include "LCD.h"

extern void Init_UART1( void );
extern void uart_send_byte(char ch);
extern void uart_send_str( uint8 *s);
extern void send_dat(void * p,uint8 len);

extern void Init_UART2(void);
extern void usart2_send_byte(char dat);

#endif


