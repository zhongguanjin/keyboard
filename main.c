#include "Task_Main.h"
#include "main.h"
#include "timer.h"
#include "system.h"
#include "uart.h"

/*****************************************************************************
 函 数 名  : Init_Sys
 功能描述  : 系统初始化函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月18日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Init_Sys(void)
{
	Init_MCU();
    BSP_init();
	Init_UART1();
    //Init_UART2();
	Init_TMR0();
	GIE		= 1;
	PEIE	= 1;
	TMR0IE	= 1;				//开TMR0中断
}

/*****************************************************************************
 函 数 名  : main
 功能描述  : 主函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月18日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void main(void)
{
	Init_Sys();
	while(1)
	{
        if(Flg.frame_ok_fag == 1)
        {
            Flg.frame_ok_fag = 0;
            Serial_Processing();
        }
        TaskProcess();            // 任务处理
	    CLRWDT();
	};
}

/*****************************************************************************
 函 数 名  : ISR
 功能描述  : 中断服务函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月18日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void interrupt ISR(void)
{

    if(RCIE && RCIF)
    {
        RCIF=0;
        receiveHandler(RCREG);
    }
    if(TMR0IF && TMR0IE)     // 1ms中断一次
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
        TaskRemarks();       //任务标记轮询处理
    }
}
