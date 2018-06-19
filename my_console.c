#include "my_console.h"
#include "system.h"
#include "dbg.h"
#include "uart.h"

#include "Task_Main.h"



void uart_bufInit(uart_buf_t * pBuf)
{
	pBuf->in = 0;
	pBuf->out = 0;
}
/*****************************************************************************
 函 数 名  : uart2_getch
 功能描述  : 串口获取字符函数
 输入参数  : char * p
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年2月9日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
int uart1_getch(char * p)
{
	if(uart1rx.in != uart1rx.out)
	{
		*p =  uart1rx.buf[uart1rx.out & CONSOLE_RX_BUF_MASK];
		uart1rx.out++;
		return 0;
	}
	else
	{
		return -1;
	}
}


void console_process(void)
{
    uint8 ch;
    uint8 check_sum = 0;
    static uint8 index=0;
   // static uint8 state =0;
    while(1)
    {
        if(0 == uart1_getch(&ch))
        {
            switch(index)
            {
                case 0: //0x02
                {
                    if(ch != 0x02)
                    {
                        break;
                    }
                    Recv_Buf[index]=ch;
                    index = 1;
                    break;
                }
                case 1://0x3A
                {
                    if(ch != 0x3A)
                    {
                        index = 0;
                        break;
                    }
                    Recv_Buf[index]=ch;
                    index = 2;
                    break;
                }
                case 2://0x01
                {
                    if(ch !=0x01)
                    {
                        index = 0;
                        break;
                    }
                    Recv_Buf[index]=ch;
                    check_sum =ch;
                    index = 3;
                    break;
                }
                case 3: //dat
                {
                    if(index < BUF_SIZE)
                    {
                        Recv_Buf[index]=ch;
                    }
                    check_sum ^=ch;
                    index++;
                    if(index == crc_len)
                    {
                        index =29;
                    }
                    break;
                }
                case 29: //check_sum
                    {
                        if(ch !=check_sum)
                        {
                            frame_err=1;
                            index = 0;
                            break;
                        }
                        Recv_Buf[index]=ch;
                        index = 30;
                        break;
                    }
                case 30:
                    {
                        if(ch !=0x0B)
                        {
                            frame_err=1;
                            index = 0;
                            break;
                        }
                        Recv_Buf[index]=ch;
                        index = 31;
                        break;
                    }
                case 31:
                    {
                        if(ch !=04)
                        {
                            frame_err=1;
                            index = 0;
                            break;
                        }
                        Recv_Buf[index]=ch;
                        frame_err=0;
                        Flg.frame_ok_fag=1;
                        index = 0;
                        break;
                    }
                default:
                    index = 0;
                    break;
            }

        }
        else
        {
           break;
        }
    }
}





