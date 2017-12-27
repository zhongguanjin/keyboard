#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"
#include "print.h"

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

#define   LOCK_VALVE        (TAP_VALVE|INC_VALVE|DEC_VALVE) //0x19


//key io

#define   KEY_SBIO_IN         (TRISB = 0XFF)
#define   KEY_DAT             (PORTB)


#define   TEMPERATURE_MAX     460         // 最大温度
#define   TEMPERATURE_MIN     150         // 最低温度
#define     BUF_SIZE   16
#define     send_cnt   1
#define     crc_len    (BUF_SIZE-3)
uint8       Recv_Buf[BUF_SIZE+10];
uint8       Send_Buf[BUF_SIZE+10];


uint8 frame_ok_fag;       //一帧数据正确标志
enum
{
  WORK_STATE_IDLE = 0,
  WORK_STATE_LOCK,      //儿童锁
  WORK_STATE_ERR,
  WORK_STATE_MAX
};

//DAT数据枚举变量
enum
{
    DAT_FUN_CMD = 0,
    DAT_TEMP_HIGH,
    DAT_TEMP_LOW,
    DAT_STATE,         // 3
    DAT_FLOW_GEAR,
    DAT_COLOUR,         // 5
    DAT_MASSAGE,        //6
    DAT_PER_TEMP,         // 7保温温度
    DAT_DRAINAGE,       //8
    DAT_SPARE1,
    DAT_SPARE2,
    DAT_MAX
};

//dat[0],功能码枚举变量
enum
{
    FUN_IDLE =0,
    FUN_CHANNEL_SWITCH,
    FUN_TEMP_GEAR,
    FUN_TEMP_POS,
    FUN_FLOW_GEAR,
    FUN_QUID,         //5
    FUN_LED,
    FUN_MASSAGE,
    FUN_PRE_TEMP,   //8
    FUN_DRAINAGE,
    FUN_OUT_WATER,
    FUN_MAX
};

/*
//DAT数据枚举变量
enum
{
    DAT_FUN_CMD = 0,
    DAT_TEMP_HIGH,
    DAT_TEMP_LOW,
    DAT_FLOW_GEAR,
    DAT_TEM_PRE, // 4
    DAT_MASSAGE,  //5
    DAT_COLOUR,   //6
    DAT_INFO,       //7
    DAT_TEM_OUT,  //8
    DAT_ERR_NUM,   //9
    DAT_SPARE1,
    DAT_MAX
};

//dat[0],功能码枚举变量
enum
{
    FUN_IDLE =0,
    FUN_CHANNEL_SWITCH,
    FUN_DRAINAGE,
    FUN_MASSAGE,
    FUN_LIGHT_CTR,
    FUN_CYCLE, //5
    FUN_TEMP,
    FUN_FLOW,
    FUN_LOCK,
    FUN_MAX
};
*/
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


typedef struct
{
    uint8 id;              //按键id
    uint8 lock_flg :1;
}tButton_t;

//串口协议结构体
typedef struct
{
    struct
    {
        uint8 sta_num;
        uint8 spare1;
        uint8 dev_addr;
        uint8 dat[BUF_SIZE-5];
        uint8 crc_num;
        uint8 end_num;
    }req;               //发送
    struct
    {
        uint8 sta_num;
        uint8 spare1;
        uint8 dev_addr;
        uint8 dat[BUF_SIZE-5];
        uint8 crc_num;
        uint8 end_num;
    }rsp;                   //接收
}tKeyCmd_t;


 tKeyCmd_t  KeyCmd;
 uint8 Recv_Len = 0;	// 接收长度
 tShowParams_t  ShowPar ;
 tButton_t   Button;

extern void BSP_init(void);
extern void key_progress(void);
extern void show_work(void);
extern void show_temp_actul(void);
extern void TaskProcess(void);
extern void TaskRemarks(void);
extern void receiveHandler(uint8 ui8Data);
extern void Serial_Processing (void);


#endif

