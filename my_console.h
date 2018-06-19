
#ifndef __MY_CONSOLE_H__
#define __MY_CONSOLE_H__

#include "config.h"

#define BUF_MAX 11



#define	CONSOLE_RX_BUF_LEN		64	// must 2**n
#define	CONSOLE_RX_BUF_MASK	(CONSOLE_RX_BUF_LEN-1)

typedef struct _UART_BUF_TAG
{
	unsigned char	in;
	unsigned char	out;
	unsigned char	buf[CONSOLE_RX_BUF_LEN];
} uart_buf_t;

uart_buf_t		uart1rx;


extern void console_process(void);
void uart_bufInit(uart_buf_t * pBuf);
int uart1_getch(char * p);



#endif




