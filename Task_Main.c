
#include "Task_Main.h"
#include "uart.h"
#include "stdio.h"
#include <string.h>
#include "dbg.h"

volatile uint8  work_state ;
typedef struct
{
     uint16 sleep ;                   //睡眠时间
     uint8  temp38;                  //温度切换38度的时间
     uint16 temp_switch;             //进水时的温度和预设温度显示的时间计数
     uint8  switch_cnt;            //on,off和温度切换的时间计数
     uint8  key_adj;              //按键lamp,water,air使用+,-的使用时间
     uint8  incdec;                //+,-键led灯亮灭控制时间
     uint16 wifi_pair;
}tTime_T;

tTime_T Time_t;


volatile uint8  key_switch_fag;          //按键调节切换标志
volatile uint8  incdec_fag;
static uint8  flash_cnt;        //极限温度 连续闪烁的次数


//按键变量
uint8   KeyPressDown=0x00; //代表的是触发 第一次有效，后面会清0
uint8   CurrReadKey;  //记录本次KeyScan()读取的IO口键值
uint8   LastKey=0x00;    //代表的是连续按下

#define SIZE 4
static uint8 key_arry[SIZE]; //按键堆栈数组
static int top = 0;
static uint16 time_clean=0;
uint8 clean_state;        //clean 步骤
static uint8 err_lock=0;

//函数申明

void TaskShow(void);
void TaskKeyScan(void);
void TaskKeyPrs(void);
void set_temp_val_dec(uint8 val);
void set_temp_val_inc(uint8 val);

void TAP_EventHandler(void);
void SHOWER_EventHandler(void);
void DRAIN_EventHandler(void);
void INC_EventHandler(void);
void DEC_EventHandler(void);
void WATER_EventHandler(void);
void AIR_EventHandler(void);
void LAMP_EventHandler(void);
void LOCK_EventHandler(void);
void IDLE_EventHandler(void);
void key_adjust(uint8 id,uint8 dat);
void time_cnt_del( uint8 id);
void CLEAN_EventHandler(void);
void show_temp_flash(void);
void TaskClean();
void WIFI_EventHandler(void);
void key_work_process( void );
void judge_err_num(void);
void check_uart(void);

// 定义结构体变量
static TASK_COMPONENTS TaskComps[] =
{
    {0, 20,  20,  TaskKeyScan},               //按键扫描
    {0, 10,  10,  TaskKeyPrs},             //按键进程函数
    {0, 100, 100, TaskShow},               // 显示任务

};

// 任务清单
typedef enum _TASK_LIST
{
    TAST_SHOW,             // 显示温度
    TAST_KEY_SCAN,
	TAST_KEY_PRS,
    TASKS_MAX                // 总的可供分配的定时任务数目
} TASK_LIST;




/*****************************************************************************
 函 数 名  : TaskShow
 功能描述  : 温度显示任务
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月25日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskShow(void) //100ms
{
    uint16 temp=0;
	check_uart();
	temp=KeyCmd.req.dat[DAT_TEMP]*10;
    show_tempture(temp);
}


void check_uart(void)
{
	if ((RC1STAbits.FERR == 1) || (RC1STAbits.OERR == 1))
	{
		static uint8 error_rc = 0;
		error_rc = RCREG;
		NOP();
		error_rc = RCREG;
		RC1STAbits.CREN = 0;
		NOP();
		RC1STAbits.CREN = 1;
		Init_UART1();
	}
}
/*****************************************************************************
 函 数 名  : TaskKeyScan
 功能描述  : 按键扫描任务
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : man_sta
    修改内容   : 新生成函数

*****************************************************************************/

void TaskKeyScan(void)  //20ms
{
    KEY_SBIO_IN;       //将按键对应的IO设置为输入状态
    //KEY_DAT|=0Xff;
    CurrReadKey=(~KEY_DAT)&0Xff; //取反
    Button_id  = CurrReadKey;
    KeyPressDown=(~LastKey)&CurrReadKey; //第一次按下键值
    LastKey=CurrReadKey;                //连续按下键值
}
/*****************************************************************************
 函 数 名  : TaskKeyPrs
 功能描述  : 按键进程函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskKeyPrs(void)  //10MS
{
    uint8 id = 0;
    static uint16 count =0;
#if 0
    id =  Button_id&0X1f;
#else
    id =  Button_id&0Xff; //ff
#endif
    switch ( id )
    {
        case TAP_VALVE:
        {
            TAP_EventHandler();
            break;
        }
        case SHOWER_VALVE:
        {
            SHOWER_EventHandler();
            break;
        }
        case DRAIN_VALVE:
        {
            DRAIN_EventHandler();
            break;
        }
        case INC_VALVE:
        {
            INC_EventHandler();
            break;
        }
        case DEC_VALVE:
        {
            DEC_EventHandler();
            break;
        }
        case WATER_VALVE:
        {
            WATER_EventHandler();
            break;
        }
        case AIR_VALVE:
        {
            AIR_EventHandler();
            break;
        }
        case LAMP_VALVE:
        {
            LAMP_EventHandler();
            break;
        }
        default:
        {
            break;
        }
    }
}




/*****************************************************************************
 函 数 名  : TAP_EventHandler
 功能描述  : 龙头按键处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TAP_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&TAP_VALVE) //第一次按下
         {
            KeyPressDown = 0;
            ShowPar.tap_state ^= 0x01;
            if(ShowPar.tap_state == STATE_ON)
            {
                LED_TAP_ON;
            }
            else
            {
               LED_TAP_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_1;  // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.tap_state; //数据码 龙头状态
        }
    }
}
/*****************************************************************************
 函 数 名  : SHOWER_EventHandler
 功能描述  : 花洒按键处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void SHOWER_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&SHOWER_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            ShowPar.shower_state ^= 0x01;
            if( ShowPar.shower_state == STATE_ON)
            {
                LED_SHOWER_ON;
            }
            else
            {
               LED_SHOWER_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_2;            // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.shower_state; //数据码 花洒
        }
    }
}
/*****************************************************************************
 函 数 名  : DRAIN_EventHandler
 功能描述  : 排水按键处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void DRAIN_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&DRAIN_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            ShowPar.drain_state ^= 0x01;
            if( ShowPar.drain_state == STATE_ON)
            {
                LED_DRAIN_ON;
            }
            else
            {
               LED_DRAIN_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_3;
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //数据码 排水
        }
    }
}
/*****************************************************************************
 函 数 名  : INC_EventHandler
 功能描述  : 增加按键处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void INC_EventHandler(void)
{
    static uint8 time_cnt = 0;
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&INC_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            ShowPar.inc_state ^= 0x01;
            if( ShowPar.inc_state == STATE_ON)
            {
                LED_INC_ON;
            }
            else
            {
               LED_INC_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_4;
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.inc_state; //数据码 排水
        }
    }
}
/*****************************************************************************
 函 数 名  : DEC_EventHandler
 功能描述  : 减少按键处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void DEC_EventHandler(void)
{
    static uint8 time_cnt = 0;
   if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&DEC_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            ShowPar.dec_state ^= 0x01;
            if( ShowPar.dec_state == STATE_ON)
            {
                LED_DEC_ON;
            }
            else
            {
               LED_DEC_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_5;
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.dec_state; //数据码 排水
        }
    }
}
/*****************************************************************************
 函 数 名  : AIR_EventHandler
 功能描述  : 气按摩处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void AIR_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown &AIR_VALVE)
        {
            KeyPressDown = 0;
            ShowPar.air_state ^= 0x01;
            if( ShowPar.air_state == STATE_ON)
            {
                LED_AIR_ON;
            }
            else
            {
               LED_AIR_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_7;            // 功能码：07
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.air_state; //数据码 排水
        }
    }
}
/*****************************************************************************
 函 数 名  : WATER_EventHandler
 功能描述  : 水按摩处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void WATER_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&WATER_VALVE)
        {
            KeyPressDown = 0;
            ShowPar.water_state ^= 0x01;
            if( ShowPar.water_state == STATE_ON)
            {
                LED_WATER_ON;
            }
            else
            {
               LED_WATER_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_6;            // 功能码：06
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.water_state; //数据码 排水
        }
    }
}
/*****************************************************************************
 函 数 名  : LAMP_EventHandler
 功能描述  : 灯光按键处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月5日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void LAMP_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&LAMP_VALVE)
        {
            KeyPressDown = 0;
            ShowPar.lamp_state ^= 0x01;
            if(ShowPar.lamp_state == STATE_ON) //打开灯，接着判断有无+ -按键
            {
               LED_LAMP_ON;
            }
            else  //关灯off
            {
                LED_LAMP_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_8;  // 功能码:06
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_state;
        }
    }
}

/*****************************************************************************
 函 数 名  : CRC8_SUM
 功能描述  : CRC校验函数
 输入参数  : void *p
             uint8 len
 输出参数  : crc8        --check sum
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年6月30日 星期五
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
uint8 CRC8_SUM(void *p,uint8 len)
{
    uint8 crc8 = 0;
    uint8 *temp =p;
    for(uint8 i=0;i<len;i++)
    {
        crc8 ^=*temp;
        temp++;
    }
    return crc8;
}

/*****************************************************************************
 函 数 名  : Serial_Processing
 功能描述  : 串口接收处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void Serial_Processing (void)
{
    static uint8 state=0;
    memcpy(&KeyCmd.rsp,Recv_Buf,sizeof(KeyCmd.rsp));
    KeyCmd.req.dat[DAT_TEMP] =KeyCmd.rsp.dat[DAT_TEMP];
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    send_dat(&KeyCmd.req, BUF_SIZE);
    KeyCmd.req.dat[DAT_FUN_CMD]=0;          //清功能码
    memset(&KeyCmd.rsp,0,sizeof(KeyCmd.rsp));
}
/*****************************************************************************
 函 数 名  : receiveHandler
 功能描述  : 串口接收回调函数
 输入参数  : uint8 ui8Data
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/

void receiveHandler(uint8 ui8Data)
{
    uint8 check_sum = 0;
    static uint8 Recv_Len = 0;    // 接收长度
    static uint8 err_cnt= 0;
    Recv_Buf[Recv_Len] = ui8Data;
    if(Recv_Buf[0]==0x02)
    {
        Recv_Len++;
        if(Recv_Len >= BUF_SIZE) //接收到32byte的数据
        {
            if((Recv_Buf[1]==0x03A)&&(Recv_Buf[2]==0x01))
            {
                if((Recv_Buf[31]== 0x04)&&(Recv_Buf[30]== 0x0B))
                {
                    for(uint8 i=2;i<(crc_len+2);i++)
                    {
                        check_sum^=Recv_Buf[i];
                    }
                    if(check_sum == Recv_Buf[29])
                    //if(CRC8_SUM(&Recv_Buf[2], crc_len) == Recv_Buf[29])
                    {
                        Recv_Len = 0;
                        frame_err=0;
                        err_cnt=0;
                        Flg.frame_ok_fag=1;
                    }
                    else
                    {
                        Recv_Len = 0;
                        Flg.frame_ok_fag=0;
                        err_cnt++;
                        if( err_cnt>=10)
                        {
                            err_cnt=0;
                            frame_err=1;
                        }
                    }
                }
                else if((Recv_Buf[31]!= 0x04)||(Recv_Buf[30]!= 0x0B))  //结束码不对
                {
                    Recv_Len = 0;
                    err_cnt++;
                    if(err_cnt>=10)
                    {
                        err_cnt=0;
                        frame_err=1;
                    }
                    Flg.frame_ok_fag = 0;
                }
            }
            else if((Recv_Buf[1]!=0x03A)||(Recv_Buf[2]!=0x01)) //
            {
                Recv_Len = 0;
                Flg.frame_ok_fag = 0;
            }
       }
    }
    else if(Recv_Buf[0]!=0x02)
    {
        Recv_Len = 0;
        Flg.frame_ok_fag = 0;
    }
}



/*****************************************************************************
 函 数 名  : BSP_init
 功能描述  : BSP初始化函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年6月29日 星期四
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void BSP_init(void)
{
    LED_INIT();
    KEY_SBIO_IN;  //RB口设置成输入模式
    /* BEGIN: Added by zgj, 2018/1/5 */
    //初始化灯颜色，按摩档位
    //ShowPar.lamp_gear = LAMP_PINK;
    //ShowPar.water_gear = MASSAGE_GEAR_ON3;
    //ShowPar.air_gear = MASSAGE_GEAR_ON3;
    /* END:   Added by zgj, 2018/1/5 */
    KeyCmd.req.sta_num1 = 0x02;
    KeyCmd.req.sta_num2  = 0xA3;
    KeyCmd.req.dat[DAT_ADDR]  = 0x01;
    //KeyCmd.req.dat[DAT_FLOW] = 0x64;
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    KeyCmd.req.end_num1 = 0x0F;
    KeyCmd.req.end_num2 = 0x04;
    work_state = WORK_STATE_IDLE;
}

/*****************************************************************************
 函 数 名  : TaskRemarks
 功能描述  : 任务标记处理函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月24日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskRemarks(void)
{
    uint8 i;
    for (i=0; i<TASKS_MAX; i++)                                 // 逐个任务时间处理
    {
         if (TaskComps[i].Timer)                                // 时间不为0
         {
            TaskComps[i].Timer--;                                // 减去一个节拍
            if (TaskComps[i].Timer == 0)                            // 时间减完了
            {
                 TaskComps[i].Timer = TaskComps[i].ItvTime;       // 恢复计时器值，从新下一次
                 TaskComps[i].Run = ON;                             // 任务可以运行
            }
        }
   }
}
/*****************************************************************************
 函 数 名  : TaskProcess
 功能描述  : 任务进程函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年5月24日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskProcess(void)
{
    uint8 i;
    for (i=0; i<TASKS_MAX; i++)           // 逐个任务时间处理
    {
         if (TaskComps[i].Run)           // 时间不为0
        {
             TaskComps[i].TaskHook();         // 运行任务
             TaskComps[i].Run = 0;          // 标志清0
        }
    }
}


