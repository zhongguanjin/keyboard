#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"
#include "print.h"

// ����ṹ�壺
typedef struct _TASK_COMPONENTS
{
    uint8 Run;                 // �������б�ǣ�0-�����У�1����
    uint16 Timer;              // ��ʱ��
    uint16 ItvTime;              // �������м��ʱ��
    void (*TaskHook)(void);    // Ҫ���е������� ����ָ��
} TASK_COMPONENTS;              // ������


/*�������ܶ���*/

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


#define   TEMPERATURE_MAX     460         // ����¶�
#define   TEMPERATURE_MIN     150         // ����¶�
#define     BUF_SIZE   16
#define     send_cnt   1
#define     crc_len    (BUF_SIZE-3)
uint8       Recv_Buf[BUF_SIZE+10];
uint8       Send_Buf[BUF_SIZE+10];


uint8 frame_ok_fag;       //һ֡������ȷ��־
enum
{
  WORK_STATE_IDLE = 0,
  WORK_STATE_LOCK,      //��ͯ��
  WORK_STATE_ERR,
  WORK_STATE_MAX
};

//DAT����ö�ٱ���
enum
{
    DAT_FUN_CMD = 0,
    DAT_TEMP_HIGH,
    DAT_TEMP_LOW,
    DAT_STATE,         // 3
    DAT_FLOW_GEAR,
    DAT_COLOUR,         // 5
    DAT_MASSAGE,        //6
    DAT_PER_TEMP,         // 7�����¶�
    DAT_DRAINAGE,       //8
    DAT_SPARE1,
    DAT_SPARE2,
    DAT_MAX
};

//dat[0],������ö�ٱ���
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
//DAT����ö�ٱ���
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

//dat[0],������ö�ٱ���
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
  LAMP_RED,     //0x01 -- ��
  LAMP_GREEN,   //0x02 -- ��
  LAMP_BLUE,    //0x03 -- ��
  LAMP_YELLOW,  //0x04 -- ��
  LAMP_CYAN,    //0x05 -- ��
  LAMP_PINK,    //0X06 --�ۺ�
  LAMP_WHITE,   //0x07 -- ��
  LAMP_CYCLE,   //ѭ��
  LAMP_STOP,
  LAMP_MAX
};


typedef struct
{
    uint16 temp_val ;                 // ��ǰ�¶�
    uint8  air_gear;              //����Ħ��λ
    uint8  water_gear;            //ˮ��Ħ��λ
    uint8  lamp_gear;                     //�ƹ���ɫ
    uint8  switch_flg: 1;               // �¶���ʾ��״̬�л����  1-- ��ǰΪ����״̬
    union
    {
        struct
        {
            uint8  tap_state: 1;                 // ˮ��ͷ״̬
            uint8  shower_state: 1;              // ����״̬
            uint8  drain_state: 1;            // ��ˮ��״̬
            uint8  inc_state: 1;
            uint8  dec_state: 1;
            uint8  water_state: 1;              // ˮ��Ħ״̬
            uint8  air_state: 1;              // ����Ħ״̬
            uint8  lamp_state: 1;               // �ƹ�״̬

        };
        uint8 val;
    };
}tShowParams_t;


typedef struct
{
    uint8 id;              //����id
    uint8 lock_flg :1;
}tButton_t;

//����Э��ṹ��
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
    }req;               //����
    struct
    {
        uint8 sta_num;
        uint8 spare1;
        uint8 dev_addr;
        uint8 dat[BUF_SIZE-5];
        uint8 crc_num;
        uint8 end_num;
    }rsp;                   //����
}tKeyCmd_t;


 tKeyCmd_t  KeyCmd;
 uint8 Recv_Len = 0;	// ���ճ���
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

