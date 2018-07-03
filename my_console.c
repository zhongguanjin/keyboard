#include "my_console.h"
#include "system.h"
#include "dbg.h"
#include "uart.h"

#include "Task_Main.h"

int uart1_getch(char * p);


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
    static uint8 check_sum = 0;
    static uint8 index=0;
    static uint8 st=0;
    while(1)
    {
        if(0 == uart1_getch(&ch))
        {
            switch(st)
            {
                case RX_START_ST: //0x02
                {
                    index = 0;
                    if(ch != 0x02)
                    {
                        st=RX_START_ST;
                        break;
                    }
                    check_sum = 0;
                    Recv_Buf[index]=ch;
                    index++;
                    st=RX_SPARE1_ST;
                    break;
                }
                case RX_SPARE1_ST://0x3A
                {
                    if(ch == 0x02) //排重问题
                    {
                        index=0;
                        Recv_Buf[index]=ch;
                        index++;
                        st=RX_SPARE1_ST;
                    }
                    else if(ch == 0x3A)
                    {
                        Recv_Buf[index]=ch;
                        index++;
                        st=RX_SPARE2_ST;
                    }
                    else
                    {
                       st=RX_START_ST;
                    }
                    break;
                }
                case RX_SPARE2_ST://0x04
                {
                    if(ch == 0x02)
                    {
                        index =0;
                        Recv_Buf[index]=ch;
                        index++;
                        st=RX_SPARE1_ST;
                    }
                    else if(ch == 0x04)
                    {
                        check_sum =ch;
                        Recv_Buf[index]=ch;
                        index++;
                        st=RX_DATA_ST;
                    }
                    else
                    {
                       st=RX_START_ST;
                    }
                    break;
                }
                case RX_DATA_ST: //dat
                {
                    Recv_Buf[index]=ch;
                    check_sum ^=ch;
                    index++;
                    if(index == 29)
                    {
                        st =RX_CHK_ST;
                    }
                    break;
                }
                case RX_CHK_ST: //check_sum
                    {
                        if(ch ==check_sum)
                        {
                            st=RX_END_ST;
                            Recv_Buf[index]=ch;
                            index++;
                        }
                        else
                        {
                           st=RX_START_ST;
                        }
                        break;
                    }
                case RX_END_ST:
                    {
                        if(ch !=0x0F)
                        {
                           st=RX_START_ST;
                            break;
                        }
                        Recv_Buf[index]=ch;
                        index++;
                        st=RX_END_ST2;
                        break;
                    }
                case RX_END_ST2:
                    {
                        if(ch ==04)
                        {
                            Recv_Buf[index]=ch;
                            Flg.frame_ok_fag=1;
                        }
                        st=RX_START_ST;
                        break;
                    }
                default:
                    index = 0;
                    st=RX_START_ST;
                    break;
            }

        }
        else
        {
           break;
        }
    }
}


void my_console_receive(uint8 ui8Data)
{
     uart1rx.buf[uart1rx.in++&CONSOLE_RX_BUF_MASK] =ui8Data;
}



