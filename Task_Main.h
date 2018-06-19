#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"

#define key_5  0

// ����ṹ�壺
typedef struct _TASK_COMPONENTS
{
    uint8 Run;                 // �������б�ǣ�0-�����У�1����
    uint16 Timer;              // ��ʱ��
    uint16 ItvTime;              // �������м��ʱ��
    void (*TaskHook)(void);    // Ҫ���е������� ����ָ��
} TASK_COMPONENTS;              // ������


/*�������ܶ���*/

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

uint8 Button_id = 0;   //����id��
uint8 frame_err=0;

typedef struct
{
    uint8 lock_flg:1;
    uint8 lcd_sleep_flg:1;      //lcd˯�߱�־
    uint8 temp_flash_flg:1;     //�����¶� ������˸3�Σ���˸Ƶ��1��/0.5���־
    uint8 frame_ok_fag:1;       //һ֡������ȷ��־
    uint8 clean_err_flg:1;         //clean err��־
    uint8 err_flg:1;
    uint8 temp_disreach_flg:1;        //ˮ�±��� 0-������1-����

}tFlag_t;

tFlag_t   Flg;

enum
{
  WORK_STATE_IDLE = 0,
  WORK_STATE_LOCK,      //��ͯ��
  WORK_STATE_CLEAN,
  WORK_WIFI_PAIR,
  WORK_STATE_MAX
};


/*
0-  ��ַ��
1-  ������
2-  ������
3-  �¶ȸ�
4-  �¶ȵ�
����
*/
//DAT����ö�ٱ���
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


//����Э��ṹ��
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
    }req;               //����
    struct
    {
        uint8 sta_num1;
        uint8 sta_num2;
        uint8 dat[crc_len];
        uint8 crc_num;
        uint8 end_num1;
        uint8 end_num2;
    }rsp;                   //����
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

