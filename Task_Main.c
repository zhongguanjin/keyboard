
#include "Task_Main.h"
#include "uart.h"
#include "stdio.h"
#include <string.h>
#include "dbg.h"

volatile uint8  work_state ;
typedef struct
{
     uint16 sleep ;                   //˯��ʱ��
     uint8  temp38;                  //�¶��л�38�ȵ�ʱ��
     uint16 temp_switch;             //��ˮʱ���¶Ⱥ�Ԥ���¶���ʾ��ʱ�����
     uint8  switch_cnt;            //on,off���¶��л���ʱ�����
     uint8  key_adj;              //����lamp,water,airʹ��+,-��ʹ��ʱ��
     uint8  incdec;                //+,-��led���������ʱ��
     uint16 wifi_pair;
}tTime_T;

tTime_T Time_t;


volatile uint8  key_switch_fag;          //���������л���־
volatile uint8  incdec_fag;
static uint8  flash_cnt;        //�����¶� ������˸�Ĵ���


//��������
uint8   KeyPressDown=0x00; //������Ǵ��� ��һ����Ч���������0
uint8   CurrReadKey;  //��¼����KeyScan()��ȡ��IO�ڼ�ֵ
uint8   LastKey=0x00;    //���������������

#define SIZE 4
static uint8 key_arry[SIZE]; //������ջ����
static int top = 0;
static uint16 time_clean=0;
uint8 clean_state;        //clean ����
static uint8 err_lock=0;

//��������

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

// ����ṹ�����
static TASK_COMPONENTS TaskComps[] =
{
    {0, 20,  20,  TaskKeyScan},               //����ɨ��
    {0, 10,  10,  TaskKeyPrs},             //�������̺���
    {0, 100, 100, TaskShow},               // ��ʾ����

};

// �����嵥
typedef enum _TASK_LIST
{
    TAST_SHOW,             // ��ʾ�¶�
    TAST_KEY_SCAN,
	TAST_KEY_PRS,
    TASKS_MAX                // �ܵĿɹ�����Ķ�ʱ������Ŀ
} TASK_LIST;




/*****************************************************************************
 �� �� ��  : TaskShow
 ��������  : �¶���ʾ����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��5��25�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : TaskKeyScan
 ��������  : ����ɨ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��10��
    ��    ��   : man_sta
    �޸�����   : �����ɺ���

*****************************************************************************/

void TaskKeyScan(void)  //20ms
{
    KEY_SBIO_IN;       //��������Ӧ��IO����Ϊ����״̬
    //KEY_DAT|=0Xff;
    CurrReadKey=(~KEY_DAT)&0Xff; //ȡ��
    Button_id  = CurrReadKey;
    KeyPressDown=(~LastKey)&CurrReadKey; //��һ�ΰ��¼�ֵ
    LastKey=CurrReadKey;                //�������¼�ֵ
}
/*****************************************************************************
 �� �� ��  : TaskKeyPrs
 ��������  : �������̺���
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : TAP_EventHandler
 ��������  : ��ͷ����������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void TAP_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&TAP_VALVE) //��һ�ΰ���
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
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_1;  // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.tap_state; //������ ��ͷ״̬
        }
    }
}
/*****************************************************************************
 �� �� ��  : SHOWER_EventHandler
 ��������  : ��������������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void SHOWER_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&SHOWER_VALVE)  //��һ�δ���
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
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_2;            // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.shower_state; //������ ����
        }
    }
}
/*****************************************************************************
 �� �� ��  : DRAIN_EventHandler
 ��������  : ��ˮ����������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void DRAIN_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&DRAIN_VALVE)  //��һ�δ���
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
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //������ ��ˮ
        }
    }
}
/*****************************************************************************
 �� �� ��  : INC_EventHandler
 ��������  : ���Ӱ���������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void INC_EventHandler(void)
{
    static uint8 time_cnt = 0;
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&INC_VALVE)  //��һ�δ���
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
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.inc_state; //������ ��ˮ
        }
    }
}
/*****************************************************************************
 �� �� ��  : DEC_EventHandler
 ��������  : ���ٰ���������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void DEC_EventHandler(void)
{
    static uint8 time_cnt = 0;
   if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&DEC_VALVE)  //��һ�δ���
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
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.dec_state; //������ ��ˮ
        }
    }
}
/*****************************************************************************
 �� �� ��  : AIR_EventHandler
 ��������  : ����Ħ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_7;            // �����룺07
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.air_state; //������ ��ˮ
        }
    }
}
/*****************************************************************************
 �� �� ��  : WATER_EventHandler
 ��������  : ˮ��Ħ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_6;            // �����룺06
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.water_state; //������ ��ˮ
        }
    }
}
/*****************************************************************************
 �� �� ��  : LAMP_EventHandler
 ��������  : �ƹⰴ��������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��5��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void LAMP_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&LAMP_VALVE)
        {
            KeyPressDown = 0;
            ShowPar.lamp_state ^= 0x01;
            if(ShowPar.lamp_state == STATE_ON) //�򿪵ƣ������ж�����+ -����
            {
               LED_LAMP_ON;
            }
            else  //�ص�off
            {
                LED_LAMP_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_8;  // ������:06
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_state;
        }
    }
}

/*****************************************************************************
 �� �� ��  : CRC8_SUM
 ��������  : CRCУ�麯��
 �������  : void *p
             uint8 len
 �������  : crc8        --check sum
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��6��30�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : Serial_Processing
 ��������  : ���ڽ��մ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void Serial_Processing (void)
{
    static uint8 state=0;
    memcpy(&KeyCmd.rsp,Recv_Buf,sizeof(KeyCmd.rsp));
    KeyCmd.req.dat[DAT_TEMP] =KeyCmd.rsp.dat[DAT_TEMP];
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    send_dat(&KeyCmd.req, BUF_SIZE);
    KeyCmd.req.dat[DAT_FUN_CMD]=0;          //�幦����
    memset(&KeyCmd.rsp,0,sizeof(KeyCmd.rsp));
}
/*****************************************************************************
 �� �� ��  : receiveHandler
 ��������  : ���ڽ��ջص�����
 �������  : uint8 ui8Data
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/

void receiveHandler(uint8 ui8Data)
{
    uint8 check_sum = 0;
    static uint8 Recv_Len = 0;    // ���ճ���
    static uint8 err_cnt= 0;
    Recv_Buf[Recv_Len] = ui8Data;
    if(Recv_Buf[0]==0x02)
    {
        Recv_Len++;
        if(Recv_Len >= BUF_SIZE) //���յ�32byte������
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
                else if((Recv_Buf[31]!= 0x04)||(Recv_Buf[30]!= 0x0B))  //�����벻��
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
 �� �� ��  : BSP_init
 ��������  : BSP��ʼ������
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��6��29�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void BSP_init(void)
{
    LED_INIT();
    KEY_SBIO_IN;  //RB�����ó�����ģʽ
    /* BEGIN: Added by zgj, 2018/1/5 */
    //��ʼ������ɫ����Ħ��λ
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
 �� �� ��  : TaskRemarks
 ��������  : �����Ǵ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��5��24�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void TaskRemarks(void)
{
    uint8 i;
    for (i=0; i<TASKS_MAX; i++)                                 // �������ʱ�䴦��
    {
         if (TaskComps[i].Timer)                                // ʱ�䲻Ϊ0
         {
            TaskComps[i].Timer--;                                // ��ȥһ������
            if (TaskComps[i].Timer == 0)                            // ʱ�������
            {
                 TaskComps[i].Timer = TaskComps[i].ItvTime;       // �ָ���ʱ��ֵ��������һ��
                 TaskComps[i].Run = ON;                             // �����������
            }
        }
   }
}
/*****************************************************************************
 �� �� ��  : TaskProcess
 ��������  : ������̺���
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��5��24�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void TaskProcess(void)
{
    uint8 i;
    for (i=0; i<TASKS_MAX; i++)           // �������ʱ�䴦��
    {
         if (TaskComps[i].Run)           // ʱ�䲻Ϊ0
        {
             TaskComps[i].TaskHook();         // ��������
             TaskComps[i].Run = 0;          // ��־��0
        }
    }
}


