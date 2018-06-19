#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"

#define key_5  0

// 任务结构体：
typedef struct _TASK_COMPONENTS
{
    uint8 Run;                 // 程序运行标记：0-不运行，1运行
    uint16 Timer;              // 计时器
    uint16 ItvTime;              // 任务运行间隔时间
    void (*TaskHook)(void);    // 要运行的任务函数 函数指针
} TASK_COMPONENTS;              // 任务定义


/*按键功能定义*/

#if 0 //key_5

#define   ALL_CLOSE          0X00
#define   TAP_VALVE          0X04
#define   SHOWER_VALVE       0X08
#define   DRAIN_VALVE        0X10
#define   INC_VALVE          0X01
#define   DEC_VALVE          0X02
#else
#define   ALL_CLOSE          0X00
#define   TAP_VALVE          0X01
#define   SHOWER_VALVE       0X02
#define   DRAIN_VALVE        0X04
#define   INC_VALVE          0X08
#define   DEC_VALVE          0X10

#endif
#define   WATER_VALVE        0X20
#define   AIR_VALVE          0X40
#define   LAMP_VALVE         0X80
#define   LOCK_VALVE        (TAP_VALVE|SHOWER_VALVE|DEC_VALVE)
#define   CLEAN_VALVE       (INC_VALVE|WATER_VALVE)
/*WIFI PAIR*/
#define   WIFI_VALVE        (TAP_VALVE|DRAIN_VALVE)


//key io
#define   KEY_SBIO_IN         (TRISB = 0XFF)
#define   KEY_DAT             (PORTB)



#define     BUF_SIZE   32
#define     send_cnt   1
#define     crc_len    (BUF_SIZE-5)
uint8       Recv_Buf[BUF_SIZE+8];

uint8 Button_id = 0;   //按键id号
uint8 frame_err=0;

typedef struct
{
    uint8 lock_flg:1;
    uint8 lcd_sleep_flg:1;      //lcd睡眠标志
    uint8 temp_flash_flg:1;     //极限温度 连续闪烁3次，闪烁频率1次/0.5秒标志
    uint8 frame_ok_fag:1;       //一帧数据正确标志
    uint8 clean_err_flg:1;         //clean err标志
    uint8 err_flg:1;
    uint8 temp_disreach_flg:1;        //水温保护 0-慢闪，1-快闪

}tFlag_t;

tFlag_t   Flg;

enum
{
  WORK_STATE_IDLE = 0,
  WORK_STATE_LOCK,      //儿童锁
  WORK_STATE_CLEAN,
  WORK_WIFI_PAIR,
  WORK_STATE_MAX
};


/*
0-  地址码
1-  功能码
2-  数据码
3-  温度高
4-  温度低
保留
*/
//DAT数据枚举变量
enum
{
    DAT_ADDR = 0,
    DAT_FUN_CMD =1,
    DAT_VALVE =2,
#if key_5
    DAT_TEMP = 6,
#else
    DAT_TEMP = 7,
#endif
    DAT_MAX
};

#define DAT_ERR_NUM  26


enum
{
    FUN_IDLE =0,
    FUN_1,
    FUN_2,   // 2
    FUN_3,
    FUN_4,
    FUN_5,     //5
    FUN_6,  //6
    FUN_7,      //7
    FUN_8,      //8
    FUN_MAX
};


enum
{
  LAMP_OFF =0,
  LAMP_RED,     //0x01 -- 红
  LAMP_GREEN,   //0x02 -- 绿
  LAMP_BLUE,    //0x03 -- 蓝
  LAMP_YELLOW,  //0x04 -- 黄
  LAMP_CYAN,    //0x05 -- 青
  LAMP_PINK,    //0X06 --粉红
  LAMP_WHITE,   //0x07 -- 白
  LAMP_CYCLE,   //循环
  LAMP_STOP,
  LAMP_MAX
};


typedef struct
{
    union
    {
        struct
        {
            uint8  tap_state: 1;                 // 水龙头状态
            uint8  shower_state: 1;              // 花洒状态
            uint8  drain_state: 1;            // 排水阀状态
            uint8  inc_state: 1;
            uint8  dec_state: 1;
            uint8  water_state: 1;              // 水按摩状态
            uint8  air_state: 1;              // 气按摩状态
            uint8  lamp_state: 1;               // 灯光状态

        };
        uint8 val;
    };
}tShowParams_t;


//串口协议结构体
typedef struct
{
    struct
    {
        uint8 sta_num1;
        uint8 sta_num2;
        uint8 dat[crc_len];
        uint8 crc_num;
        uint8 end_num1;
        uint8 end_num2;
    }req;               //发送
    struct
    {
        uint8 sta_num1;
        uint8 sta_num2;
        uint8 dat[crc_len];
        uint8 crc_num;
        uint8 end_num1;
        uint8 end_num2;
    }rsp;                   //接收
}tKeyCmd_t;


 tKeyCmd_t  KeyCmd;

 tShowParams_t  ShowPar ;


extern void BSP_init(void);
extern void key_progress(void);
extern void show_work(void);

extern void TaskProcess(void);
extern void TaskRemarks(void);
extern void receiveHandler(uint8 ui8Data);
extern void Serial_Processing (void);
extern uint8 CRC8_SUM(void * p, uint8 len);

#endif

