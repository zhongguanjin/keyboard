
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
}tTime_T;

tTime_T Time_t;


volatile uint8  key_switch_fag;          //���������л���־
volatile uint8  incdec_fag;
static uint8  flash_cnt;        //�����¶� ������˸�Ĵ���

volatile uint8 massage_dat =0;
//��������
uint8   KeyPressDown=0x00; //������Ǵ��� ��һ����Ч���������0
uint8   CurrReadKey;  //��¼����KeyScan()��ȡ��IO�ڼ�ֵ
uint8   LastKey=0x00;    //���������������

#define SIZE 4
static uint8 key_arry[SIZE]; //������ջ����
static int top = 0;
static uint16 time_clean=0;
uint8 clean_state;        //clean ����

//��������
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
void show_temp_flash(void);
void TaskClean();


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
void TaskShow(void)
{
    show_work();
    show_temp_flash();
    if(key_switch_fag ==0)
    {
        show_temp_actul();
	}
    if(work_state == WORK_STATE_CLEAN)
    {
       TaskClean();
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
void TaskClean()
{
    switch ( clean_state )
    {
        case STATE_1:
            {
                static uint16 state1_time = 0;
                if((state1_time++)==10)  // 1s ����
                {
                    state1_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x01;
                    clean_state = STATE_2;
                }
            }
            break;
        case STATE_2 :
            {
                static uint16 state2_time = 0;
                if((state2_time++)==1800)// 3min
                {
                    state2_time = 0;
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x02;
                    clean_state = STATE_3;
                }
            }
            break;
        case STATE_3 :
            {
                static uint16 state3_time = 0;
                if((KeyCmd.req.dat[DAT_LIQUID]&0x01)!=0x01) //���ڵ�Һλ
                {
                    if((state3_time++)==4800)  //8min
                    {
                        state3_time = 0;
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                        KeyCmd.req.dat[DAT_VALVE] = 0x03;
                        clean_state = STATE_4;
                    }
                }
            }
            break;
        case STATE_4 :
            {
                static uint16 state4_time = 0;
                if((KeyCmd.req.dat[DAT_LIQUID]&0x01)==0x01) //���ڵ�Һλ
                {
                    if((state4_time++)==10)  // 1s
                    {
                        state4_time = 0;
                        KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                        KeyCmd.req.dat[DAT_VALVE] = 0x04;
                        clean_state = STATE_5;
                    }
                }
            }
            break;
        case STATE_5 :
            {
                static uint16 state5_time = 0;
                if((state5_time++)==1200)// 2min
                {
                    KeyCmd.req.dat[DAT_FUN_CMD] =FUN_CLEAN;
                    KeyCmd.req.dat[DAT_VALVE] = 0x05;
                }
                if(state5_time ==1220)
                {
                    state5_time = 0;
                    work_state =WORK_STATE_IDLE;
                    KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //������
                    KeyCmd.req.dat[DAT_VALVE] =0x00;
                    show_tempture(ShowPar.temp_val);
                    LED_WATER_OFF;
                    LED_INC_OFF;
                    clean_state = STATE_0;
                }
            }
            break;
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
		    //if(ShowPar.val == 0x00) //���й��ܶ��ر�
            if((KeyCmd.req.dat[DAT_LIQUID]&0x01) == 0x01) //��Һλ
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
        default:
        {
            count = 0;
            Flg.lock_flg =0;
            time_clean = 0;
            break;
        }
    }
    IDLE_EventHandler();
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
    key_switch_fag =1;
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
void   time_cnt_del( uint8 id)
{
    Time_t.sleep = 0;
    Time_t.temp38 = 0;
    if((id==TAP_VALVE)||(id==SHOWER_VALVE))//��ͷ����
    {
        Time_t.switch_cnt = 0;                  //on /off ���ʱ��Ҫ��0
        Time_t.temp_switch =0;
        Flg.temp_disreach_flg =0;
    }
    if((id==INC_VALVE)||(id==DEC_VALVE))    //+,-
    {
        if(key_switch_fag==0) //����ģʽ��Ҫ���temp_switch��ʱ��
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
        ShowPar.switch_flg = 0;
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
    if(work_state == WORK_STATE_IDLE)
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
    //if((work_state == WORK_STATE_IDLE)&&(Flg.err_f1_flg == 0))
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&TAP_VALVE) //��һ�ΰ���
         {
            KeyPressDown = 0;
            if(Flg.lcd_sleep_flg == 1)
            {
                show_awaken();
                return;
            }
            ShowPar.tap_state ^= 0x01;
            if(ShowPar.tap_state == STATE_ON)
            {
                ShowPar.shower_state = STATE_OFF;
                //ShowPar.drain_state = STATE_OFF;
                LED_TAP_ON;
                LED_SHOWER_OFF;
                //LED_DRAIN_OFF;
            }
            else
            {
               LED_TAP_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;  // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_VALVE] =  ShowPar.val&0x03; //������ ��ͷ״̬
            time_cnt_del(TAP_VALVE);
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.tap_state);
            dbg("tap,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
    /*
    else if((Flg.err_f1_flg == 1)&&(KeyCmd.req.dat[DAT_ERR_NUM]&0x80))  //f1 err ʱ
    {
        time_cnt_del(TAP_VALVE);
        ShowPar.tap_state = STATE_ON;
        Flg.err_f1_flg =0;
        ShowPar.temp_val = 385;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // �����룺��ˮ���ظı�
        KeyCmd.req.dat[DAT_STATE] = ShowPar.val&0x03;
        KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // �¶ȸ�
        KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
        show_tempture(ShowPar.temp_val);
    }
    */
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
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.shower_state);
            dbg("shower,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
        }
    }
    /*
    else if((Flg.err_f1_flg == 1)&&(KeyCmd.req.dat[DAT_ERR_NUM]&0x80))  //f1 err ʱ
    {
        time_cnt_del(SHOWER_VALVE);
        ShowPar.shower_state = STATE_ON;
        Flg.err_f1_flg =0;
        ShowPar.temp_val = 380;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_INFLOW;            // �����룺��ˮ���ظı�
        KeyCmd.req.dat[DAT_STATE] = ShowPar.val&0x03;
        KeyCmd.req.dat[DAT_TEMP_H] = ShowPar.temp_val >> 8;            // �¶ȸ�
        KeyCmd.req.dat[DAT_TEMP_L]  =  ShowPar.temp_val;
        show_tempture(ShowPar.temp_val);
    }
    */
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
            ShowPar.switch_flg = STATE_ON;
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
                       dbg("lamp %x\r\n", KeyCmd.req.dat[DAT_VALVE]);
                    }
                    break;
                }
            case WATER_VALVE:
                {
                    if(KeyPressDown&DEC_VALVE)  //��һ�δ���
                    {
                        KeyPressDown = 0;
                        ShowPar.water_gear--;
                        if(ShowPar.water_gear <MASSAGE_GEAR_ON1)
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
                        ShowPar.air_gear--;
                        if(ShowPar.air_gear <MASSAGE_GEAR_ON1)
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
                   key_switch_fag = 0;
                   if(Flg.err_flg ==1) //�д���
                   {
                        write_err_num(KeyCmd.req.dat[DAT_ERR_NUM]&0x0F);
                   }
                   else
                   {
                        show_tempture( ShowPar.temp_val);
                   }
               }
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
                    key_switch_fag = 0;
                    if(Flg.err_flg ==1) //�д���
                    {
                         write_err_num(KeyCmd.req.dat[DAT_ERR_NUM]&0x0F);
                    }
                    else
                    {
                         show_tempture( ShowPar.temp_val);
                    }
                }
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
    		LED_WATER_OFF;
    		LED_INC_OFF;
    		work_state =WORK_STATE_IDLE;
    		KeyCmd.req.dat[DAT_FUN_CMD] = FUN_CLEAN; //������
    		KeyCmd.req.dat[DAT_VALVE] =0x00;
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
               //ShowPar.lamp_gear = LAMP_CYCLE;
               add(LAMP_VALVE);
               key_adjust(key_arry[top],ShowPar.lamp_gear);
            }
            else  //�ص�off
            {
                LED_LAMP_OFF;
                KeyCmd.req.dat[DAT_VALVE] = LAMP_OFF;
               // ShowPar.lamp_gear = LAMP_OFF;
               del(LAMP_VALVE);
               if(key_arry[top]==0)  //Ϊ��
               {
                   Time_t.key_adj = 0;
                   key_switch_fag = 0;
                   if(Flg.err_flg ==1) //�д���
                   {
                        write_err_num(KeyCmd.req.dat[DAT_ERR_NUM]&0x0F);
                   }
                   else
                   {
                        show_tempture( ShowPar.temp_val);
                   }
               }
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
        Flg.lock_flg =1;
        work_state = WORK_STATE_LOCK;
        //ShowPar.shower_state = OFF;
        //ShowPar.tap_state = OFF;
        //LED_TAP_OFF;
        //LED_SHOWER_OFF;
        if(ShowPar.drain_state == ON)
        {
            LED_DRAIN_OFF;
        }
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // �����룺��ˮ���ظı�
        KeyCmd.req.dat[DAT_VALVE] = 0x01;
        //KeyCmd.req.dat[DAT_LOCK] = 0x01;
        dbg("idle -> lock,%x\r\n",KeyCmd.req.dat[DAT_VALVE]);
    }
    if((work_state == WORK_STATE_LOCK)&&(Flg.lock_flg ==0))
    {
       Flg.lock_flg =1;
       show_awaken();
       work_state = WORK_STATE_IDLE;
       KeyCmd.req.dat[DAT_FUN_CMD]= FUN_LOCK;            // �����룺��ˮ���ظı�
       KeyCmd.req.dat[DAT_VALVE] = 0x00;
       //KeyCmd.req.dat[DAT_LOCK] = 0x00;
       dbg("lock -> idle\r\n");
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
    if( work_state == WORK_STATE_LOCK)
    {
        if((Time_t.sleep%100)==40)//400ms
        {
            show_lock();
        }
        if((Time_t.sleep%100)==90)//900ms
        {
           show_sleep(OFF);
        }
    }
    if(work_state == WORK_STATE_IDLE)
    {
        if(Time_t.temp38 >= 30)  //�¶ȱ���30min�� ���л���38��
        {
            //if((ShowPar.shower_state== OFF)&&(ShowPar.tap_state == OFF)) //��ͷ���ٽ�ˮʱ
            {
                ShowPar.temp_val = 380;
                KeyCmd.req.dat[DAT_FUN_CMD]= FUN_TEMP;        // �����룺ˮ��ͷ��ˮ�¶ȸı�
                KeyCmd.req.dat[DAT_VALVE]=0x00;
                KeyCmd.req.dat[DAT_TEMP_H] = (ShowPar.temp_val&0xff00) >> 8;            // �¶ȸ�
                KeyCmd.req.dat[DAT_TEMP_L] = ShowPar.temp_val&0x00ff;                 // �¶ȵ�
                //show_tempture( ShowPar.temp_val);
                dbg("30min temp->38\r\n");
            }
            Time_t.temp38 = 0;
        }
        if((KeyCmd.req.dat[DAT_ERR_NUM]&0x0F) != 0x00)  //�д���
        {
            if((key_switch_fag==0)&&(ShowPar.switch_flg==0)&&(incdec_fag == 0))
            {
                Flg.err_flg =1;
                write_err_num(KeyCmd.req.dat[DAT_ERR_NUM]&0x0F);
            }
        }
        else
        {
            if(Flg.err_flg ==1)
            {
                Flg.err_flg =0;
                show_tempture(ShowPar.temp_val);
            }
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
       }
       if(work_state == WORK_STATE_IDLE)
       {
            dbg("1 min get\r\n");
            if((ShowPar.val&0xfb)== OFF)
            {
                dbg("sleep\r\n");
                if(ShowPar.drain_state == ON)
                {
                    LED_DRAIN_OFF;
                }
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
			LED_INC_ON;
			LED_WATER_ON;
			work_state = WORK_STATE_CLEAN;
			show_clean();                           //�����ʾ---
			clean_state =STATE_1;
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
    if( 1 == ShowPar.switch_flg)
    {
         Time_t.switch_cnt ++ ;
         if( Time_t.switch_cnt >= 20 ) // 2sʱ�䵽
         {
            Time_t.switch_cnt = 0;
            ShowPar.switch_flg = 0;
            show_tempture( ShowPar.temp_val);
         }
    }
    if(1 == key_switch_fag)
    {
        Time_t.key_adj++;
        if(Time_t.key_adj>=50) // 5s
        {
            Time_t.key_adj = 0;
            key_switch_fag = 0;
            show_tempture(ShowPar.temp_val);
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
    if(work_state == WORK_STATE_IDLE)
    {
        if((ShowPar.tap_state == ON)||(ShowPar.shower_state == ON )) //��ˮ״̬
        {
            uint16 pre_tem=0;
            static uint8 temp_count =0;
            pre_tem = KeyCmd.req.dat[DAT_TEM_OUT]*10;
            if((pre_tem < ShowPar.temp_val-20)||((pre_tem > ShowPar.temp_val+20))) //�ﲻ��Ԥ���¶�
            {
                temp_count = 0;
                if(Time_t.temp_switch<1000)
                {
                    Time_t.temp_switch++;
                }
                 if((Time_t.temp_switch >= 200)&&(Flg.temp_disreach_flg == 0))// 20s
                 {
                     static uint8 cnt =0;
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
                     static uint8 cnt1 =0;
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
             else
             {
                  Flg.temp_disreach_flg =0;
                  Time_t.temp_switch = 0;
                  if((temp_count++)>=5)
                  {
                      temp_count = 0;
                      show_tempture( ShowPar.temp_val);
                  }
             }
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
 �� �� ��  : key_state_update
 ��������  : ״̬����ͬ������
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
void key_state_update(void)
{
    /* BEGIN: Added by zgj, 2018/1/15 */
    massage_dat = (ShowPar.water_gear<<2)
                 +(ShowPar.air_gear<<5)+((ShowPar.val&0x60)>>5);
    if((KeyCmd.rsp.dat[DAT_MASSAGE]!=KeyCmd.req.dat[DAT_MASSAGE])
        ||(KeyCmd.rsp.dat[DAT_MASSAGE]!=massage_dat)) //��Ħ״̬����
    //if(KeyCmd.rsp.dat[DAT_MASSAGE]!=KeyCmd.req.dat[DAT_MASSAGE])
    {
        time_cnt_del(AIR_VALVE);
        ShowPar.air_gear = (KeyCmd.rsp.dat[DAT_MASSAGE]&0xE0)>>5;
        ShowPar.water_gear = (KeyCmd.rsp.dat[DAT_MASSAGE]&0x1C)>>2;
        KeyCmd.req.dat[DAT_MASSAGE] =KeyCmd.rsp.dat[DAT_MASSAGE];
        if((KeyCmd.req.dat[DAT_MASSAGE]&0x01)==0x01) //ˮ��Ħ����
        {
            ShowPar.water_state =ON;
            LED_WATER_ON;
            add(WATER_VALVE);
            key_adjust(key_arry[top],ShowPar.water_gear);
        }
        else if((KeyCmd.req.dat[DAT_MASSAGE]&0x01)==0)
        {
            ShowPar.water_state =OFF;
            if(work_state!=WORK_STATE_CLEAN)
            {
                LED_WATER_OFF;
            }
            del(WATER_VALVE);
            if((key_arry[top]==0)&&(work_state != WORK_STATE_LOCK))  //Ϊ��
            {
                Time_t.key_adj = 0;
                key_switch_fag = 0;
                show_tempture( ShowPar.temp_val);
            }
        }
        if((KeyCmd.req.dat[DAT_MASSAGE]&0x02)==0x02) //air��Ħ����
        {
            ShowPar.air_state =ON;
            LED_AIR_ON;
            add(AIR_VALVE);
            key_adjust(key_arry[top],ShowPar.air_gear);
        }
        else if((KeyCmd.req.dat[DAT_MASSAGE]&0x02)==0)
        {
            ShowPar.air_state =OFF;
            LED_AIR_OFF;
            del(AIR_VALVE);
            if((key_arry[top]==0)&&(work_state != WORK_STATE_LOCK))  //Ϊ��
            {
                Time_t.key_adj = 0;
                key_switch_fag = 0;
                show_tempture( ShowPar.temp_val);
            }
        }
        /*
        switch ( KeyCmd.req.dat[DAT_MASSAGE]&0x03)
        {
            case 0:
                {
                    ShowPar.air_state = OFF;
                    ShowPar.water_state =OFF;
                    if(work_state!=WORK_STATE_CLEAN)
                    {
                        LED_WATER_OFF;
                    }
                    LED_AIR_OFF;

                }
                break;
            case 1 :
                {
                    ShowPar.air_state = OFF;
                    ShowPar.water_state =ON;
                    LED_WATER_ON;
                    LED_AIR_OFF;
                }
                 break;
            case 2 :
                {
                    ShowPar.air_state = ON;
                    ShowPar.water_state =OFF;
                    if(work_state!=WORK_STATE_CLEAN)
                    {
                        LED_WATER_OFF;
                    }
                    LED_AIR_ON;
                }
                break;
            case 3 :
                {
                    ShowPar.air_state = ON;
                    ShowPar.water_state =ON;
                    LED_WATER_ON;
                    LED_AIR_ON;
                }
                break;
        }
        */
    }
    if((KeyCmd.rsp.dat[DAT_STATE]!=KeyCmd.req.dat[DAT_STATE])
        ||(KeyCmd.rsp.dat[DAT_STATE]!=(ShowPar.val&0x03))) //��ˮ״̬����
    {
        KeyCmd.req.dat[DAT_STATE] =KeyCmd.rsp.dat[DAT_STATE];
        time_cnt_del(TAP_VALVE);
        /*
        if(work_state == WORK_STATE_IDLE)
        {
            show_tempture(ShowPar.temp_val);
        }*/
        /* END: Deleted by zgj, 2018/3/22 */
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
    if(KeyCmd.rsp.dat[DAT_LIGHT]!=KeyCmd.req.dat[DAT_LIGHT]) //lamp״̬����
    {
        KeyCmd.req.dat[DAT_LIGHT] =KeyCmd.rsp.dat[DAT_LIGHT];
        time_cnt_del(LAMP_VALVE);
       if(KeyCmd.req.dat[DAT_LIGHT] == 0)
       {
            ShowPar.lamp_state = OFF;
            LED_LAMP_OFF;
            del(LAMP_VALVE);
            if((key_arry[top]==0)&&(work_state != WORK_STATE_LOCK))  //Ϊ��
            {
                Time_t.key_adj = 0;
                key_switch_fag = 0;
                show_tempture( ShowPar.temp_val);
            }
       }
       else
       {
           ShowPar.lamp_state = ON;
           LED_LAMP_ON;
           ShowPar.lamp_gear =KeyCmd.rsp.dat[DAT_LIGHT];
           add(LAMP_VALVE);
           key_adjust(key_arry[top],ShowPar.lamp_gear);
       }
    }
    if(KeyCmd.rsp.dat[DAT_DRAIN]!=KeyCmd.req.dat[DAT_DRAIN]) //drain״̬����
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
    if(KeyCmd.rsp.dat[DAT_LOCK]!=KeyCmd.req.dat[DAT_LOCK]) //ͯ��״̬����
    {
        KeyCmd.req.dat[DAT_LOCK] =KeyCmd.rsp.dat[DAT_LOCK];
        if(KeyCmd.req.dat[DAT_LOCK] == 1)                // lock
        {
            show_lock();
            if(ShowPar.drain_state == ON)
            {
                LED_DRAIN_OFF;
            }
            work_state =WORK_STATE_LOCK;
        }
        else
        {
            show_tempture(ShowPar.temp_val);
            work_state =WORK_STATE_IDLE;
        }
    }
    if(KeyCmd.rsp.dat[DAT_CLAEN]!=KeyCmd.req.dat[DAT_CLAEN]) //���״̬����
    {
        KeyCmd.req.dat[DAT_CLAEN] =KeyCmd.rsp.dat[DAT_CLAEN];
        if(KeyCmd.req.dat[DAT_CLAEN] == 1)                // clean
        {
            show_clean();
            work_state =WORK_STATE_CLEAN;
        }
        else
        {
            show_tempture(ShowPar.temp_val);
            work_state =WORK_STATE_IDLE;
        }
    }
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
            if((work_state == WORK_STATE_IDLE)&&(Flg.lcd_sleep_flg == 0))
            {
                show_tempture( ShowPar.temp_val);
                Time_t.sleep = 0; //�������µ�һ����
            }
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
    static uint8 state=0;
    memcpy(&KeyCmd.rsp,Recv_Buf,sizeof(KeyCmd.rsp));
    //if(work_state == WORK_STATE_IDLE)
    {
        if(0 == KeyCmd.req.dat[DAT_FUN_CMD])  //�жϹ����� �������Ƿ���ʹ��
        {
           key_state_update();
        }
    }
    KeyCmd.req.dat[DAT_ERR_NUM] =KeyCmd.rsp.dat[DAT_ERR_NUM];      //������
    KeyCmd.req.dat[DAT_LIQUID] = KeyCmd.rsp.dat[DAT_LIQUID];       //Һλ��Ϣ
    KeyCmd.req.dat[DAT_TEM_OUT] = KeyCmd.rsp.dat[DAT_TEM_OUT];     //ʵ���¶�
    KeyCmd.req.dat[DAT_KEEP_WARM] = KeyCmd.rsp.dat[DAT_KEEP_WARM];     //����״̬
    KeyCmd.req.dat[DAT_CLAEN] = KeyCmd.rsp.dat[DAT_CLAEN];     //���״̬
    KeyCmd.req.dat[DAT_TEM_PRE] = KeyCmd.rsp.dat[DAT_TEM_PRE];     //ԡ��ˮ��
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.dat[DAT_ADDR], crc_len);
    delay_ms(5);
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
    Recv_Buf[Recv_Len] = ui8Data;
    if(Recv_Buf[0]==0x02)
    {
        Recv_Len++;
        if(Recv_Len >= BUF_SIZE) //���յ�32byte������
        {
             if((Recv_Buf[31]== 0x04)&&(Recv_Buf[30]== 0x0B)&&(Recv_Buf[2] == 0x01)&&(Recv_Buf[1] == 0x3A))
             {
                 for(uint8 i=2;i<(crc_len+2);i++)
                 {
                     check_sum^=Recv_Buf[i];
                 }
                 if(check_sum == Recv_Buf[29])
                 //if(CRC8_SUM(&Recv_Buf[2], crc_len) == Recv_Buf[29])
                 {
                     Recv_Len = 0;
                     Flg.frame_ok_fag=1;
                 }
                 else
                 {
                      Recv_Len = 0;
                      Flg.frame_ok_fag=0;
                 }
             }
            if((Recv_Buf[31]!= 0x04)||(Recv_Buf[30]!= 0x0B)||(Recv_Buf[2] != 0x01)||(Recv_Buf[1] != 0x3A))  //��������ߵ�ַ����
             {
                  Recv_Len = 0;
                  Flg.frame_ok_fag = 0;
             }
        }
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


