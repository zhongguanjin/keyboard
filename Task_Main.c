
#include "Task_Main.h"
#include "uart.h"
#include "stdio.h"
#include <string.h>
#include "dbg.h"
#include "eeprom.h"

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
     uint16 err_cnt;
}tTime_T;

tTime_T Time_t;

typedef union {
      unsigned int word;
      unsigned char byte[2];
} wordbyte;

volatile uint8  key_adjust_fag;          //按键调节切换标志
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
static uint16 Cstate_time =0;


//zgj 2018-07-12  ota update
#define   soft_version      0x01  //软件版本号
UN32      soft_chksum;         // 软件校验码，4字节
uint8 chksum=0;
uint16 block=0;
uint8 verify=0;  //
void get_hex_file(void);
uint8 update_flg=0;
uint8 bak_ok_flg = 0;
wordbyte addr;
static UN16 index=0;         //
//end
uint8 f6_err_cnt=0;   //zgj 2018-7-26 通信故障

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
void TaskClean(void);
void WIFI_EventHandler(void);

void judge_err_num(void);
void check_uart(void);
void clear_fun_show(void);

//2018-08-15 zgj
void child_lock_show(void);
void wifi_pair_pro(void);
void sync_temp_show(void);

void key_massage_sync(void);
void key_lamp_sync(void);
void key_inflow_sync(void);
void key_temp_sync( void );
void key_state_sync(void);


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



void clear_f6_cnt(void)
{
    f6_err_cnt = 0;
    if(Flg.err_f6_flg==1)  //出现f6时
    {
        Flg.err_f6_flg=0;
        show_tempture(ShowPar.temp_val);
        dbg("f6 err->temp\r\n");
    }
}

void clear_fun_show(void)
{
   static uint8 cnt =0,flash_cnt=0;
   if(Flg.clean_err_flg == 0)
   {
        cnt=0;
        flash_cnt=0;
        TaskClean();
   }
   else
   {
        if((cnt%10)==5)
        {
           show_sleep(OFF);
        }
        if((cnt%10)==9)
        {
            show_clean();
            flash_cnt++;
        }
        if((cnt++)>=100)
        {
            cnt=0;
        }
        if(flash_cnt>=3)
        {
           flash_cnt=0;
           show_tempture(ShowPar.temp_val);
           dbg("clean flash->idle\r\n");
           work_state =  WORK_STATE_IDLE;
        }
   }
}
/*****************************************************************************
 函 数 名  : TaskShow
 功能描述  : 显示任务
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
    if(work_state != WORK_MCU_UPDATE)
    {
        if((f6_err_cnt++)>=100)//10s
        {
            f6_err_cnt = 70;
            Flg.err_f6_flg=1;
            write_err_num(ERR_F6);
        }
    	check_uart();
    }
    switch(work_state)
    {
        case WORK_STATE_IDLE:
        {
            show_work();
            show_temp_flash();
            if((key_adjust_fag==0)&&(ShowPar.on_off_flg==0)
                &&(incdec_fag == 0)&&(Flg.err_flg!=1))           //显示处于空闲时
            {
                show_temp_actul();
            }
            break;
        }
        case WORK_STATE_LOCK:
        {
            child_lock_show();
            break;
        }
        case WORK_STATE_CLEAN:
        {
            clear_fun_show();
            break;
        }
        case WORK_WIFI_PAIR:
        {
            wifi_pair_pro();
            break;
        }
        default:
        {
            break;
        }
    }
}

/*****************************************************************************
 函 数 名  : TaskClean
 功能描述  : 清洁管道任务函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月25日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void TaskClean() //100ms
{
    switch ( clean_state )
    {
        case STATE_1:
            {
                if((Cstate_time++)==10)  // 1s 后发送
                {
                    Cstate_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x01;
                    clean_state = STATE_2;
                }
            }
            break;
        case STATE_2 :
            {
                if((Cstate_time++)==600)// 1min
                {
                    Cstate_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x02;
                    clean_state = STATE_3;
                }
            }
            break;
        case STATE_3 :
            {
                //if((KeyCmd.req.dat[DAT_LIQUID]&0x01)!=0x01) //低于低液位
                {
                    if((Cstate_time++)==3600)  //6min
                    {
                        Cstate_time = 0;
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                        KeyCmd.req.dat[DAT_VALVE] = 0x03;
                        clean_state = STATE_4;
                    }
                }
            }
            break;
        case STATE_4 :
            {
                //if((KeyCmd.req.dat[DAT_LIQUID]&0x01)==0x01) //高于低液位
                {
                    if((Cstate_time++)==600)  // 1min
                    {
                        Cstate_time = 0;
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                        KeyCmd.req.dat[DAT_VALVE] = 0x04;
                        clean_state = STATE_5;
                    }
                }
            }
            break;
        case STATE_5 :
            {
                if((Cstate_time++)==3600)// 6min
                {
                    Cstate_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x05;
                    clean_state = STATE_6;
                }
            }
             break;
        case STATE_6 :
            {
                if((Cstate_time++)==600)// 1min
                {
                    Cstate_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x06;
                    clean_state = STATE_7;
                }
            }
             break;
        case STATE_7 :
            {
                if((Cstate_time++)==3600)// 6min
                {
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x07;
                }
                if(Cstate_time ==3620) // 6min2s后
                {
                    Cstate_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //功能码
                    KeyCmd.req.dat[DAT_VALVE] =0x00;
                    show_tempture(ShowPar.temp_val);
                    dbg(" clean ok->idle\r\n");
                    clean_state = STATE_0;
                    work_state =WORK_STATE_IDLE;
                }
            }
            break;
         default:
            break;
    }

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
#if key_5
    id =  Button_id&0X1f;
#else
    id =  Button_id&0Xff; //ff
#endif
    if(work_state !=WORK_MCU_UPDATE)
    {
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
                //if((KeyCmd.req.dat[DAT_LIQUID]&0x01) == 0x01) //低液位
    		    {
    			    CLEAN_EventHandler();
    			}
                break;
    		}
            case LOCK_VALVE:
            {
                if((count++)>=300)
                {
                    count = 0;
                    LOCK_EventHandler();
                }
               break;
            }
            case WIFI_VALVE:
            {
                if((count++)>=300)
                {
                    count = 0;
                    WIFI_EventHandler();
                }
               break;
            }
            default:
            {
                count = 0;
                Flg.lock_flg =0;
                time_clean = 0;
                break;
            }
        }
        if(work_state !=WORK_WIFI_PAIR)
        {
            judge_err_num();
        }
        IDLE_EventHandler();
    }
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
    key_adjust_fag =1;
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
    Time_t.err_cnt=0;
    if((id==TAP_VALVE)||(id==SHOWER_VALVE))//龙头或花洒
    {
        Time_t.switch_cnt = 0;                  //on /off 间隔时间要清0
        Time_t.temp_switch =0;                  //温度切换时间变量
        Flg.temp_disreach_flg =0;               //快闪和慢闪标志清0
        key_adjust_fag=0;
    }
    if((id==INC_VALVE)||(id==DEC_VALVE))    //+,-
    {
        if(key_adjust_fag==0) //调节模式不要清除temp_switch的时间
        {
            Time_t.temp_switch =0;
            Flg.temp_disreach_flg =0;
        }
        Time_t.incdec = 0;
        Time_t.key_adj = 0;
        incdec_fag =1;
    }
    if((id==AIR_VALVE)||(id==WATER_VALVE)||(id==LAMP_VALVE))//按摩，灯光键
    {
         Time_t.key_adj = 0;
         Time_t.switch_cnt = 0;
         ShowPar.on_off_flg = 0;  //清on
    }
    if(id==DRAIN_VALVE)
    {
       Time_t.switch_cnt =0;
    }
}

/*****************************************************************************
 函 数 名  : show_temp_flash
 功能描述  : 温度最大或最小闪烁函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年1月17日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void show_temp_flash(void)//100ms
{
    if(Flg.temp_flash_flg == 1)
    {
        static uint8 cnt =0;
        if((cnt%10)==5)
        {
           show_sleep(OFF);
        }
        if((cnt%10)==9)
        {
            show_tempture(ShowPar.temp_val);
            dbg("temp max min flash\r\n");
            flash_cnt++;
        }
        cnt++;
        if(flash_cnt >= 3)// 3次
        {
            flash_cnt = 0;
            cnt=0;
            Flg.temp_flash_flg = 0;
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
    if((work_state == WORK_STATE_IDLE)&&(Flg.err_f1_flg == 0)) //没有F1
    {
        if(KeyPressDown&TAP_VALVE) //第一次按下
         {
            KeyPressDown = 0;
            if(Flg.lcd_sleep_flg == 1)
            {
                show_awaken();
                dbg("wake lcd\r\n");
                return;
            }
            ShowPar.tap_state ^= 0x01;
            if(ShowPar.tap_state == STATE_ON)
            {
                ShowPar.shower_state = STATE_OFF;
                LED_TAP_ON;
                LED_SHOWER_OFF;
            }
            else
            {
               LED_TAP_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;  // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //数据码 龙头状态
            time_cnt_del(TAP_VALVE);
            show_state(ShowPar.tap_state);
            dbg("tap,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
    //else if((Flg.err_f1_flg == 1)&&(KeyCmd.req.dat[DAT_ERR_NUM]&0x80))  //f1 err 时
    else if(Flg.err_f1_flg == 1)  //f1 err 时
    {
         if(KeyPressDown&TAP_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            time_cnt_del(TAP_VALVE);
            ShowPar.tap_state = STATE_ON;
            LED_TAP_ON;
            Flg.err_f1_flg =0;
            ShowPar.temp_val = 380;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.val&0x03;
            KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // 温度高
            KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
            show_tempture(ShowPar.temp_val);
             dbg("f1 err inflow\r\n");
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
    if((work_state == WORK_STATE_IDLE)&&(Flg.err_f1_flg == 0))
    {
        if(KeyPressDown&SHOWER_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            if(Flg.lcd_sleep_flg == 1)
            {
                show_awaken();
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
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //数据码 花洒
            time_cnt_del(SHOWER_VALVE);
            show_state(ShowPar.shower_state);
            dbg("shower,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
    //else if((Flg.err_f1_flg == 1)&&(KeyCmd.req.dat[DAT_ERR_NUM]&0x80))  //f1 err 时
    else if(Flg.err_f1_flg == 1)  //f1 err 时
    {
        if(KeyPressDown&SHOWER_VALVE)  //第一次触发
        {
            KeyPressDown = 0;
            time_cnt_del(SHOWER_VALVE);
            ShowPar.shower_state = STATE_ON;
            LED_SHOWER_ON;
            Flg.err_f1_flg =0;
            ShowPar.temp_val = 380;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.val&0x03;
            KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // 温度高
            KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
            show_tempture(ShowPar.temp_val);
            dbg("f1 err inflow\r\n");
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
            if(Flg.lcd_sleep_flg == 1)
            {
                show_awaken();
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
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //数据码 排水
            time_cnt_del(DRAIN_VALVE);
            show_state(ShowPar.drain_state);
            dbg("drainage,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
        if(Flg.lcd_sleep_flg == 1)
        {
            show_awaken();
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
                        /* BEGIN: Added by zgj, 2018/1/5 */
                        ShowPar.lamp_gear++;
                        if(ShowPar.lamp_gear >LAMP_CYCLE)
                        {
                          ShowPar.lamp_gear = LAMP_RED;
                        }
                        /* END:   Added by zgj, 2018/1/5 */
                        key_adjust(key_arry[top], ShowPar.lamp_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // 功能码:08
                        KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_gear;
                        ShowPar.light_state=ShowPar.lamp_gear;
                        dbg("lamp %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
                        key_adjust(key_arry[top],ShowPar.water_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:05
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5); //数据码
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
                        key_adjust(key_arry[top],ShowPar.air_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:07
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5); //数据码
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
        if(Flg.lcd_sleep_flg == 1)//休眠唤醒
        {
            show_awaken();
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
                       /* BEGIN: Added by zgj, 2018/1/5 */
                        ShowPar.lamp_gear--;
                        if(ShowPar.lamp_gear < LAMP_RED)
                        {
                            ShowPar.lamp_gear =LAMP_CYCLE;
                        }
                       /* END:   Added by zgj, 2018/1/5 */
                       key_adjust(key_arry[top],ShowPar.lamp_gear);
                       KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // 功能码:06
                       KeyCmd.req.dat[DAT_VALVE]=ShowPar.lamp_gear;
                       ShowPar.light_state=ShowPar.lamp_gear;
                       dbg("lamp %x\r\n", KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case WATER_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        if(ShowPar.water_gear > MASSAGE_GEAR_ON1)
                        {
                            ShowPar.water_gear--;
                        }
                        else
                        {
                            ShowPar.water_gear = MASSAGE_GEAR_ON1;
                        }
                        key_adjust(key_arry[top],ShowPar.water_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:07
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case AIR_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //第一次触发
                    {
                        KeyPressDown = 0;
                        if(ShowPar.air_gear > MASSAGE_GEAR_ON1)
                        {
                            ShowPar.air_gear--;
                        }
                        else
                        {
                            ShowPar.air_gear = MASSAGE_GEAR_ON1;
                        }
                        key_adjust(key_arry[top],ShowPar.air_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // 功能码:07
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
            if(Flg.lcd_sleep_flg ==1)
            {
                show_awaken();
                return ;
            }
            ShowPar.air_state ^= 0x01;
            if( ShowPar.air_state == STATE_ON)
            {
                LED_AIR_ON;
                add(AIR_VALVE);
                KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                        +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                key_adjust(key_arry[top],ShowPar.air_gear);
            }
            else
            {
               LED_AIR_OFF;
               del(AIR_VALVE);
               KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                        +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
               if(key_arry[top]==0)  //为空
               {
                   Time_t.key_adj = 0;
                   key_adjust_fag = 0;
               }
               show_tempture(ShowPar.temp_val);
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // 功能码：07
            dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
            if(Flg.lcd_sleep_flg ==1)
            {
                show_awaken();
                return ;
            }
            ShowPar.water_state ^= 0x01;
            if( ShowPar.water_state == STATE_ON)
            {
                LED_WATER_ON;
                add(WATER_VALVE);
                KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                        +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                key_adjust(key_arry[top],ShowPar.water_gear);
            }
            else
            {
               LED_WATER_OFF;
               del(WATER_VALVE);
               KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                        +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                if(key_arry[top]==0)  //为空
                {
                    Time_t.key_adj = 0;
                    key_adjust_fag = 0;
                }
                show_tempture(ShowPar.temp_val);
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // 功能码：07
            dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
	if(work_state == WORK_STATE_CLEAN)
	{
        if((LastKey&WATER_VALVE)&&((time_clean++)>=200))// 2s
        {
            KeyPressDown = 0;
            Flg.clean_err_flg = 0;
    		work_state =WORK_STATE_IDLE;
    		KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //功能码
    		KeyCmd.req.dat[DAT_VALVE] =0x00;
            Cstate_time = 0;
    		show_tempture(ShowPar.temp_val);
            clean_state = STATE_0;
    		dbg("clean cancel\r\n");
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
            time_cnt_del(LAMP_VALVE);
            if(Flg.lcd_sleep_flg ==1)
            {
                show_awaken();
                return ;
            }
            ShowPar.lamp_state ^= 0x01;
            if(ShowPar.lamp_state == STATE_ON) //打开灯，接着判断有无+ -按键
            {
               LED_LAMP_ON;
               KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_gear;
               ShowPar.light_state = ShowPar.lamp_gear;
               add(LAMP_VALVE);
               key_adjust(key_arry[top],ShowPar.lamp_gear);
            }
            else  //关灯off
            {
                LED_LAMP_OFF;
                KeyCmd.req.dat[DAT_VALVE] = LAMP_OFF;
                ShowPar.light_state = LAMP_OFF;
               del(LAMP_VALVE);
               if(key_arry[top]==0)  //为空
               {
                   Time_t.key_adj = 0;
                   key_adjust_fag = 0;
               }
               show_tempture(ShowPar.temp_val);
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // 功能码:06
            dbg("lamp %x\r\n", KeyCmd.req.dat[DAT_VALVE]);
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
    if((work_state == WORK_STATE_IDLE)&&(Flg.lock_flg ==0))
    {
        Flg.lock_flg =1;   //检测按键松开 0-松开，1-按下
        work_state = WORK_STATE_LOCK;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // 功能码：进水开关改变
        KeyCmd.req.dat[DAT_VALVE] = 0x01;
        dbg("idle -> lock,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
    }
    if((work_state == WORK_STATE_LOCK)&&(Flg.lock_flg ==0))
    {
       Flg.lock_flg =1;
       show_awaken();
       work_state = WORK_STATE_IDLE;
       KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // 功能码：进水开关改变
       KeyCmd.req.dat[DAT_VALVE] = 0x00;
       dbg("lock -> idle\r\n");
    }
}

void WIFI_EventHandler(void) //10ms
{
    if(work_state == WORK_STATE_IDLE)
    {
        work_state =WORK_WIFI_PAIR;
        show_wifi_pair(10,10,10);
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_WIFI;            // 功能码：wifi pair
        KeyCmd.req.dat[DAT_VALVE] = 0x01;
		KeyCmd.req.dat[DAT_WIFI_PAIR] = 0x00;
        dbg("idle -> wifi pair\r\n");
    }
}

/*****************************************************************************
 函 数 名  : judge_err_num
 功能描述  : 故障处理部分
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年4月20日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void judge_err_num(void)//10ms
{
    if((KeyCmd.req.dat[DAT_ERR_NUM]&0x7F) != 0x00)  //有错误
    {
        if((key_adjust_fag==0)&&(ShowPar.on_off_flg==0)&&(incdec_fag == 0))
        {
            Flg.err_flg =1;
            if((Time_t.err_cnt++)>=400)
            {
                Time_t.err_cnt=0;
                write_err_num(KeyCmd.req.dat[DAT_ERR_NUM]&0x7F);
            }
            if((KeyCmd.req.dat[DAT_ERR_NUM]&0x01) == 0x01)//f1错误
            {
                Flg.err_f1_flg =1;
            }
            work_state = WORK_STATE_IDLE;
        }
    }
    else //无错误
    {
        if(Flg.err_flg ==1) //
        {
            Time_t.err_cnt=0;
            Flg.err_flg =0;
            Flg.err_f1_flg =0;
            show_tempture(ShowPar.temp_val);
            dbg("err->idle\r\n");
            work_state = WORK_STATE_IDLE;
        }
    }
}

/*****************************************************************************
 函 数 名  : wifi_pair_pro
 功能描述  : wifi 配对处理
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void wifi_pair_pro(void)
{
    if((Time_t.wifi_pair++)==10) // 1s后发送关闭按钮
    {
        if(ShowPar.tap_state==ON)
        {
            ShowPar.tap_state=OFF;
            LED_TAP_OFF;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;  // 功能码：进水开关改变
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //数据码 龙头状态
        }
        else
        {
            ShowPar.drain_state=OFF;
            LED_DRAIN_OFF;
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_DRAINAGE;
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //数据码 排水
        }
    }
    if(Time_t.wifi_pair>=600)
    {
        Time_t.wifi_pair =0;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_WIFI;            // 功能码：wifi pair
        KeyCmd.req.dat[DAT_VALVE] = 0x00;
        work_state = WORK_STATE_IDLE;
        show_tempture(ShowPar.temp_val);
        dbg("wifi pair over time\r\n");
    }
}

/*****************************************************************************
 函 数 名  : child_lock_show
 功能描述  : 儿童保护显示
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void child_lock_show(void)
{
    static uint8 lock_cnt=0;
    if((lock_cnt%10)==4)//400ms
    {
        show_lock();
    }
    if((lock_cnt%10)==9)//900ms
    {
       show_sleep(OFF);
    }
    if((lock_cnt++)>9)
    {
        lock_cnt=0;
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
    static uint8 flg=0;
    if((Time_t.temp38 == 30)&&(work_state == WORK_STATE_IDLE))  //温度保持30min钟 后切换成38度
    {
        if(flg==0)
        {
            flg=1;
            ShowPar.temp_val = 380;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;        // 功能码：水龙头出水温度改变
            KeyCmd.req.dat[DAT_VALVE]=0x00;
            KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // 温度高
            KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // 温度低
            KeyCmd.req.dat[25]=0x01;  //告诉温控板复位
            dbg("30min temp->38\r\n");
            Time_t.temp38 = 0;
        }
    }
    if(Time_t.sleep++ >=6000) // 1min钟时间到  时基10ms
    {
       Time_t.sleep = 0;
       if(Flg.lcd_sleep_flg == 1) //面板休眠
       {
            Time_t.temp38++;
       }
       else
       {
            Time_t.temp38 = 0;
            flg=0;
       }
       if(work_state == WORK_STATE_IDLE)
       {
            dbg("1 min get\r\n");
            if((ShowPar.val&0xff)== OFF)
            {
                dbg("sleep\r\n");
                show_sleep(ON);
            }
       }
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
		if((LastKey&CLEAN_VALVE)&&((time_clean++)>=300))// 3s
		{
            time_clean=0;
            work_state = WORK_STATE_CLEAN;
		    if((KeyCmd.req.dat[DAT_LIQUID]&0x01) == 0x01) //低液位
		    {
		        Flg.clean_err_flg = 0;
    			show_clean();                           //清洁显示---
    			clean_state =STATE_1;
    	    }
    	    else
    	    {
                Flg.clean_err_flg = 1;
    	    }
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
    if( 1 == ShowPar.on_off_flg)
    {
         Time_t.switch_cnt ++ ;
         if( Time_t.switch_cnt >= 20 ) // 2s时间到
         {
            Time_t.switch_cnt = 0;
            ShowPar.on_off_flg = 0;
            //if(work_state == WORK_STATE_IDLE)
            {
                show_tempture(ShowPar.temp_val);
            }
            dbg("on off->temp\r\n");
         }
    }
    if(1 == key_adjust_fag)
    {
        Time_t.key_adj++;
        if(Time_t.key_adj>=50) // 5s
        {
            Time_t.key_adj = 0;
            key_adjust_fag = 0;
            //if(work_state == WORK_STATE_IDLE)
            {
                show_tempture(ShowPar.temp_val);
            }
            clear();
            dbg("clear_adj,key_arry[%d]=%x\r\n",top,key_arry[top]);
        }
    }
    if(1 == incdec_fag)
    {
        Time_t.incdec++;
        if(Time_t.incdec>=10) // 1s
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
    uint16 pre_tem=0;
    static uint8 cnt1 =0,cnt=0,show_flg=0;
    if((ShowPar.tap_state == ON)||(ShowPar.shower_state == ON )) //出水状态
    {
        pre_tem = KeyCmd.req.dat[DAT_TEM_OUT]*10;
        if((pre_tem < ShowPar.temp_val-20)||((pre_tem > ShowPar.temp_val+20))) //达不到预设温度
        {
            show_flg=0;
            if(Time_t.temp_switch<1000)
            {
                Time_t.temp_switch++;
            }
            if((Time_t.temp_switch >= 200)&&(Flg.temp_disreach_flg == 0))// 20s
            {
                if((cnt%24)==0)
                {
                    show_tempture(pre_tem);
                }
                if((cnt%24)==10)
                {
                    show_sleep(OFF);
                }
                if((cnt%24)==12)
                {
                    show_tempture( ShowPar.temp_val);
                }
                if((cnt%24)==22)
                {
                    show_sleep(OFF);
                }
                cnt++;
                if(cnt>=24)
                {
                    cnt=0;
                }
            }
            if(Time_t.temp_switch >= 800)// 120s
            {
                Flg.temp_disreach_flg =1;
                if((cnt1%45)==0)
                {
                    show_tempture(pre_tem);
                }
                if((cnt1%45)==20)
                {
                    show_sleep(OFF);
                }
                if((cnt1%45)==22)
                {
                    show_tempture( ShowPar.temp_val);
                }
                if((cnt1%45)==42)
                {
                    show_sleep(OFF);
                }
                cnt1++;
                if(cnt1>=45)
                {
                    cnt1=0;
                }
            }
        }
        else if(show_flg==0)
        {
            cnt=0;
            cnt1=0;
            show_flg=1;
            Flg.temp_disreach_flg =0;
            Time_t.temp_switch = 0;
            show_tempture( ShowPar.temp_val);
            dbg("2 temp ok\r\n");
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
 函 数 名  : sync_temp_show
 功能描述  : 同步状态温度显示
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void sync_temp_show(void)
{
    if((key_adjust_fag==0)&&(ShowPar.on_off_flg==0)
        &&(incdec_fag == 0)&&(Flg.err_flg!=1))           //显示处于空闲时
    {
        show_tempture(ShowPar.temp_val);
        dbg("chk\r\n");
    }
}

/*****************************************************************************
 函 数 名  : key_massage_sync
 功能描述  : 按摩状态同步
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void key_massage_sync(void)
{
    /* BEGIN: Added by zgj, 2018/1/15 */
    uint8 massage_dat = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
    if(((KeyCmd.rsp.dat[DAT_MASSAGE]&0x1D)!=(KeyCmd.req.dat[DAT_MASSAGE]&0x1D))
        ||((KeyCmd.rsp.dat[DAT_MASSAGE]&0x1D)!=(massage_dat&0x1D))) //水按摩状态更新
    {
        time_cnt_del(WATER_VALVE);
        ShowPar.water_gear = (KeyCmd.rsp.dat[DAT_MASSAGE]&0x1C)>>2;
        KeyCmd.req.dat[DAT_MASSAGE] =KeyCmd.rsp.dat[DAT_MASSAGE];
        if((KeyCmd.req.dat[DAT_MASSAGE]&0x01)==0x01) //水按摩开启
        {
            ShowPar.water_state =ON;
            LED_WATER_ON;
            if(work_state == WORK_STATE_IDLE)
            {
                add(WATER_VALVE);
                key_adjust(key_arry[top],ShowPar.water_gear);
            }
        }
        else if((KeyCmd.req.dat[DAT_MASSAGE]&0x01)==0) //水按摩关闭
        {
            ShowPar.water_state =OFF;
            LED_WATER_OFF;
            del(WATER_VALVE);
            if((key_arry[top]==0)&&(work_state == WORK_STATE_IDLE))  //为空
            {
                Time_t.key_adj = 0;
                key_adjust_fag = 0;
                sync_temp_show();
                dbg("sync water off\r\n");
            }
        }
    }
    if(((KeyCmd.rsp.dat[DAT_MASSAGE]&0xE2)!=(KeyCmd.req.dat[DAT_MASSAGE]&0xE2)
        ||((KeyCmd.rsp.dat[DAT_MASSAGE]&0xE2)!=(massage_dat&0xE2)))) //气按摩状态更新
    {
        time_cnt_del(AIR_VALVE);
        ShowPar.air_gear = (KeyCmd.rsp.dat[DAT_MASSAGE]&0xE0)>>5;
        KeyCmd.req.dat[DAT_MASSAGE] =KeyCmd.rsp.dat[DAT_MASSAGE];
        if((KeyCmd.req.dat[DAT_MASSAGE]&0x02)==0x02) //气按摩开启
        {
            ShowPar.air_state =ON;
            LED_AIR_ON;
            if(work_state == WORK_STATE_IDLE)
            {
                add(AIR_VALVE);
                key_adjust(key_arry[top],ShowPar.air_gear);
            }
        }
        else if((KeyCmd.req.dat[DAT_MASSAGE]&0x02)==0) //气按摩关闭
        {
            ShowPar.air_state =OFF;
            LED_AIR_OFF;
            del(AIR_VALVE);
            if((key_arry[top]==0)&&(work_state == WORK_STATE_IDLE))  //为空
            {
                Time_t.key_adj = 0;
                key_adjust_fag = 0;
                sync_temp_show();
                dbg("sync air off\r\n");
            }
        }
    }
}

/*****************************************************************************
 函 数 名  : key_lamp_sync
 功能描述  : 灯光状态同步
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void key_lamp_sync(void)
{
    if((KeyCmd.rsp.dat[DAT_LIGHT]!=KeyCmd.req.dat[DAT_LIGHT])
        ||(KeyCmd.rsp.dat[DAT_LIGHT]!=ShowPar.light_state))
    {
        KeyCmd.req.dat[DAT_LIGHT] =KeyCmd.rsp.dat[DAT_LIGHT];
        time_cnt_del(LAMP_VALVE);
       if(KeyCmd.req.dat[DAT_LIGHT] == 0)
       {
            ShowPar.lamp_state = OFF;
            ShowPar.light_state = OFF;
            LED_LAMP_OFF;
            del(LAMP_VALVE);
            if((key_arry[top]==0)&&(work_state == WORK_STATE_IDLE))  //为空
            {
                Time_t.key_adj = 0;
                key_adjust_fag = 0;
                sync_temp_show();
                dbg("sync lamp off\r\n");
            }
       }
       else
       {
           ShowPar.lamp_state = ON;
           LED_LAMP_ON;
           ShowPar.lamp_gear =KeyCmd.rsp.dat[DAT_LIGHT];
            ShowPar.light_state = ShowPar.lamp_gear;
           if(work_state == WORK_STATE_IDLE)
           {
               add(LAMP_VALVE);
               key_adjust(key_arry[top],ShowPar.lamp_gear);
           }
       }
    }
}

/*****************************************************************************
 函 数 名  : key_inflow_sync
 功能描述  : 龙头花洒状态同步
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void key_inflow_sync(void)
{
    if((KeyCmd.rsp.dat[DAT_STATE]!=KeyCmd.req.dat[DAT_STATE])
        ||(KeyCmd.rsp.dat[DAT_STATE]!=(ShowPar.val&0x03))) //出水状态更新
    {
        KeyCmd.req.dat[DAT_STATE] =KeyCmd.rsp.dat[DAT_STATE];
        time_cnt_del(TAP_VALVE);
        switch ( KeyCmd.req.dat[DAT_STATE]&0x03)
        {
            case 0 :
                {
                    ShowPar.tap_state = OFF;
                    ShowPar.shower_state =OFF;
                    LED_TAP_OFF;
                    LED_SHOWER_OFF;
                }
                break;
            case 1 :
                {
                    ShowPar.tap_state = ON;
                    ShowPar.shower_state =OFF;
                    LED_TAP_ON;
                    LED_SHOWER_OFF;
                }
                break;
            case 2 :
                {
                    ShowPar.tap_state = OFF;
                    ShowPar.shower_state =ON;
                    LED_TAP_OFF;
                    LED_SHOWER_ON;
                }
                break;
        }
    }
}
/*****************************************************************************
 函 数 名  : key_drain_sync
 功能描述  : 下水器状态同步
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年8月15日 星期三
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void key_drain_sync(void)
{
    if((KeyCmd.rsp.dat[DAT_DRAIN]!=KeyCmd.req.dat[DAT_DRAIN])
        ||(KeyCmd.rsp.dat[DAT_DRAIN]!=ShowPar.drain_state)) //下水器状态更新
    {
         time_cnt_del(DRAIN_VALVE);
         KeyCmd.req.dat[DAT_DRAIN] =KeyCmd.rsp.dat[DAT_DRAIN];
        if(KeyCmd.req.dat[DAT_DRAIN] == 0)
        {
             ShowPar.drain_state = OFF;
             LED_DRAIN_OFF;
        }
        else
        {
            ShowPar.drain_state = ON;
            LED_DRAIN_ON;
        }
    }
}
/*****************************************************************************
 函 数 名  : key_state_sync
 功能描述  : 其他状态更新同步函数
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
void key_state_sync(void)
{
    if(KeyCmd.rsp.dat[DAT_LOCK]!=KeyCmd.req.dat[DAT_LOCK]) //童锁状态更新
    {
        Time_t.sleep=0;
        KeyCmd.req.dat[DAT_LOCK] =KeyCmd.rsp.dat[DAT_LOCK];
        if(KeyCmd.req.dat[DAT_LOCK] == 1)                // lock
        {
            show_lock();
            work_state =WORK_STATE_LOCK;
        }
        else
        {
            sync_temp_show();
            dbg("sync lock->idle\r\n");
            work_state =WORK_STATE_IDLE;
        }
    }
    if(KeyCmd.rsp.dat[DAT_CLAEN]!=KeyCmd.req.dat[DAT_CLAEN]) //清洁状态更新
    {
       Time_t.sleep=0;
       KeyCmd.req.dat[DAT_CLAEN] =KeyCmd.rsp.dat[DAT_CLAEN];
       if(KeyCmd.req.dat[DAT_CLAEN] == 1)                // clean
       {
           show_clean();
           work_state =WORK_STATE_CLEAN;
       }
       else
       {
           sync_temp_show();
           dbg("sync clear->idle\r\n");
           work_state =WORK_STATE_IDLE;
       }
    }
    if(KeyCmd.rsp.dat[DAT_WIFI_PAIR]!=KeyCmd.req.dat[DAT_WIFI_PAIR]) // WIFI_PAIR NUM 更新
    {
        Time_t.sleep=0;
        KeyCmd.req.dat[DAT_WIFI_PAIR] = KeyCmd.rsp.dat[DAT_WIFI_PAIR];
        uint8 dat =KeyCmd.req.dat[DAT_WIFI_PAIR];
        if(KeyCmd.req.dat[DAT_WIFI_PAIR]!=0)
        {
            show_wifi_pair(dat/100, (dat%100)/10, dat%10);
        }
        else
        {
           Time_t.wifi_pair =0;
           work_state = WORK_STATE_IDLE;
           dbg("sync pair->idle\r\n");
           sync_temp_show();
        }
    }
}
/*****************************************************************************
 函 数 名  : key_temp_sync
 功能描述  : 温度同步处理
 输入参数  : void
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2018年4月19日
    作    者   : zgj
    修改内容   : 新生成函数

*****************************************************************************/
void key_temp_sync( void )
{
    if(work_state == WORK_STATE_IDLE)
    {
        if((KeyCmd.rsp.dat[DAT_TEMP_H]!=KeyCmd.req.dat[DAT_TEMP_H])
            ||(KeyCmd.rsp.dat[DAT_TEMP_L]!=KeyCmd.req.dat[DAT_TEMP_L]))   //温度更新
        {
            uint16 tem = 0;
            tem = (KeyCmd.rsp.dat[DAT_TEMP_H]<<8)+ KeyCmd.rsp.dat[DAT_TEMP_L];
            if((tem >= TEMPERATURE_MIN)&&(tem <= TEMPERATURE_MAX))
            {
                KeyCmd.req.dat[DAT_TEMP_H]=KeyCmd.rsp.dat[DAT_TEMP_H];
                KeyCmd.req.dat[DAT_TEMP_L] =KeyCmd.rsp.dat[DAT_TEMP_L];
                ShowPar.temp_val= (KeyCmd.req.dat[DAT_TEMP_H]<<8)+ KeyCmd.req.dat[DAT_TEMP_L];
                if(Flg.lcd_sleep_flg == 0)
                {
                    sync_temp_show();
                    dbg("sync temp\r\n");
                    Time_t.sleep = 0; //清零重新等一分钟
                }
            }
        }
    }
}


void get_hex_file(void)
{
    static uint8 config=0;
    unsigned char rectype,len;
    wordbyte data;
    wordbyte addrbak;
    uint8 num=8;
    UN16 index_bak;         //
    index_bak.uch[1]= KeyCmd.rsp.dat[2];//get the  high byte
    index_bak.uch[0]= KeyCmd.rsp.dat[3];//索引号
    if(index.ush != index_bak.ush)
    {
        KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
        KeyCmd.req.dat[DAT_VALVE] =0x01;
        KeyCmd.req.dat[3]=index.uch[1];
        KeyCmd.req.dat[4]=index.uch[0];//索引号
        return ;
    }
    index.uch[1]= KeyCmd.rsp.dat[2];//get the  high byte
    index.uch[0]= KeyCmd.rsp.dat[3];//索引号
    len = KeyCmd.rsp.dat[4]>>1;    //长度/2
    rectype =KeyCmd.rsp.dat[7];   //记录类型
    chksum=0;
    for(uint8 i=4;i<=24;i++)
    {
        chksum = chksum+ KeyCmd.rsp.dat[i];
    }
    switch(rectype)
    {
        case 0: //数据
        {
            if(config == 1)
            {
                index.ush++;
                KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
                KeyCmd.req.dat[DAT_VALVE] =0x02;
                KeyCmd.req.dat[3]=index.uch[1];
                KeyCmd.req.dat[4]=index.uch[0];//索引号
                break;
            }
            addr.byte[1]=KeyCmd.rsp.dat[5];//get the addr high byte
            addr.byte[0]=KeyCmd.rsp.dat[6];//get the addr low byte
            addr.word>>=1;
            addrbak.word=addr.word-APP_START+APP_BAK; //备份地址
            verify =ok;
            if(chksum == 0)
            {
                if(block != (addrbak.word/0x20))
                {
                    block = addrbak.word/0x20;
                    nv_erase(block*0x20); // erase 32 word
                }
                while(len!=0)
                {
                    data.byte[0] = KeyCmd.rsp.dat[num++];   // get the data low byte
                    data.byte[1] = KeyCmd.rsp.dat[num++];  // get the data high byte
                    nv_write(1,addrbak.word,data.word);
                    if(data.word!=nv_read(1,addrbak.word)) //写一个字ok?
                    {
                        verify = err;
                    }
                    len--;
                    addrbak.word++;
                }
                //请求下一帧
               index.ush++;
               KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
               KeyCmd.req.dat[DAT_VALVE] =0x02;
               KeyCmd.req.dat[3]=index.uch[1];
               KeyCmd.req.dat[4]=index.uch[0];//索引号
             }
            else if((chksum !=0)||(verify == err))   //err,请求当前帧
            {
                KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
                KeyCmd.req.dat[DAT_VALVE] =0x01;
                KeyCmd.req.dat[3]=index.uch[1];
                KeyCmd.req.dat[4]=index.uch[0];//索引号
            }
            break;
        }
        case 1:  //结束
        {
            for(uint8 i=4;i<=8;i++)
            {
                chksum = chksum+ KeyCmd.rsp.dat[i];
            }
            if(chksum!=0)
            {
                KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
                KeyCmd.req.dat[DAT_VALVE] =0x01;
                KeyCmd.req.dat[3]=index.uch[1];
                KeyCmd.req.dat[4]=index.uch[0];//索引号
            }
            else
            {
                config=0;
                bak_ok_flg=1;
                KeyCmd.req.dat[DAT_FUN_CMD] =0xE5;
                KeyCmd.req.dat[DAT_VALVE] =0x01;
                work_state = WORK_STATE_IDLE;
            }
            index.ush=0;
            break;
        }
        case 2:
        {
            break;
        }
        case 4: //线地址 eeprom or config
        {
            config=1;
            index.ush++;
            KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
            KeyCmd.req.dat[DAT_VALVE] =0x02;
            KeyCmd.req.dat[3]=index.uch[1];
            KeyCmd.req.dat[4]=index.uch[0];//索引号
            break;
        }
        default:
        {
            break;
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
    uint8 cmd = KeyCmd.rsp.dat[DAT_FUN_CMD];
    switch(cmd)
    {
        case 0xD1://升级
        {
            index.ush=0,       //
            work_state = WORK_MCU_UPDATE;
            if(KeyCmd.req.dat[DAT_VALVE]== 0xFC) //强制升级
            {
                nv_write(0, 0,0xAA);
                #asm
                    ljmp BOOT_START
                #endasm
            }
            else
            {
                if(KeyCmd.rsp.dat[3]>soft_version) //大于当前版本号
                {
                    KeyCmd.req.dat[DAT_FUN_CMD] =0xE1;
                    KeyCmd.req.dat[DAT_VALVE] =0x01;
                    soft_chksum.uch[3]=KeyCmd.rsp.dat[4]; //
                    soft_chksum.uch[2]=KeyCmd.rsp.dat[5]; //
                    soft_chksum.uch[1]=KeyCmd.rsp.dat[6]; //
                    soft_chksum.uch[0]=KeyCmd.rsp.dat[7]; //
                    update_flg= 1;
                    //show_update();
                }
                else
                {
                    KeyCmd.req.dat[DAT_FUN_CMD] =0xE1;
                    KeyCmd.req.dat[DAT_VALVE] =0x02;
                    work_state = WORK_STATE_IDLE;
                    update_flg = 0;
                    //show_awaken();
                }
            }
            break;
        }
        case 0xD2://解析hex文件
        {
            if(work_state == WORK_MCU_UPDATE)
            {
                if(update_flg != 1)  //重新升级
                {
                    KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
                    KeyCmd.req.dat[DAT_VALVE] =0x03;
                    return ;
                }
                get_hex_file();
            }
            break;
        }
        default:
        {
            if(work_state !=WORK_MCU_UPDATE)
            {
                if(0 == KeyCmd.req.dat[DAT_FUN_CMD])  //判断功能码 按键板是否在使用
                {
               #if key_5
                    key_inflow_sync();
                    key_drain_sync();
                    key_state_sync();
                    key_temp_sync();
                #else
                    key_massage_sync();
                    key_lamp_sync();
                    key_inflow_sync();
                    key_drain_sync();
                    key_state_sync();
                    key_temp_sync();
                #endif
                }
                KeyCmd.req.dat[DAT_ERR_NUM] =KeyCmd.rsp.dat[DAT_ERR_NUM];      //错误码
                KeyCmd.req.dat[DAT_LIQUID] = KeyCmd.rsp.dat[DAT_LIQUID];       //液位信息
                KeyCmd.req.dat[DAT_TEM_OUT] = KeyCmd.rsp.dat[DAT_TEM_OUT];     //实际温度
                KeyCmd.req.dat[DAT_KEEP_WARM] = KeyCmd.rsp.dat[DAT_KEEP_WARM];  //保温状态
                KeyCmd.req.dat[DAT_CLAEN] = KeyCmd.rsp.dat[DAT_CLAEN];           //清洁状态
                KeyCmd.req.dat[DAT_TEM_PRE] = KeyCmd.rsp.dat[DAT_TEM_PRE];     //浴缸水温
                KeyCmd.req.dat[DAT_MAS_TIME] = KeyCmd.rsp.dat[DAT_MAS_TIME];     //按摩时间
            }
            break;
        }

    }
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    delay_ms(5);
    send_dat(&KeyCmd.req, BUF_SIZE);
    KeyCmd.req.dat[DAT_FUN_CMD]=0;          //清功能码
    KeyCmd.req.dat[25]=0;
    if(bak_ok_flg == 1)//成功备份
    {
        bak_ok_flg=0;
        nv_write(0, 0,0x55);
        addr.word+=0x08;      //发送终止地址
        nv_write(0, 1,addr.byte[1]);
        nv_write(0, 2,addr.byte[0]);
        #asm
            ljmp BOOT_START
        #endasm
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
    ShowPar.temp_val -= val ;
    if(ShowPar.temp_val < TEMPERATURE_MIN)
    {
        ShowPar.temp_val = TEMPERATURE_MIN;
         Flg.temp_flash_flg =1;
         flash_cnt =0;
    }
    else
    {
        Flg.temp_flash_flg = 0;
    }
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;                              // 功能码：水龙头出水温度改变
    KeyCmd.req.dat[DAT_VALVE]=0x00;
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
    ShowPar.temp_val += val ;
    if(ShowPar.temp_val > TEMPERATURE_MAX)
    {
        ShowPar.temp_val = TEMPERATURE_MAX;
        Flg.temp_flash_flg =1;
        flash_cnt =0;
    }
    else
    {
        Flg.temp_flash_flg = 0;
    }
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;        // 功能码：水龙头出水温度改变
    KeyCmd.req.dat[DAT_VALVE]=0x00;
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
    //KeyCmd.req.dat[DAT_TEM_PRE]  = 0x26;
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    KeyCmd.req.end_num1 = 0x0F;
    KeyCmd.req.end_num2 = 0x04;
    show_tempture( ShowPar.temp_val);
    dbg("temp init\r\n");
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


