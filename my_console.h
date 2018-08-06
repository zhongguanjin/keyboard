
#ifndef __MY_CONSOLE_H__
#define __MY_CONSOLE_H__

#include "config.h"




#define	CONSOLE_RX_BUF_LEN		64	// must 2**n
#define	CONSOLE_RX_BUF_MASK	(CONSOLE_RX_BUF_LEN-1)

typedef struct _UART_BUF_TAG
{
	unsigned char	in;
	unsigned char	out;
	unsigned char	buf[CONSOLE_RX_BUF_LEN];
} uart_buf_t;

uart_buf_t		uart1rx;

#define RX_START_ST		0
#define RX_SPARE1_ST	1
#define RX_SPARE2_ST	2
#define RX_DATA_ST		3
#define RX_CHK_ST			4
#define RX_END_ST			5
#define RX_END_ST2			6

extern void console_process(void);
extern void uart_bufInit(uart_buf_t * pBuf);
extern void my_console_receive(uint8 ui8Data);


#endif




