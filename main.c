#include "task_main.h"
#include "main.h"
#include "timer.h"
#include "system.h"
#include "uart.h"


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
	Init_UART1();
    BSP_init();
	Init_TMR0();
	//Init_TMR6();
	GIE		= 1;
	PEIE	= 1;
	TMR0IE	= 1;				//��TMR0�ж�
	//TMR6IE	= 1;				//��TMR6�ж�
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
	Init_Sys();
	while(1)
	{
        if(frame_ok_fag == 1)
        {
            frame_ok_fag = 0;
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
/*
	if (TMR6IF && TMR6IE) // 100us �ж�һ��
	{
	    TMR6IF = 0;
        static uint8  led_count = 0;
        led_count ++ ;
        if(led_count>=2)
        {
            led_count = 0;
            led_scan();
        }
	}
	*/
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
        receiveHandler(RCREG);
    }
}
