
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
     uint16 temp_switch;                //进水时的温度和预设温度显示的时间计数
     uint8  switch_cnt;            //on,off和温度切换的时间计数
     uint8  key_adj;              //按键lamp,water,air使用+,-的使用时间
     uint8  incdec;                //+,-键led灯亮灭控制时间
}tTime_T;

tTime_T Time_t;


volatile uint8  key_gear;                 //按键调节档位
volatile uint8  key_switch_fag;          //按键调节切换标志
volatile uint8  incdec_fag;
uint8 lcd_flag = 0;   //lcd开启以关闭的状态标志

//按键变量
uint8   KeyPressDown=0x00; //代表的是触发 第一次有效，后面会清0
uint8   CurrReadKey;  //记录本次KeyScan()读取的IO口键值
uint8   LastKey=0x00;    //代表的是连续按下

#define SIZE 4
static uint8 key_arry[SIZE]; //按键堆栈数组
static int top = 0;

//函数申明
uint8 CRC8_SUM(void * p, uint8 len);
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
void TaskShow(void)
{
    show_work();
    if(key_switch_fag ==0)
    {
        show_temp_actul();
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
    Button.id = CurrReadKey;
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
    id = Button.id&0Xff;
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
		case CLEAN_VALVE:
		{
			CLEAN_EventHandler();
            break;
		}
        case LOCK_VALVE:
        {
            static uint16 count =0;
            if((count++)>=300)
            {
                count = 0;
                LOCK_EventHandler();
            }
           break;
        }
        default:
        {
            break;
        }
    }
    IDLE_EventHandler();
}

/* 添加 */
void add(uint8 value)
{
    if(top == SIZE-1) //满
    {
        key_arry[top] = value;
        return ;
    }
    top += 1;
    key_arry[top] = value;
}

void clear(void)
{
    if(top == 0)  //空
    {
        return ;
    }
    if(key_arry[top] != 0x00)
    {
        for(uint8 i=0;i<=top;i++)
        {
           key_arry[i]=0;
        }
        top = 0;
    }
}
/* 删除 */
void del(uint8 value)
{
    if(top == 0)  //空
    {
        return ;
    }
    if(key_arry[top] == value) //是栈顶元素
    {
        key_arry[top] = 0; //清零
        top = 0;
    }
    else
    {
        for(uint8 i=0;i<top;i++)
        {
            if(key_arry[i] == value)
            {
                for(uint8 j=i; j<top;j++) //在i数组后面的元素往前移
                {
                    key_arry[j]=key_arry[j+1];
                }
                key_arry[top] = 0; //清零
                top -= 1;  //移完后需把栈顶标号减1
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : key_adjust
 功能描述  : 按键调节功能：灯光，水按摩，气按摩
 输入参数  : uint8 id
             uint8 dat
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年12月1日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void key_adjust(uint8 id,uint8 dat)
{
    key_switch_fag =1;
    show_adj_key(id, dat);
}

/*****************************************************************************
 函 数 名  : time_cnt_del
 功能描述  : 时间计数清零函数
 输入参数  : uint8 id -按键值
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月6日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void time_cnt_del( uint8 id)
{
    Time_t.sleep = 0;
    Time_t.temp38 = 0;
    if((id==TAP_VALVE)||(id==SHOWER_VALVE))//龙头或花洒
    {
        Time_t.switch_cnt = 0;                  //on /off 间隔时间要清0
        Time_t.temp_switch =0;
    }
    if((id==INC_VALVE)||(id==DEC_VALVE))//+,-
    {
        if(key_switch_fag==0) //调节模式不要清除temp_switch的时间
        {
            Time_t.temp_switch =0;
        }
        Time_t.incdec = 0;
        Time_t.key_adj = 0;
        incdec_fag =1;
    }
    if((id==AIR_VALVE)||(id==WATER_VALVE)||(id==LAMP_VALVE))//按摩，灯光键
    {
         Time_t.key_adj = 0;
    }
    if(id==DRAIN_VALVE)
    {
       Time_t.switch_cnt =0;
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
            if(lcd_flag == 1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return;
            }
            time_cnt_del(TAP_VALVE);
            ShowPar.tap_state ^= 0x01;
            if(ShowPar.tap_state == STATE_ON)
            {
                ShowPar.shower_state = STATE_OFF;
                ShowPar.drain_state = STATE_OFF;
                LED_TAP_ON;
                LED_SHOWER_OFF;
                LED_DRAIN_OFF;
            }
            else
            {
               LED_TAP_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;  // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_STATE] = ShowPar.val&0x03;
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //数据码 龙头状态
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.tap_state);
            dbg("tap,dat[%d]:%x\r\n",DAT_STATE,KeyCmd.req.dat[DAT_STATE]);
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
            time_cnt_del(SHOWER_VALVE);
            if(lcd_flag == 1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return;
            }
            ShowPar.shower_state ^= 0x01;
            if( ShowPar.shower_state == STATE_ON)
            {
                ShowPar.tap_state = STATE_OFF;
                LED_SHOWER_ON;
                LED_TAP_OFF;
            }
            else
            {
               LED_SHOWER_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_STATE] = ShowPar.val&0x03;
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //数据码 花洒
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.shower_state);
            dbg("shower,dat[%d]:%x\r\n",DAT_STATE,KeyCmd.req.dat[DAT_STATE]);
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
            time_cnt_del(DRAIN_VALVE);
            if(lcd_flag == 1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.drain_state ^= 0x01;
            if( ShowPar.drain_state == STATE_ON)
            {
                LED_DRAIN_ON;
            }
            else
            {
               LED_DRAIN_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_DRAINAGE;
            KeyCmd.req.dat[DAT_DRAIN] = ShowPar.drain_state;
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //数据码 排水
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.drain_state);
            dbg("drainage,dat[%d]:%x\r\n",DAT_DRAIN,KeyCmd.req.dat[DAT_DRAIN]);
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
        LED_INC_ON;
        LED_DEC_OFF;
        time_cnt_del(INC_VALVE);
        if(lcd_flag == 1)
        {
            lcd_flag = 0;
            show_tempture( ShowPar.temp_val);
            return;
        }
        switch (key_arry[top])
        {
            case ALL_CLOSE :
                {
                    time_cnt++;
                    if(KeyPressDown&INC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        time_cnt = 0;
                        set_temp_val_inc(5);
                    }
                    else if((LastKey&INC_VALVE)&&(time_cnt>=40)) //连续按下400MS
                    {
                         time_cnt = 0;
                         if(ShowPar.temp_val>=380)
                         {
                            set_temp_val_inc(5);
                         }
                         else
                         {
                            set_temp_val_inc(10);
                         }
                    }
                    break;
                }
            case LAMP_VALVE:
                {
                    if(KeyPressDown&INC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        /*
                        if(ShowPar.lamp_gear == LAMP_CYCLE) // 如果是循环状态,按下就定色
                        {
                          ShowPar.lamp_gear = LAMP_STOP;
                        }
                        else
                        {
                           ShowPar.lamp_gear++;
                           if(ShowPar.lamp_gear >LAMP_WHITE)
                           {
                             ShowPar.lamp_gear = LAMP_RED;
                           }
                        }
                        */
                        /* BEGIN: Added by zgj, 2018/1/5 */
                        ShowPar.lamp_gear++;
                        if(ShowPar.lamp_gear >LAMP_CYCLE)
                        {
                          ShowPar.lamp_gear = LAMP_RED;
                        }
                        /* END:   Added by zgj, 2018/1/5 */
                        key_gear = ShowPar.lamp_gear;
                        key_adjust(key_arry[top],key_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // 功能码:08
                        KeyCmd.req.dat[DAT_LIGHT] = ShowPar.lamp_gear;
                        KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_gear;
                        dbg("lamp dat[%d]:%x\r\n",DAT_LIGHT, KeyCmd.req.dat[DAT_LIGHT]);
                    }
                    break;
                }
            case WATER_VALVE:
                {
                    if(KeyPressDown&INC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        ShowPar.water_gear++;
                        if(ShowPar.water_gear >MASSAGE_GEAR_ON5)
                        {
                            ShowPar.water_gear = MASSAGE_GEAR_ON5;
                        }
                        key_gear = ShowPar.water_gear;
                        key_adjust(key_arry[top],key_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:05
                        KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5); //数据码
                        dbg("massage dat[%d]:%x\r\n",DAT_MASSAGE, KeyCmd.req.dat[DAT_MASSAGE]);
                    }
                    break;
                }
            case AIR_VALVE:
                {
                    if(KeyPressDown&INC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        ShowPar.air_gear++;
                        if(ShowPar.air_gear >MASSAGE_GEAR_ON5)
                        {
                            ShowPar.air_gear = MASSAGE_GEAR_ON5;
                        }
                        key_gear = ShowPar.air_gear;
                        key_adjust(key_arry[top],key_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:07
                        KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5); //数据码
                        dbg("massage dat[%d]:%x\r\n",DAT_MASSAGE, KeyCmd.req.dat[DAT_MASSAGE]);
                    }
                    break;
                }
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
        LED_DEC_ON;
        LED_INC_OFF;
        time_cnt_del(DEC_VALVE); //清零时间
        if(lcd_flag == 1)//休眠唤醒
        {
            lcd_flag = 0;
            show_tempture( ShowPar.temp_val);
            return;
        }
        switch (key_arry[top])
        {
            case ALL_CLOSE :
                {
                    time_cnt++;
                    if(KeyPressDown&DEC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        time_cnt = 0;
                        set_temp_val_dec(5);
                    }
                    else if((LastKey&DEC_VALVE)&&(time_cnt>=40)) //连续按下
                    {
                         time_cnt = 0;
                         set_temp_val_dec(10);
                    }
                    break;
                }
            case LAMP_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        /*
                        if(ShowPar.lamp_gear == LAMP_CYCLE) // 如果是循环状态,按下就定色
                        {
                            ShowPar.lamp_gear = LAMP_STOP;
                        }
                        else
                        {
                            if(ShowPar.lamp_gear > LAMP_CYCLE)
                            {
                                ShowPar.lamp_gear=ShowPar.lamp_gear-2;
                            }
                            else
                            {
                                 ShowPar.lamp_gear--;
                                 if(ShowPar.lamp_gear < LAMP_RED)
                                 {
                                     ShowPar.lamp_gear =LAMP_WHITE;
                                 }
                            }
                       }
                       */
                       /* BEGIN: Added by zgj, 2018/1/5 */
                        ShowPar.lamp_gear--;
                        if(ShowPar.lamp_gear < LAMP_RED)
                        {
                            ShowPar.lamp_gear =LAMP_CYCLE;
                        }
                       /* END:   Added by zgj, 2018/1/5 */
                       key_gear = ShowPar.lamp_gear;
                       key_adjust(key_arry[top],key_gear);
                       KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // 功能码:06
                       KeyCmd.req.dat[DAT_LIGHT]=ShowPar.lamp_gear;
                       KeyCmd.req.dat[DAT_VALVE]=ShowPar.lamp_gear;
                       dbg("lamp dat[%d]:%x\r\n",DAT_LIGHT, KeyCmd.req.dat[DAT_LIGHT]);
                    }
                    break;
                }
            case WATER_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        ShowPar.water_gear--;
                        if(ShowPar.water_gear <MASSAGE_GEAR_ON1)
                        {
                            ShowPar.water_gear = MASSAGE_GEAR_ON1;
                        }
                        key_gear = ShowPar.water_gear;
                        key_adjust(key_arry[top],key_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:07
                        KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        dbg("massage dat[%d]:%x\r\n",DAT_MASSAGE, KeyCmd.req.dat[DAT_MASSAGE]);
                    }
                    break;
                }
            case AIR_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        ShowPar.air_gear--;
                        if(ShowPar.air_gear <MASSAGE_GEAR_ON1)
                        {
                            ShowPar.air_gear = MASSAGE_GEAR_ON1;
                        }
                        key_gear = ShowPar.air_gear;
                        key_adjust(key_arry[top],key_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:07
                        KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        dbg("massage dat[%d]:%x\r\n",DAT_MASSAGE, KeyCmd.req.dat[DAT_MASSAGE]);
                    }
                    break;
                }
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
            time_cnt_del(AIR_VALVE);
            if(lcd_flag ==1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.air_state ^= 0x01;
            if( ShowPar.air_state == STATE_ON)
            {
                LED_AIR_ON;
                add(AIR_VALVE);
                KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                //ShowPar.air_gear= MASSAGE_GEAR_ON3;
                key_gear = ShowPar.air_gear;
                key_adjust(key_arry[top],key_gear);
            }
            else
            {
               LED_AIR_OFF;
               del(AIR_VALVE);
               KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
               KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
               //ShowPar.air_gear= MASSAGE_GEAR_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // 功能码：07
            dbg("massage dat[%d]:%x\r\n",DAT_MASSAGE, KeyCmd.req.dat[DAT_MASSAGE]);
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
            time_cnt_del(WATER_VALVE);
            if(lcd_flag ==1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.water_state ^= 0x01;
            if( ShowPar.water_state == STATE_ON)
            {
                LED_WATER_ON;
                add(WATER_VALVE);
                KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                //ShowPar.water_gear= MASSAGE_GEAR_ON3;
                key_gear = ShowPar.water_gear;
                key_adjust(key_arry[top],key_gear);
            }
            else
            {
               LED_WATER_OFF;
               del(WATER_VALVE);
               KeyCmd.req.dat[DAT_MASSAGE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
               KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
               //ShowPar.water_gear= MASSAGE_GEAR_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // 功能码：07
            dbg("massage dat[%d]:%x\r\n",DAT_MASSAGE, KeyCmd.req.dat[DAT_MASSAGE]);
        }
    }
	if(work_state == WORK_STATE_CLEAN)
	{
		LED_WATER_OFF;
		LED_INC_OFF;
		work_state =WORK_STATE_IDLE;
		KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //功能码
		KeyCmd.req.dat[DAT_VALVE] =0x00;
		KeyCmd.req.dat[DAT_CLAEN] =0x00;
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
            time_cnt_del(LAMP_VALVE);
            if(lcd_flag ==1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.lamp_state ^= 0x01;
            if(ShowPar.lamp_state == STATE_ON) //打开灯，接着判断有无+ -按键
            {
               LED_LAMP_ON;
               KeyCmd.req.dat[DAT_LIGHT] = ShowPar.lamp_gear;
               KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_gear;
               //ShowPar.lamp_gear = LAMP_CYCLE;
               add(LAMP_VALVE);
               key_gear = ShowPar.lamp_gear;
               key_adjust(key_arry[top],key_gear);
            }
            else  //关灯off
            {
                LED_LAMP_OFF;
                KeyCmd.req.dat[DAT_LIGHT] = LAMP_OFF;
                KeyCmd.req.dat[DAT_VALVE] = LAMP_OFF;
               // ShowPar.lamp_gear = LAMP_OFF;
                del(LAMP_VALVE);
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // 功能码:06
            dbg("lamp dat[%d]:%x\r\n",DAT_LIGHT, KeyCmd.req.dat[DAT_LIGHT]);

        }
    }
}

/*****************************************************************************
 函 数 名  : LOCK_EventHandler
 功能描述  : 童锁按键处理函数
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
void LOCK_EventHandler(void) //10ms
{
    if(work_state == WORK_STATE_IDLE)
    {
        work_state = WORK_STATE_LOCK;
        ShowPar.shower_state = OFF;
        ShowPar.tap_state = OFF;
        LED_TAP_OFF;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // 功能码：进水开关改变
        KeyCmd.req.dat[DAT_VALVE] = 0x01;
        KeyCmd.req.dat[DAT_LOCK] = 0x01;
        dbg("idle -> lock,dat[%d]:%x\r\n",DAT_VALVE,KeyCmd.req.dat[DAT_VALVE]);
    }
    else if(work_state == WORK_STATE_LOCK)
    {
       show_tempture( ShowPar.temp_val);
       work_state = WORK_STATE_IDLE;
       KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // 功能码：进水开关改变
       KeyCmd.req.dat[DAT_VALVE] = 0x00;
       KeyCmd.req.dat[DAT_LOCK] = 0x00;
       dbg("lock -> idle\r\n");
    }
}
/*****************************************************************************
 函 数 名  : IDLE_EventHandler
 功能描述  : 空闲回调函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月4日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void IDLE_EventHandler(void) //10ms
{
    if( work_state == WORK_STATE_LOCK)
    {
        if((Time_t.sleep%100)==40)//400ms
        {
            show_lock();
        }
        if((Time_t.sleep%100)==90)//900ms
        {
           show_sleep();
        }
    }
    if(Time_t.sleep++ >=6000) // 1min钟时间到  时基10ms
    {
       Time_t.sleep = 0;
       Time_t.temp38++;
       if(work_state == WORK_STATE_IDLE)
       {
            if(ShowPar.val == OFF)
            {
                show_sleep();
                lcd_flag = 1;  //置位关闭标志位
            }
       }
    }
    if(work_state == WORK_STATE_IDLE)
    {
        if(Time_t.temp38 >= 30)  //温度保持30min钟 后切换成38度
        {
            if((ShowPar.shower_state== OFF)&&(ShowPar.tap_state == OFF)) //龙头不再进水时
            {
                ShowPar.temp_val = 380;
                KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;        // 功能码：水龙头出水温度改变
                KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // 温度高
                KeyCmd.req.dat[DAT_TEMP_L]  = ShowPar.temp_val;
            }
            /* BEGIN: Added by zgj, 2018/1/5 */
             Time_t.temp38 = 0;
        }

        /* BEGIN: Deleted by zgj, 2018/1/5 */
        /*
        if(Time_t.temp38 == 40) //40min钟到自动关闭龙头及花洒
        {
            if((ShowPar.shower_state== ON)||(ShowPar.tap_state == ON)) //龙头进水时
            {
                KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;// 功能码：进水开关改变
                ShowPar.tap_state = OFF;
                ShowPar.shower_state = OFF;
                KeyCmd.req.dat[DAT_STATE] = (ShowPar.val&0x07)<<4;
                show_tempture( ShowPar.temp_val);
            }
            Time_t.temp38 = 0;
        }
        */
     }
}

/*****************************************************************************
 函 数 名  : CLEAN_EventHandler
 功能描述  : 清洁功能回调函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月6日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void CLEAN_EventHandler(void)
{
	if(work_state == WORK_STATE_IDLE)
	{
		static uint16 time_clean=0;
		if((time_clean++)>=300)// 3s
		{
			time_clean=0;
			LED_INC_ON;
			LED_WATER_ON;
			work_state = WORK_STATE_CLEAN;
			show_clean();                           //清洁显示---
			KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
			KeyCmd.req.dat[DAT_CLAEN] = 0x01;
            KeyCmd.req.dat[DAT_VALVE] = 0x01;
		}
	}
}

/*****************************************************************************
 函 数 名  : show_work
 功能描述  : 按键状态和温度切换函数
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
void show_work(void)
{
    if( 1 == ShowPar.switch_flg)
    {
         Time_t.switch_cnt ++ ;
         if( Time_t.switch_cnt >= 20 ) // 2s时间到
         {
            Time_t.switch_cnt = 0;
            ShowPar.switch_flg = 0;
            show_tempture( ShowPar.temp_val);
         }
    }
    if(1 == key_switch_fag)
    {
        Time_t.key_adj++;
        if(Time_t.key_adj>=30) // 3s
        {
            Time_t.key_adj = 0;
            key_switch_fag = 0;
            show_tempture( ShowPar.temp_val);
            clear();
            dbg("clear_adj,key_arry[%d]=%x\r\n",top,key_arry[top]);
        }
    }
    if(1 == incdec_fag)
    {
        Time_t.incdec++;
        if(Time_t.incdec>=10)
        {
            incdec_fag = 0;
            Time_t.incdec = 0;
            LED_DEC_OFF;
            LED_INC_OFF;
        }
    }
}
/*****************************************************************************
 函 数 名  : show_temp_actul
 功能描述  : 实际温度和预设温度显示函数
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
void show_temp_actul(void) // 100ms
{
    if(work_state == WORK_STATE_IDLE)
    {
        if((ShowPar.tap_state == ON)||(ShowPar.shower_state == ON )) //出水状态
        {
            uint16 pre_tem;
            pre_tem = KeyCmd.req.dat[DAT_TEM_OUT]*10;
            if((pre_tem < ShowPar.temp_val-20)||((pre_tem > ShowPar.temp_val+20))) //达不到预设温度
            {
                if(Time_t.temp_switch<300)
                {
                    Time_t.temp_switch++;
                }
                if(Time_t.temp_switch >= 200)// 20s
                {
                    static uint8 cnt =0;
                    if((cnt%24)==0)
                    {
                      show_tempture(pre_tem);
                    }
                    if((cnt%24)==10)
                    {
                        show_sleep();
                    }
                    if((cnt%24)==12)
                    {
                      show_tempture( ShowPar.temp_val);
                    }
                    if((cnt%24)==22)
                    {
                        show_sleep();
                    }
                   cnt++;
                   if(cnt>=24)
                   {
                        cnt=0;
                   }
                }
            }
            else
            {
                 Time_t.temp_switch = 0;
            }
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
 函 数 名  : key_state
 功能描述  : 状态更新同步函数
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
void key_state(void)
{
/*
    //水龙头及花洒状态更新
    if(KeyCmd.rsp.dat[DAT_INFO]!=KeyCmd.req.dat[DAT_INFO])
    {
        if((KeyCmd.rsp.dat[DAT_INFO]&0x10)== 0x10) //tap 打开 0010 0000
        {
            ShowPar.tap_state = STATE_ON;
        }
        else
        {
            ShowPar.tap_state = STATE_OFF;
        }
        if((KeyCmd.rsp.dat[DAT_INFO]&0x20) == 0x20) //shower 打开 0001 0000
        {
            ShowPar.shower_state = STATE_ON;
        }
        else
        {
            ShowPar.shower_state = STATE_OFF;
        }
        if((KeyCmd.rsp.dat[DAT_INFO]&0x40)== 0X40) //drain 打开
        {
            ShowPar.drain_state = STATE_ON;
        }
        else
        {
            ShowPar.drain_state = STATE_OFF;
        }
        KeyCmd.req.dat[DAT_INFO] = KeyCmd.rsp.dat[DAT_INFO];
        show_tempture( ShowPar.temp_val);
    }
    */
    // 主机与从机温度不一致，从机需更新温度
    if((KeyCmd.rsp.dat[DAT_TEMP_H]!=KeyCmd.req.dat[DAT_TEMP_H])
        ||(KeyCmd.rsp.dat[DAT_TEMP_L]!=KeyCmd.req.dat[DAT_TEMP_L]))
    {
        uint16 tem = 0;
        tem = (KeyCmd.rsp.dat[DAT_TEMP_H]<<8)+ KeyCmd.rsp.dat[DAT_TEMP_L];
        if((tem >= TEMPERATURE_MIN)&&(tem <= TEMPERATURE_MAX))
        {
            KeyCmd.req.dat[DAT_TEMP_H]=KeyCmd.rsp.dat[DAT_TEMP_H];
            KeyCmd.req.dat[DAT_TEMP_L] =KeyCmd.rsp.dat[DAT_TEMP_L];
            ShowPar.temp_val= (KeyCmd.req.dat[DAT_TEMP_H]<<8)+ KeyCmd.req.dat[DAT_TEMP_L];
            if(work_state == WORK_STATE_IDLE)
            {
                if(lcd_flag == 0) //开启状态才显示出来
                {
                    show_tempture( ShowPar.temp_val);
                    Time_t.sleep = 0; //清零重新等一分钟
                }
            }
        }
    }
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
    memcpy(&KeyCmd.rsp,Recv_Buf,sizeof(KeyCmd.rsp));
    if(work_state != WORK_STATE_LOCK)
    {
        if((Send_Buf[DAT_TEMP_H+3] == KeyCmd.req.dat[DAT_TEMP_H])
            && (Send_Buf[DAT_TEMP_L+3] == KeyCmd.req.dat[DAT_TEMP_L]))  //当前没有按键按下
        {
           key_state();
        }
    }
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    send_dat(&KeyCmd.req, BUF_SIZE);
    memcpy(Send_Buf,&KeyCmd.req,sizeof(KeyCmd.req));
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
    Recv_Buf[Recv_Len] = ui8Data;
    if(Recv_Buf[0]==0x02)
    {
        Recv_Len++;
        if(Recv_Len >= BUF_SIZE) //接收到32byte的数据
        {
            if((Recv_Buf[31]== 0x04)&&(Recv_Buf[30]== 0x0B)&&(Recv_Buf[2] == 0x01)&&(Recv_Buf[1] == 0x3A))
            {
                for(uint8 i=2;i<crc_len;i++)
                {
                    check_sum^=Recv_Buf[i];
                }
                if(check_sum == Recv_Buf[29])
                //if(CRC8_SUM(&Recv_Buf[2], crc_len) == Recv_Buf[29])
                {
                    Recv_Len = 0;
                    frame_ok_fag=1;
                }
                else
                {
                     Recv_Len = 0;
                     frame_ok_fag=0;
                }
            }
           if((Recv_Buf[31]!= 0x04)||(Recv_Buf[30]!= 0x0B)||(Recv_Buf[2] != 0x01)||(Recv_Buf[1] != 0x3A))  //结束码或者地址不对
            {
                 Recv_Len = 0;
                 frame_ok_fag = 0;
            }
        }
    }

}
/*****************************************************************************
 函 数 名  : set_temp_val_dec
 功能描述  : 温度减值函数
 输入参数  : uint8 val
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void set_temp_val_dec(uint8 val)
{
    Time_t.sleep = 0;
    Time_t.temp38 = 0;
    if( ShowPar.temp_val > TEMPERATURE_MIN)
    {
        ShowPar.temp_val -= val ;
        if(ShowPar.temp_val < TEMPERATURE_MIN)
        {
            ShowPar.temp_val = TEMPERATURE_MIN;
        }
    }
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;                              // 功能码：水龙头出水温度改变
    KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // 温度高
    KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // 温度低
    show_tempture( ShowPar.temp_val);
    dbg("temp:%d\r\n",ShowPar.temp_val);
}

/*****************************************************************************
 函 数 名  : set_temp_val_inc
 功能描述  : 温度加值函数
 输入参数  : uint8 val
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2017年11月10日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void set_temp_val_inc(uint8 val)
{
    Time_t.sleep = 0;
    Time_t.temp38 = 0;
    if( ShowPar.temp_val < TEMPERATURE_MAX)
    {
        ShowPar.temp_val += val ;
        if(ShowPar.temp_val > TEMPERATURE_MAX)
        {
            ShowPar.temp_val = TEMPERATURE_MAX;
        }
    }
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;        // 功能码：水龙头出水温度改变
    KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // 温度高
    KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // 温度低
    show_tempture( ShowPar.temp_val);
    dbg("temp:%d\r\n",ShowPar.temp_val);
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
    ShowPar.lamp_gear = LAMP_PINK;
    ShowPar.water_gear = MASSAGE_GEAR_ON3;
    ShowPar.air_gear = MASSAGE_GEAR_ON3;
    /* END:   Added by zgj, 2018/1/5 */
    KeyCmd.req.sta_num1 = 0x02;
    KeyCmd.req.sta_num2  = 0xA3;
    ShowPar.temp_val = 380;
    KeyCmd.req.dat[DAT_ADDR]  = 0x01;
    KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // 温度高
    KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
    KeyCmd.req.dat[DAT_FLOW] = 0x64;
    KeyCmd.req.dat[DAT_TEM_PRE]  = 0x26;
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    KeyCmd.req.end_num1 = 0x0F;
    KeyCmd.req.end_num2 = 0x04;
    memcpy(Send_Buf,&KeyCmd.req,sizeof(KeyCmd.req));
    show_tempture( ShowPar.temp_val);
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


