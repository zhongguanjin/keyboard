
#include "Task_Main.h"
#include "uart.h"
#include "stdio.h"
#include <string.h>
#include "dbg.h"
#include "eeprom.h"

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
     uint16 err_cnt;
}tTime_T;

tTime_T Time_t;

typedef union {
      unsigned int word;
      unsigned char byte[2];
} wordbyte;

volatile uint8  key_adjust_fag;          //���������л���־
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
static uint16 Cstate_time =0;


//zgj 2018-07-12  ota update
#define   soft_version      0x01  //����汾��
UN32      soft_chksum;         // ���У���룬4�ֽ�
uint8 chksum=0;
uint16 block=0;
uint8 verify=0;  //
void get_hex_file(void);
uint8 update_flg=0;
uint8 bak_ok_flg = 0;
wordbyte addr;
static UN16 index=0;         //
//end
uint8 f6_err_cnt=0;   //zgj 2018-7-26 ͨ�Ź���

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



void clear_f6_cnt(void)
{
    f6_err_cnt = 0;
    if(Flg.err_f6_flg==1)  //����f6ʱ
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
 �� �� ��  : TaskShow
 ��������  : ��ʾ����
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
                &&(incdec_fag == 0)&&(Flg.err_flg!=1))           //��ʾ���ڿ���ʱ
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
 �� �� ��  : TaskClean
 ��������  : ���ܵ�������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��25��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void TaskClean() //100ms
{
    switch ( clean_state )
    {
        case STATE_1:
            {
                if((Cstate_time++)==10)  // 1s ����
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
                //if((KeyCmd.req.dat[DAT_LIQUID]&0x01)!=0x01) //���ڵ�Һλ
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
                //if((KeyCmd.req.dat[DAT_LIQUID]&0x01)==0x01) //���ڵ�Һλ
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
                if(Cstate_time ==3620) // 6min2s��
                {
                    Cstate_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //������
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
                //if((KeyCmd.req.dat[DAT_LIQUID]&0x01) == 0x01) //��Һλ
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

/* ��� */
void add(uint8 value)
{
    if(top == SIZE-1) //��
    {
        key_arry[top] = value;
        return ;
    }
    top += 1;
    key_arry[top] = value;
}

void clear(void)
{
    if(top == 0)  //��
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
/* ɾ�� */
void del(uint8 value)
{
    if(top == 0)  //��
    {
        return ;
    }
    if(key_arry[top] == value) //��ջ��Ԫ��
    {
        key_arry[top] = 0; //����
        top = 0;
    }
    else
    {
        for(uint8 i=0;i<top;i++)
        {
            if(key_arry[i] == value)
            {
                for(uint8 j=i; j<top;j++) //��i��������Ԫ����ǰ��
                {
                    key_arry[j]=key_arry[j+1];
                }
                key_arry[top] = 0; //����
                top -= 1;  //��������ջ����ż�1
            }
        }
    }
}

/*****************************************************************************
 �� �� ��  : key_adjust
 ��������  : �������ڹ��ܣ��ƹ⣬ˮ��Ħ������Ħ
 �������  : uint8 id
             uint8 dat
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��12��1��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void key_adjust(uint8 id,uint8 dat)
{
    key_adjust_fag =1;
    show_adj_key(id, dat);
}

/*****************************************************************************
 �� �� ��  : time_cnt_del
 ��������  : ʱ��������㺯��
 �������  : uint8 id -����ֵ
 �������  : ��
 �� �� ֵ  : void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��6��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void time_cnt_del( uint8 id)
{
    Time_t.sleep = 0;
    Time_t.temp38 = 0;
    Time_t.err_cnt=0;
    if((id==TAP_VALVE)||(id==SHOWER_VALVE))//��ͷ����
    {
        Time_t.switch_cnt = 0;                  //on /off ���ʱ��Ҫ��0
        Time_t.temp_switch =0;                  //�¶��л�ʱ�����
        Flg.temp_disreach_flg =0;               //������������־��0
        key_adjust_fag=0;
    }
    if((id==INC_VALVE)||(id==DEC_VALVE))    //+,-
    {
        if(key_adjust_fag==0) //����ģʽ��Ҫ���temp_switch��ʱ��
        {
            Time_t.temp_switch =0;
            Flg.temp_disreach_flg =0;
        }
        Time_t.incdec = 0;
        Time_t.key_adj = 0;
        incdec_fag =1;
    }
    if((id==AIR_VALVE)||(id==WATER_VALVE)||(id==LAMP_VALVE))//��Ħ���ƹ��
    {
         Time_t.key_adj = 0;
         Time_t.switch_cnt = 0;
         ShowPar.on_off_flg = 0;  //��on
    }
    if(id==DRAIN_VALVE)
    {
       Time_t.switch_cnt =0;
    }
}

/*****************************************************************************
 �� �� ��  : show_temp_flash
 ��������  : �¶�������С��˸����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��17��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
        if(flash_cnt >= 3)// 3��
        {
            flash_cnt = 0;
            cnt=0;
            Flg.temp_flash_flg = 0;
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
    if((work_state == WORK_STATE_IDLE)&&(Flg.err_f1_flg == 0)) //û��F1
    {
        if(KeyPressDown&TAP_VALVE) //��һ�ΰ���
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
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;  // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //������ ��ͷ״̬
            time_cnt_del(TAP_VALVE);
            show_state(ShowPar.tap_state);
            dbg("tap,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
    //else if((Flg.err_f1_flg == 1)&&(KeyCmd.req.dat[DAT_ERR_NUM]&0x80))  //f1 err ʱ
    else if(Flg.err_f1_flg == 1)  //f1 err ʱ
    {
         if(KeyPressDown&TAP_VALVE)  //��һ�δ���
        {
            KeyPressDown = 0;
            time_cnt_del(TAP_VALVE);
            ShowPar.tap_state = STATE_ON;
            LED_TAP_ON;
            Flg.err_f1_flg =0;
            ShowPar.temp_val = 380;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.val&0x03;
            KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // �¶ȸ�
            KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
            show_tempture(ShowPar.temp_val);
             dbg("f1 err inflow\r\n");
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
    if((work_state == WORK_STATE_IDLE)&&(Flg.err_f1_flg == 0))
    {
        if(KeyPressDown&SHOWER_VALVE)  //��һ�δ���
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
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //������ ����
            time_cnt_del(SHOWER_VALVE);
            show_state(ShowPar.shower_state);
            dbg("shower,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
    //else if((Flg.err_f1_flg == 1)&&(KeyCmd.req.dat[DAT_ERR_NUM]&0x80))  //f1 err ʱ
    else if(Flg.err_f1_flg == 1)  //f1 err ʱ
    {
        if(KeyPressDown&SHOWER_VALVE)  //��һ�δ���
        {
            KeyPressDown = 0;
            time_cnt_del(SHOWER_VALVE);
            ShowPar.shower_state = STATE_ON;
            LED_SHOWER_ON;
            Flg.err_f1_flg =0;
            ShowPar.temp_val = 380;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.val&0x03;
            KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // �¶ȸ�
            KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
            show_tempture(ShowPar.temp_val);
            dbg("f1 err inflow\r\n");
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
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //������ ��ˮ
            time_cnt_del(DRAIN_VALVE);
            show_state(ShowPar.drain_state);
            dbg("drainage,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
                    if(KeyPressDown&INC_VALVE)  //��һ�δ���
                    {
                        KeyPressDown = 0;
                        time_cnt = 0;
                        set_temp_val_inc(5);
                    }
                    else if((LastKey&INC_VALVE)&&(time_cnt>=40)) //��������400MS
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
                    if(KeyPressDown&INC_VALVE)  //��һ�δ���
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
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // ������:08
                        KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_gear;
                        ShowPar.light_state=ShowPar.lamp_gear;
                        dbg("lamp %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case WATER_VALVE:
                {
                    if(KeyPressDown&INC_VALVE)  //��һ�δ���
                    {
                        KeyPressDown = 0;
                        ShowPar.water_gear++;
                        if(ShowPar.water_gear >MASSAGE_GEAR_ON5)
                        {
                            ShowPar.water_gear = MASSAGE_GEAR_ON5;
                        }
                        key_adjust(key_arry[top],ShowPar.water_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // ������:05
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5); //������
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case AIR_VALVE:
                {
                    if(KeyPressDown&INC_VALVE)  //��һ�δ���
                    {
                        KeyPressDown = 0;
                        ShowPar.air_gear++;
                        if(ShowPar.air_gear >MASSAGE_GEAR_ON5)
                        {
                            ShowPar.air_gear = MASSAGE_GEAR_ON5;
                        }
                        key_adjust(key_arry[top],ShowPar.air_gear);
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // ������:07
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5); //������
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
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
        LED_DEC_ON;
        LED_INC_OFF;
        time_cnt_del(DEC_VALVE); //����ʱ��
        if(Flg.lcd_sleep_flg == 1)//���߻���
        {
            show_awaken();
            return;
        }
        switch (key_arry[top])
        {
            case ALL_CLOSE :
                {
                    time_cnt++;
                    if(KeyPressDown&DEC_VALVE)  //��һ�δ���
                    {
                        KeyPressDown = 0;
                        time_cnt = 0;
                        set_temp_val_dec(5);
                    }
                    else if((LastKey&DEC_VALVE)&&(time_cnt>=40)) //��������
                    {
                         time_cnt = 0;
                         set_temp_val_dec(10);
                    }
                    break;
                }
            case LAMP_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //��һ�δ���
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
                       KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // ������:06
                       KeyCmd.req.dat[DAT_VALVE]=ShowPar.lamp_gear;
                       ShowPar.light_state=ShowPar.lamp_gear;
                       dbg("lamp %x\r\n", KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case WATER_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //��һ�δ���
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
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // ������:07
                        KeyCmd.req.dat[DAT_VALVE] = (ShowPar.water_gear<<2)
                                +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
                        dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case AIR_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //��һ�δ���
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
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_MASSAGE;  // ������:07
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
               if(key_arry[top]==0)  //Ϊ��
               {
                   Time_t.key_adj = 0;
                   key_adjust_fag = 0;
               }
               show_tempture(ShowPar.temp_val);
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // �����룺07
            dbg("massage %x\r\n",KeyCmd.req.dat[DAT_VALVE]);
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
                if(key_arry[top]==0)  //Ϊ��
                {
                    Time_t.key_adj = 0;
                    key_adjust_fag = 0;
                }
                show_tempture(ShowPar.temp_val);
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // �����룺07
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
    		KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //������
    		KeyCmd.req.dat[DAT_VALVE] =0x00;
            Cstate_time = 0;
    		show_tempture(ShowPar.temp_val);
            clean_state = STATE_0;
    		dbg("clean cancel\r\n");
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
            time_cnt_del(LAMP_VALVE);
            if(Flg.lcd_sleep_flg ==1)
            {
                show_awaken();
                return ;
            }
            ShowPar.lamp_state ^= 0x01;
            if(ShowPar.lamp_state == STATE_ON) //�򿪵ƣ������ж�����+ -����
            {
               LED_LAMP_ON;
               KeyCmd.req.dat[DAT_VALVE] = ShowPar.lamp_gear;
               ShowPar.light_state = ShowPar.lamp_gear;
               add(LAMP_VALVE);
               key_adjust(key_arry[top],ShowPar.lamp_gear);
            }
            else  //�ص�off
            {
                LED_LAMP_OFF;
                KeyCmd.req.dat[DAT_VALVE] = LAMP_OFF;
                ShowPar.light_state = LAMP_OFF;
               del(LAMP_VALVE);
               if(key_arry[top]==0)  //Ϊ��
               {
                   Time_t.key_adj = 0;
                   key_adjust_fag = 0;
               }
               show_tempture(ShowPar.temp_val);
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LIGHT;  // ������:06
            dbg("lamp %x\r\n", KeyCmd.req.dat[DAT_VALVE]);
        }
    }
}

/*****************************************************************************
 �� �� ��  : LOCK_EventHandler
 ��������  : ͯ������������
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
void LOCK_EventHandler(void) //10ms
{
    if((work_state == WORK_STATE_IDLE)&&(Flg.lock_flg ==0))
    {
        Flg.lock_flg =1;   //��ⰴ���ɿ� 0-�ɿ���1-����
        work_state = WORK_STATE_LOCK;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // �����룺��ˮ���ظı�
        KeyCmd.req.dat[DAT_VALVE] = 0x01;
        dbg("idle -> lock,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
    }
    if((work_state == WORK_STATE_LOCK)&&(Flg.lock_flg ==0))
    {
       Flg.lock_flg =1;
       show_awaken();
       work_state = WORK_STATE_IDLE;
       KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // �����룺��ˮ���ظı�
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
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_WIFI;            // �����룺wifi pair
        KeyCmd.req.dat[DAT_VALVE] = 0x01;
		KeyCmd.req.dat[DAT_WIFI_PAIR] = 0x00;
        dbg("idle -> wifi pair\r\n");
    }
}

/*****************************************************************************
 �� �� ��  : judge_err_num
 ��������  : ���ϴ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��4��20��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void judge_err_num(void)//10ms
{
    if((KeyCmd.req.dat[DAT_ERR_NUM]&0x7F) != 0x00)  //�д���
    {
        if((key_adjust_fag==0)&&(ShowPar.on_off_flg==0)&&(incdec_fag == 0))
        {
            Flg.err_flg =1;
            if((Time_t.err_cnt++)>=400)
            {
                Time_t.err_cnt=0;
                write_err_num(KeyCmd.req.dat[DAT_ERR_NUM]&0x7F);
            }
            if((KeyCmd.req.dat[DAT_ERR_NUM]&0x01) == 0x01)//f1����
            {
                Flg.err_f1_flg =1;
            }
            work_state = WORK_STATE_IDLE;
        }
    }
    else //�޴���
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
 �� �� ��  : wifi_pair_pro
 ��������  : wifi ��Դ���
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void wifi_pair_pro(void)
{
    if((Time_t.wifi_pair++)==10) // 1s���͹رհ�ť
    {
        if(ShowPar.tap_state==ON)
        {
            ShowPar.tap_state=OFF;
            LED_TAP_OFF;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;  // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //������ ��ͷ״̬
        }
        else
        {
            ShowPar.drain_state=OFF;
            LED_DRAIN_OFF;
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_DRAINAGE;
            KeyCmd.req.dat[DAT_VALVE] = ShowPar.drain_state; //������ ��ˮ
        }
    }
    if(Time_t.wifi_pair>=600)
    {
        Time_t.wifi_pair =0;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_WIFI;            // �����룺wifi pair
        KeyCmd.req.dat[DAT_VALVE] = 0x00;
        work_state = WORK_STATE_IDLE;
        show_tempture(ShowPar.temp_val);
        dbg("wifi pair over time\r\n");
    }
}

/*****************************************************************************
 �� �� ��  : child_lock_show
 ��������  : ��ͯ������ʾ
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
 �� �� ��  : IDLE_EventHandler
 ��������  : ���лص�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��4��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void IDLE_EventHandler(void) //10ms
{
    static uint8 flg=0;
    if((Time_t.temp38 == 30)&&(work_state == WORK_STATE_IDLE))  //�¶ȱ���30min�� ���л���38��
    {
        if(flg==0)
        {
            flg=1;
            ShowPar.temp_val = 380;
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;        // �����룺ˮ��ͷ��ˮ�¶ȸı�
            KeyCmd.req.dat[DAT_VALVE]=0x00;
            KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // �¶ȸ�
            KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // �¶ȵ�
            KeyCmd.req.dat[25]=0x01;  //�����¿ذ帴λ
            dbg("30min temp->38\r\n");
            Time_t.temp38 = 0;
        }
    }
    if(Time_t.sleep++ >=6000) // 1min��ʱ�䵽  ʱ��10ms
    {
       Time_t.sleep = 0;
       if(Flg.lcd_sleep_flg == 1) //�������
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
 �� �� ��  : CLEAN_EventHandler
 ��������  : ��๦�ܻص�����
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��1��6��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void CLEAN_EventHandler(void)
{
	if(work_state == WORK_STATE_IDLE)
	{
		if((LastKey&CLEAN_VALVE)&&((time_clean++)>=300))// 3s
		{
            time_clean=0;
            work_state = WORK_STATE_CLEAN;
		    if((KeyCmd.req.dat[DAT_LIQUID]&0x01) == 0x01) //��Һλ
		    {
		        Flg.clean_err_flg = 0;
    			show_clean();                           //�����ʾ---
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
 �� �� ��  : show_work
 ��������  : ����״̬���¶��л�����
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
void show_work(void)
{
    if( 1 == ShowPar.on_off_flg)
    {
         Time_t.switch_cnt ++ ;
         if( Time_t.switch_cnt >= 20 ) // 2sʱ�䵽
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
 �� �� ��  : show_temp_actul
 ��������  : ʵ���¶Ⱥ�Ԥ���¶���ʾ����
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
void show_temp_actul(void) // 100ms
{
    uint16 pre_tem=0;
    static uint8 cnt1 =0,cnt=0,show_flg=0;
    if((ShowPar.tap_state == ON)||(ShowPar.shower_state == ON )) //��ˮ״̬
    {
        pre_tem = KeyCmd.req.dat[DAT_TEM_OUT]*10;
        if((pre_tem < ShowPar.temp_val-20)||((pre_tem > ShowPar.temp_val+20))) //�ﲻ��Ԥ���¶�
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
 �� �� ��  : sync_temp_show
 ��������  : ͬ��״̬�¶���ʾ
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void sync_temp_show(void)
{
    if((key_adjust_fag==0)&&(ShowPar.on_off_flg==0)
        &&(incdec_fag == 0)&&(Flg.err_flg!=1))           //��ʾ���ڿ���ʱ
    {
        show_tempture(ShowPar.temp_val);
        dbg("chk\r\n");
    }
}

/*****************************************************************************
 �� �� ��  : key_massage_sync
 ��������  : ��Ħ״̬ͬ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void key_massage_sync(void)
{
    /* BEGIN: Added by zgj, 2018/1/15 */
    uint8 massage_dat = (ShowPar.water_gear<<2)+(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
    if(((KeyCmd.rsp.dat[DAT_MASSAGE]&0x1D)!=(KeyCmd.req.dat[DAT_MASSAGE]&0x1D))
        ||((KeyCmd.rsp.dat[DAT_MASSAGE]&0x1D)!=(massage_dat&0x1D))) //ˮ��Ħ״̬����
    {
        time_cnt_del(WATER_VALVE);
        ShowPar.water_gear = (KeyCmd.rsp.dat[DAT_MASSAGE]&0x1C)>>2;
        KeyCmd.req.dat[DAT_MASSAGE] =KeyCmd.rsp.dat[DAT_MASSAGE];
        if((KeyCmd.req.dat[DAT_MASSAGE]&0x01)==0x01) //ˮ��Ħ����
        {
            ShowPar.water_state =ON;
            LED_WATER_ON;
            if(work_state == WORK_STATE_IDLE)
            {
                add(WATER_VALVE);
                key_adjust(key_arry[top],ShowPar.water_gear);
            }
        }
        else if((KeyCmd.req.dat[DAT_MASSAGE]&0x01)==0) //ˮ��Ħ�ر�
        {
            ShowPar.water_state =OFF;
            LED_WATER_OFF;
            del(WATER_VALVE);
            if((key_arry[top]==0)&&(work_state == WORK_STATE_IDLE))  //Ϊ��
            {
                Time_t.key_adj = 0;
                key_adjust_fag = 0;
                sync_temp_show();
                dbg("sync water off\r\n");
            }
        }
    }
    if(((KeyCmd.rsp.dat[DAT_MASSAGE]&0xE2)!=(KeyCmd.req.dat[DAT_MASSAGE]&0xE2)
        ||((KeyCmd.rsp.dat[DAT_MASSAGE]&0xE2)!=(massage_dat&0xE2)))) //����Ħ״̬����
    {
        time_cnt_del(AIR_VALVE);
        ShowPar.air_gear = (KeyCmd.rsp.dat[DAT_MASSAGE]&0xE0)>>5;
        KeyCmd.req.dat[DAT_MASSAGE] =KeyCmd.rsp.dat[DAT_MASSAGE];
        if((KeyCmd.req.dat[DAT_MASSAGE]&0x02)==0x02) //����Ħ����
        {
            ShowPar.air_state =ON;
            LED_AIR_ON;
            if(work_state == WORK_STATE_IDLE)
            {
                add(AIR_VALVE);
                key_adjust(key_arry[top],ShowPar.air_gear);
            }
        }
        else if((KeyCmd.req.dat[DAT_MASSAGE]&0x02)==0) //����Ħ�ر�
        {
            ShowPar.air_state =OFF;
            LED_AIR_OFF;
            del(AIR_VALVE);
            if((key_arry[top]==0)&&(work_state == WORK_STATE_IDLE))  //Ϊ��
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
 �� �� ��  : key_lamp_sync
 ��������  : �ƹ�״̬ͬ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
            if((key_arry[top]==0)&&(work_state == WORK_STATE_IDLE))  //Ϊ��
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
 �� �� ��  : key_inflow_sync
 ��������  : ��ͷ����״̬ͬ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void key_inflow_sync(void)
{
    if((KeyCmd.rsp.dat[DAT_STATE]!=KeyCmd.req.dat[DAT_STATE])
        ||(KeyCmd.rsp.dat[DAT_STATE]!=(ShowPar.val&0x03))) //��ˮ״̬����
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
 �� �� ��  : key_drain_sync
 ��������  : ��ˮ��״̬ͬ��
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��8��15�� ������
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void key_drain_sync(void)
{
    if((KeyCmd.rsp.dat[DAT_DRAIN]!=KeyCmd.req.dat[DAT_DRAIN])
        ||(KeyCmd.rsp.dat[DAT_DRAIN]!=ShowPar.drain_state)) //��ˮ��״̬����
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
 �� �� ��  : key_state_sync
 ��������  : ����״̬����ͬ������
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
void key_state_sync(void)
{
    if(KeyCmd.rsp.dat[DAT_LOCK]!=KeyCmd.req.dat[DAT_LOCK]) //ͯ��״̬����
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
    if(KeyCmd.rsp.dat[DAT_CLAEN]!=KeyCmd.req.dat[DAT_CLAEN]) //���״̬����
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
    if(KeyCmd.rsp.dat[DAT_WIFI_PAIR]!=KeyCmd.req.dat[DAT_WIFI_PAIR]) // WIFI_PAIR NUM ����
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
 �� �� ��  : key_temp_sync
 ��������  : �¶�ͬ������
 �������  : void
 �������  : ��
 �� �� ֵ  : void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2018��4��19��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

*****************************************************************************/
void key_temp_sync( void )
{
    if(work_state == WORK_STATE_IDLE)
    {
        if((KeyCmd.rsp.dat[DAT_TEMP_H]!=KeyCmd.req.dat[DAT_TEMP_H])
            ||(KeyCmd.rsp.dat[DAT_TEMP_L]!=KeyCmd.req.dat[DAT_TEMP_L]))   //�¶ȸ���
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
                    Time_t.sleep = 0; //�������µ�һ����
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
    index_bak.uch[0]= KeyCmd.rsp.dat[3];//������
    if(index.ush != index_bak.ush)
    {
        KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
        KeyCmd.req.dat[DAT_VALVE] =0x01;
        KeyCmd.req.dat[3]=index.uch[1];
        KeyCmd.req.dat[4]=index.uch[0];//������
        return ;
    }
    index.uch[1]= KeyCmd.rsp.dat[2];//get the  high byte
    index.uch[0]= KeyCmd.rsp.dat[3];//������
    len = KeyCmd.rsp.dat[4]>>1;    //����/2
    rectype =KeyCmd.rsp.dat[7];   //��¼����
    chksum=0;
    for(uint8 i=4;i<=24;i++)
    {
        chksum = chksum+ KeyCmd.rsp.dat[i];
    }
    switch(rectype)
    {
        case 0: //����
        {
            if(config == 1)
            {
                index.ush++;
                KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
                KeyCmd.req.dat[DAT_VALVE] =0x02;
                KeyCmd.req.dat[3]=index.uch[1];
                KeyCmd.req.dat[4]=index.uch[0];//������
                break;
            }
            addr.byte[1]=KeyCmd.rsp.dat[5];//get the addr high byte
            addr.byte[0]=KeyCmd.rsp.dat[6];//get the addr low byte
            addr.word>>=1;
            addrbak.word=addr.word-APP_START+APP_BAK; //���ݵ�ַ
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
                    if(data.word!=nv_read(1,addrbak.word)) //дһ����ok?
                    {
                        verify = err;
                    }
                    len--;
                    addrbak.word++;
                }
                //������һ֡
               index.ush++;
               KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
               KeyCmd.req.dat[DAT_VALVE] =0x02;
               KeyCmd.req.dat[3]=index.uch[1];
               KeyCmd.req.dat[4]=index.uch[0];//������
             }
            else if((chksum !=0)||(verify == err))   //err,����ǰ֡
            {
                KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
                KeyCmd.req.dat[DAT_VALVE] =0x01;
                KeyCmd.req.dat[3]=index.uch[1];
                KeyCmd.req.dat[4]=index.uch[0];//������
            }
            break;
        }
        case 1:  //����
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
                KeyCmd.req.dat[4]=index.uch[0];//������
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
        case 4: //�ߵ�ַ eeprom or config
        {
            config=1;
            index.ush++;
            KeyCmd.req.dat[DAT_FUN_CMD] =0xE2;
            KeyCmd.req.dat[DAT_VALVE] =0x02;
            KeyCmd.req.dat[3]=index.uch[1];
            KeyCmd.req.dat[4]=index.uch[0];//������
            break;
        }
        default:
        {
            break;
        }
    }
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
    memcpy(&KeyCmd.rsp,Recv_Buf,sizeof(KeyCmd.rsp));
    uint8 cmd = KeyCmd.rsp.dat[DAT_FUN_CMD];
    switch(cmd)
    {
        case 0xD1://����
        {
            index.ush=0,       //
            work_state = WORK_MCU_UPDATE;
            if(KeyCmd.req.dat[DAT_VALVE]== 0xFC) //ǿ������
            {
                nv_write(0, 0,0xAA);
                #asm
                    ljmp BOOT_START
                #endasm
            }
            else
            {
                if(KeyCmd.rsp.dat[3]>soft_version) //���ڵ�ǰ�汾��
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
        case 0xD2://����hex�ļ�
        {
            if(work_state == WORK_MCU_UPDATE)
            {
                if(update_flg != 1)  //��������
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
                if(0 == KeyCmd.req.dat[DAT_FUN_CMD])  //�жϹ����� �������Ƿ���ʹ��
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
                KeyCmd.req.dat[DAT_ERR_NUM] =KeyCmd.rsp.dat[DAT_ERR_NUM];      //������
                KeyCmd.req.dat[DAT_LIQUID] = KeyCmd.rsp.dat[DAT_LIQUID];       //Һλ��Ϣ
                KeyCmd.req.dat[DAT_TEM_OUT] = KeyCmd.rsp.dat[DAT_TEM_OUT];     //ʵ���¶�
                KeyCmd.req.dat[DAT_KEEP_WARM] = KeyCmd.rsp.dat[DAT_KEEP_WARM];  //����״̬
                KeyCmd.req.dat[DAT_CLAEN] = KeyCmd.rsp.dat[DAT_CLAEN];           //���״̬
                KeyCmd.req.dat[DAT_TEM_PRE] = KeyCmd.rsp.dat[DAT_TEM_PRE];     //ԡ��ˮ��
                KeyCmd.req.dat[DAT_MAS_TIME] = KeyCmd.rsp.dat[DAT_MAS_TIME];     //��Ħʱ��
            }
            break;
        }

    }
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    delay_ms(5);
    send_dat(&KeyCmd.req, BUF_SIZE);
    KeyCmd.req.dat[DAT_FUN_CMD]=0;          //�幦����
    KeyCmd.req.dat[25]=0;
    if(bak_ok_flg == 1)//�ɹ�����
    {
        bak_ok_flg=0;
        nv_write(0, 0,0x55);
        addr.word+=0x08;      //������ֹ��ַ
        nv_write(0, 1,addr.byte[1]);
        nv_write(0, 2,addr.byte[0]);
        #asm
            ljmp BOOT_START
        #endasm
    }
}
/*****************************************************************************
 �� �� ��  : set_temp_val_dec
 ��������  : �¶ȼ�ֵ����
 �������  : uint8 val
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;                              // �����룺ˮ��ͷ��ˮ�¶ȸı�
    KeyCmd.req.dat[DAT_VALVE]=0x00;
    KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // �¶ȸ�
    KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // �¶ȵ�
    show_tempture( ShowPar.temp_val);
    dbg("temp:%d\r\n",ShowPar.temp_val);
}

/*****************************************************************************
 �� �� ��  : set_temp_val_inc
 ��������  : �¶ȼ�ֵ����
 �������  : uint8 val
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��11��10��
    ��    ��   : zgj
    �޸�����   : �����ɺ���

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
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;        // �����룺ˮ��ͷ��ˮ�¶ȸı�
    KeyCmd.req.dat[DAT_VALVE]=0x00;
    KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // �¶ȸ�
    KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // �¶ȵ�
    show_tempture( ShowPar.temp_val);
    dbg("temp:%d\r\n",ShowPar.temp_val);
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
    ShowPar.lamp_gear = LAMP_PINK;
    ShowPar.water_gear = MASSAGE_GEAR_ON3;
    ShowPar.air_gear = MASSAGE_GEAR_ON3;
    /* END:   Added by zgj, 2018/1/5 */
    KeyCmd.req.sta_num1 = 0x02;
    KeyCmd.req.sta_num2  = 0xA3;
    ShowPar.temp_val = 380;
    KeyCmd.req.dat[DAT_ADDR]  = 0x01;
    KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // �¶ȸ�
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


