#ifndef _DBG_H
#define _DBG_H
#include <stdio.h>
#include <stdarg.h>
#include "uart.h"
#include "config.h"
#include<stdlib.h>
#include<string.h>

#define MAX 6       //����������С�����ֵ�λ��


void  my_printf(const char *pFormat, ...);
extern void dbg_hex(char *buf,char len);






//void my_print(uint8 *Data,...);


#define dbg  (my_printf("[%d]",__LINE__),my_printf)


#endif

