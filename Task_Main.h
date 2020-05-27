#ifndef __TASK_MAIN_H__
#define __TASK_MAIN_H__

#include "config.h"

#define Key_5  1
#define Key_6  0
#define Key_7  0
#define Key_8  0

#define panel_5  1
#define panel_6  2
#define panel_7  3
#define panel_8  4

#define   DEV_TYPE    panel_5

#if(DEV_TYPE == panel_8)
uint8 partnum[12]={2,9,9,0,0,0,0,9,1,1,0,0}; //299000091100
#endif

#if(DEV_TYPE == panel_7)
uint8 partnum[12]={2,9,9,0,0,0,0,3,2,1,0,0};
#endif

#if(DEV_TYPE == panel_6)
uint8 partnum[12]={2,9,9,0,0,0,0,9,1,1,0,0};
#endif

#if(DEV_TYPE == panel_5)
uint8 partnum[12]={2,9,9,0,0,0,0,4,1,1,0,0};
#endif

#define   soft_version      14 //����汾�� v1.4



// ����ṹ�壺
typedef struct _TASK_COMPONENTS
{
    uint8 Run;                 // �������б�ǣ�0-�����У�1����
    uint16 Timer;              // ��ʱ��
    uint16 ItvTime;              // �������м��ʱ��
    void (*TaskHook)(void);    // Ҫ���е������� ����ָ��
} TASK_COMPONENTS;              // ������


#define BOOT_START 0x0000
// 2 k
#define APP_START 	0x0800//0x0800
// 7 k
#define APP_BAK 	0x2400
// 7 k


/*�������ܶ���*/





#if (Key_5||Key_6)
#define   ALL_CLOSE          0X00
#define   TAP_VALVE          0X04
#define   SHOWER_VALVE       0X08
#define   DRAIN_VALVE        0X10
#define   INC_VALVE          0X01
#define   DEC_VALVE          0X02
#define   LAMP_VALVE         0X80
#define   AIR_VALVE          0X40
#define   WATER_VALVE        0X20
#else
#define   ALL_CLOSE          0X00
#define   TAP_VALVE          0X01
#define   SHOWER_VALVE       0X02
#define   DRAIN_VALVE        0X04
#define   INC_VALVE          0X08
#define   DEC_VALVE          0X10
#if (Key_8)
#define   LAMP_VALVE         0X80
#define   AIR_VALVE          0X40
#define   WATER_VALVE        0X20
#elif (Key_7)
#define   LAMP_VALVE         0X40
#define   AIR_VALVE          0X80
#define   WATER_VALVE        0X20
#endif
#endif



#define   LOCK_VALVE        (TAP_VALVE|SHOWER_VALVE|DEC_VALVE)
#define   CLEAN_VALVE       (INC_VALVE|WATER_VALVE)
/*WIFI PAIR*/
#define   WIFI_VALVE        (TAP_VALVE|DRAIN_VALVE)
// WIFI NET CONFIG
#define   NETWORK_VAL       (SHOWER_VALVE|DRAIN_VALVE)



//key io
#define   KEY_SBIO_IN         (TRISB = 0XFF)
#define   KEY_DAT             (PORTB)


#define   TEMPERATURE_MAX     460         // ����¶�
#define   TEMPERATURE_MIN     150         // ����¶�

#define     BUF_SIZE   32
#define     send_cnt   1
#define     crc_len    (BUF_SIZE-5)
uint8       Recv_Buf[BUF_SIZE];

uint8 Button_id = 0;   //����id��


typedef struct
{
    uint8 lock_flg:1;
    uint8 lcd_sleep_flg:1;      //lcd˯�߱�־
    uint8 temp_flash_flg:1;     //�����¶� ������˸3�Σ���˸Ƶ��1��/0.5���־
    uint8 frame_ok_fag:1;       //һ֡������ȷ��־
    uint8 clean_err_flg:1;         //clean err��־
    uint8 err_flg:1;
    uint8 temp_disreach_flg:1;        //ˮ�±��� 0-������1-����
    uint8 err_f1_flg:1;         //f1�����־
    uint8 err_f6_flg:1;
    uint8 net_config_flg:1;
    uint8 othet_st_flg:1;
    uint8 exhibition_flg:1;
    uint8 panel_drain_key:1;  //�����ˮ����
    uint8 holte_mode_flg:1; //�Ƶ�ģʽ
}tFlag_t;

tFlag_t   Flg;

enum
{
  WORK_STATE_IDLE = 0,
  WORK_STATE_LOCK,      //��ͯ��
  WORK_STATE_CLEAN,
  WORK_WIFI_PAIR,
  WORK_MCU_UPDATE,
  WORK_STATE_MAX
};


/*
0-  ��ַ��
1-  ������
2-  ������
3-  �¶ȸ�
4-  �¶ȵ�
5-  ������λ
6-  ��ˮ�¶�
7-  �����¶�
8-  Һλ��Ϣ
9-  ��Ħ��Ϣ
10- ��ͷ+����
11- �ƹ�
12- ����״̬
13- ��ˮ��
14- �ܵ����
����
*/
//DAT����ö�ٱ���
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
    DAT_MAX
};

#define DAT_ERR_NUM     26
#define DAT_OTHER_ST    25
#define DAT_WIFI_PAIR   19
#define DAT_MAS_TIME    18
/*
0x00 -- ��ָ�� ��ѯ
0x01 -- ��ˮͨ���л�(��ʱ�������¶ȶ�Ӧ�����仯)
0x02 -- ��ˮ
0x03 -- �����仯
0x04 -- �¶ȱ仯
0x05 -- ��Ħ
0x06 -- ����
0x07 -- ��๦��
0x08 -- �ƹ�
0x09 -- ͯ��
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
    FUN_WIFI,
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
    uint8  lamp_gear;                     //�ƹ���ɫ 1-8  ��ס�·���ɫ��¼
    uint8  light_state;                 //�ƹ�״̬ 0-8
    uint8  on_off_flg: 1;               // �¶���ʾ��״̬�л����  1-- ��ǰΪ����״̬
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
            uint8  lamp_state: 1;               // �ƹ�״̬ on - off

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
extern void show_temp_actul(void);
extern void TaskProcess(void);
extern void TaskRemarks(void);
extern void receiveHandler(uint8 ui8Data);
extern void Serial_Processing (void);
extern uint8 CRC8_SUM(void * p, uint8 len);
extern void clear_f6_cnt(void);
#endif

