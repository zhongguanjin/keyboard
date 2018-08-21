#include "Task_Main.h"
#include "main.h"
#include "timer.h"
#include "system.h"
#include "uart.h"
#include "my_console.h"
#include "LCD.h"
#include "dbg.h"


/*****************************************************************************
 �� �� ��  : Init_Sys
 ��������  : ϵͳ��ʼ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��5��18�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Init_Sys(void)
{
	Init_MCU();
    BSP_init();
	Init_UART1();
#if dbglog
	Init_UART2();
#endif
	Init_TMR0();
	//Init_TMR6();
	GIE		= 1;
	PEIE	= 1;
	TMR0IE	= 1;				//��TMR0�ж�
	//TMR6IE	= 1;				//��TMR6�ж�
}

void wdt_enable(void)
{
    SWDTEN =1;
    WDTCONbits.WDTPS = 0x0B;
}
void wdt_disable(void)
{
    SWDTEN =0;
}

/*****************************************************************************
 �� �� ��  : main
 ��������  : ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��5��18�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void main(void)
{
    wdt_disable();
	Init_Sys();
    uart_bufInit(&uart1rx);
    wdt_enable();
	while(1)
	{
        console_process();
        if(Flg.frame_ok_fag == 1)
        {
            clear_f6_cnt();
            Flg.frame_ok_fag = 0;
            Serial_Processing();
        }
        TaskProcess();            // ������
	    CLRWDT();
	};
}

/*****************************************************************************
 �� �� ��  : ISR
 ��������  : �жϷ�����
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��5��18�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void interrupt ISR(void)
{
    if(TMR0IF && TMR0IE)     // 1ms�ж�һ��
    {
        TMR0IF = 0;
        TMR0 = TMR0+TMR0_VALUE;
        static uint8  led_count = 0;
        led_count ++ ;
        if(led_count>=5)// 200hz=5ms
        {
            led_count = 0;
            led_scan();
        }
        TaskRemarks();       //��������ѯ����
    }
    if(RCIE && RCIF)
    {
        RCIF=0;
       my_console_receive(RCREG);
    }
}
