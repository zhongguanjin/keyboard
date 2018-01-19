#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"


// 任务结构体：
typedef struct _TASK_COMPONENTS
{
    uint8 Run;                 // 程序运行标记：0-不运行，1运行
    uint16 Timer;              // 计时器
    uint16 ItvTime;              // 任务运行间隔时间
    void (*TaskHook)(void);    // 要运行的任务函数 函数指针
} TASK_COMPONENTS;              // 任务定义


/*按键功能定义*/

#define   ALL_CLOSE          0X00
#define   TAP_VALVE          0X01
#define   SHOWER_VALVE       0X02
#define   DRAIN_VALVE        0X04
#define   INC_VALVE          0X08
#define   DEC_VALVE          0X10
#define   WATER_VALVE        0X20
#define   AIR_VALVE          0X40
#define   LAMP_VALVE         0X80

#define   LOCK_VALVE        (TAP_VALVE|SHOWER_VALVE|DEC_VALVE)
#define   CLEAN_VALVE       (INC_VALVE|WATER_VALVE)


//key io
#define   KEY_SBIO_IN         (TRISB = 0XFF)
#define   KEY_DAT             (PORTB)


#define   TEMPERATURE_MAX     460         // 最大温度
#define   TEMPERATURE_MIN     150         // 最低温度

#define     BUF_SIZE   32
#define     send_cnt   1
#define     crc_len    (BUF_SIZE-5)
uint8       Recv_Buf[BUF_SIZE+8];

uint8 Button_id = 0;   //按键id号

#define   eeprom_addr       0x02

typedef struct
{
    uint8 lock_flg:1;
    uint8 lcd_sleep_flg:1;      //lcd睡眠标志
    uint8 temp_flash_flg:1;     //极限温度 连续闪烁3次，闪烁频率1次/0.5秒标志
    uint8 frame_ok_fag:1;       //一帧数据正确标志
    uint8 err_del_flg:1;
}tFlag_t;

tFlag_t   Flg;

enum
{
  WORK_STATE_IDLE = 0,
  WORK_STATE_LOCK,      //儿童锁
  WORK_STATE_ERR,
  WORK_STATE_CLEAN,
  WORK_STATE_TEST,

  WORK_STATE_MAX
};


/*
0-  地址码
1-  功能码
2-  数据码
3-  温度高
4-  温度低
5-  流量档位
6-  出水温度
7-  保温温度
8-  液位信息
9-  按摩信息
10- 龙头+花洒
11- 灯光
12- 保温状态
13- 下水器
14- 管道清洁
保留
18- 错误码
*/
//DAT数据枚举变量
enum
{
    DAT_ADDR = 0,
    DAT_FUN_CMD,
    DAT_VALVE,
    DAT_TEMP_H,
    DAT_TEMP_L,
    DAT_FLOW,          //5
    DAT_TEM_OUT,       //6
    DAT_TEM_PRE,      // 7
    DAT_LIQUID,       //8
    DAT_MASSAGE,      //9
    DAT_STATE,        //10
    DAT_LIGHT,        //11
    DAT_KEEP_WARM,   //12
    DAT_DRAIN,       //13
    DAT_CLAEN,
    DAT_LOCK,       //15
    DAT_SPARE2,
    DAT_SPARE3,
    DAT_ERR_NUM,   //18
    DAT_MAX
};

/*
0x00 -- 空指令 查询
0x01 -- 进水通道切换(此时流量与温度对应发生变化)
0x02 -- 排水
0x03 -- 流量变化
0x04 -- 温度变化
0x05 -- 按摩
0x06 -- 保温
0x07 -- 清洁功能
0x08 -- 灯光
0x09 -- 童锁
*/
enum
{
    FUN_IDLE =0,
    FUN_INFLOW,
    FUN_DRAINAGE,   // 2
    FUN_FLOW,
    FUN_TEMP,
    FUN_MASSAGE,     //5
    FUN_KEEP_WARM,  //6
    FUN_CLEAN,      //7
    FUN_LIGHT,      //8
    FUN_LOCK,       //9
    FUN_MAX
};


enum
{
    MASSAGE_GEAR_OFF =0,
    MASSAGE_GEAR_ON1,
    MASSAGE_GEAR_ON2,
    MASSAGE_GEAR_ON3,
    MASSAGE_GEAR_ON4,
    MASSAGE_GEAR_ON5,
    MASSAGE_GEAR_MAX
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
    uint16 temp_val ;                 // 当前温度
    uint8  air_gear;              //气按摩档位
    uint8  water_gear;            //水按摩档位
    uint8  lamp_gear;                     //灯光颜色
    uint8  switch_flg: 1;               // 温度显示与状态切换标记  1-- 当前为开关状态
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

/* BEGIN: Added by zgj, 2018/1/19 */
enum
{
    STATE_0 = 0,
    STATE_1,
    STATE_2,
    STATE_3,
    STATE_4,
    STATE_5,
    STATE_6,
    STATE_7,
    STATE_8,
    STATE_9,
    STATE_10,
    STATE_11,      //11
    STATE_12,
    STATE_13,    //13
    STATE_14,
    STATE_15,      //15
    STATE_MAX
};
/* END:   Added by zgj, 2018/1/19 */


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
 uint8 Recv_Len = 0;	// 接收长度
 tShowParams_t  ShowPar ;


extern void BSP_init(void);
extern void key_progress(void);
extern void show_work(void);
extern void show_temp_actul(void);
extern void TaskProcess(void);
extern void TaskRemarks(void);
extern void receiveHandler(uint8 ui8Data);
extern void Serial_Processing (void);


#endif

