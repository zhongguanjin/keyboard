
#include "task_main.h"
#include "uart.h"
#include "stdio.h"
#include	<string.h>

volatile uint8  work_state ;
static uint16 min_time =0;
static uint8  min_cnt;                  //
static uint16 switch_time_count = 0;    //on,off���¶��л���ʱ�����
static uint16 tim_cnt = 0;              //��ˮʱ���¶Ⱥ�Ԥ���¶���ʾ��ʱ�����


uint8 lcd_flag = 0;   //lcd�����Թرյ�״̬��־




//��������
uint8   KeyPressDown=0x00; //������Ǵ��� ��һ����Ч���������0
uint8   CurrReadKey;  //��¼����KeyScan()��ȡ��IO�ڼ�ֵ
uint8   LastKey=0x00;    //���������������

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

#define SIZE 3
static uint8 key_arry[SIZE];
static int top = -1;

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

/* ɾ�� */
void del(uint8 value)
{
    if(top == -1)  //��
    {
        return ;
    }
    if(key_arry[top] == value) //��ջ��Ԫ��
    {
        key_arry[top] = 0; //����
        top -= 1;
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
    //show_temp_actul();
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
    Button.id = CurrReadKey;
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
    id = Button.id&0Xff;
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
        case LOCK_VALVE:
        {
            static uint16 count =0;
            if((count++)>=300)
            {
                count = 0;
                LOCK_EventHandler();
            }
           break;
        }
        default:
        {
            break;
        }
    }
    IDLE_EventHandler();
}


void TAP_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&TAP_VALVE) //��һ�ΰ���
         {
             KeyPressDown = 0;
            if(lcd_flag == 1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return;
            }
            min_time = 0;
            min_cnt = 0;
            switch_time_count = 0;      //on /off ���ʱ��Ҫ��0
            ShowPar.tap_state ^= 0x01;
            if(ShowPar.tap_state == STATE_ON)
            {
                ShowPar.shower_state = STATE_OFF;
                ShowPar.drain_state = STATE_OFF;
                LED_TAP_ON;
                LED_SHOWER_OFF;
                LED_DRAIN_OFF;
                add(TAP_VALVE);
            }
            else
            {
               LED_TAP_OFF;
               del(TAP_VALVE);
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;  // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_STATE] = ((ShowPar.val&0x01)<<5)+((ShowPar.val&0x02)<<3);
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.tap_state);
        }
    }
}

void SHOWER_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&SHOWER_VALVE)  //��һ�δ���
        {
            KeyPressDown = 0;
            if(lcd_flag == 1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return;
            }
            min_time = 0;
            min_cnt = 0;
            switch_time_count = 0;
            ShowPar.shower_state ^= 0x01;
            if( ShowPar.shower_state == STATE_ON)
            {
                ShowPar.tap_state = STATE_OFF;
                LED_SHOWER_ON;
                LED_TAP_OFF;
                add(SHOWER_VALVE);
            }
            else
            {
               LED_SHOWER_OFF;
               del(SHOWER_VALVE);
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;            // �����룺��ˮ���ظı�
            KeyCmd.req.dat[DAT_STATE] = ((ShowPar.val&0x01)<<5)+((ShowPar.val&0x02)<<3);
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.shower_state);
        }
    }
}

void DRAIN_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&DRAIN_VALVE)  //��һ�δ���
        {
            KeyPressDown = 0;
            if(lcd_flag == 1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            min_time = 0;
            switch_time_count = 0;
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
            KeyCmd.req.dat[DAT_DRAINAGE] = (ShowPar.val&0x04)<<5;
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.drain_state);
        }
    }
}

void INC_EventHandler(void)
{
    static uint8 time_cnt = 0;
    if(work_state == WORK_STATE_IDLE)
    {
        LED_INC_ON;
        ShowPar.switch_flg = STATE_ON;
        if(lcd_flag == 1)
        {
            lcd_flag = 0;
            show_tempture( ShowPar.temp_val);
            return;
        }
        tim_cnt = 0;
        if((ShowPar.tap_state == OFF)&&(ShowPar.shower_state==OFF)
                                    &&(ShowPar.lamp_state == OFF)) //Ĭ�Ͼ͵��¶�
            {
                time_cnt++;
                if(KeyPressDown&INC_VALVE)  //��һ�δ���
                {
                    KeyPressDown = 0;
                    time_cnt = 0;
                    set_temp_val_inc(5);
                }
                else if((LastKey&INC_VALVE)&&(time_cnt>=50)) //��������500MS
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
            }
        if((ShowPar.tap_state == ON&&key_arry[top]==TAP_VALVE)
            ||(ShowPar.shower_state==ON&&key_arry[top]==SHOWER_VALVE)) //��ͷ�������򿪾͵�
        {
            time_cnt++;
            if(KeyPressDown&INC_VALVE)  //��һ�δ���
            {
                KeyPressDown = 0;
                set_temp_val_inc(5);
            }
            else if((LastKey&INC_VALVE)&&(time_cnt>=50)) //��������500MS
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
        }
        if((key_arry[top]==LAMP_VALVE)&&(ShowPar.lamp_state == ON))
        {
            if(KeyPressDown&INC_VALVE)  //��һ�δ���
            {
                KeyPressDown = 0;
                if(ShowPar.lamp_gear == LAMP_CYCLE) // �����ѭ��״̬,���¾Ͷ�ɫ
                {
                  ShowPar.lamp_gear = LAMP_STOP;
                }
                else
                {
                   ShowPar.lamp_gear++;
                   if(ShowPar.lamp_gear >LAMP_WHITE)
                   {
                     ShowPar.lamp_gear = LAMP_RED;
                   }
                }
                KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LED;  // ������:06
                KeyCmd.req.dat[DAT_COLOUR] = ShowPar.lamp_gear;
            }
         }
    }
}

void DEC_EventHandler(void)
{
    static uint8 time_cnt = 0;
   if(work_state == WORK_STATE_IDLE)
    {
        LED_DEC_ON;
        ShowPar.switch_flg = STATE_ON;
        if(lcd_flag == 1)
        {
            lcd_flag = 0;
            show_tempture( ShowPar.temp_val);
            return;
        }
        tim_cnt = 0;
        if((ShowPar.tap_state == OFF)&&(ShowPar.shower_state==OFF)
                                    &&(ShowPar.lamp_state == OFF)) //Ĭ�Ͼ͵��¶�
        {
            time_cnt++;
            if(KeyPressDown&DEC_VALVE)  //��һ�δ���
            {
                KeyPressDown = 0;
                time_cnt = 0;
                set_temp_val_dec(5);
            }
            else if((LastKey&DEC_VALVE)&&(time_cnt>=50)) //��������
            {
                 time_cnt = 0;
                 set_temp_val_dec(10);
            }
        }
        if((ShowPar.tap_state == ON&&key_arry[top]==TAP_VALVE)
            ||(ShowPar.shower_state==ON&&key_arry[top]==SHOWER_VALVE)) //��ͷ�������򿪾͵�
        {
            time_cnt++;
            if(KeyPressDown&DEC_VALVE)  //��һ�δ���
            {
                KeyPressDown = 0;
                time_cnt = 0;
                set_temp_val_dec(5);
            }
            else if((LastKey&DEC_VALVE)&&(time_cnt>=50)) //��������
            {
                 time_cnt = 0;
                 set_temp_val_dec(10);
            }
        }
        if((key_arry[top] == LAMP_VALVE)&&(ShowPar.lamp_state == ON)) //�ƹ��(ON)
        {
            if(KeyPressDown&DEC_VALVE)  //��һ�δ���
            {
                KeyPressDown = 0;
                if(ShowPar.lamp_gear == LAMP_CYCLE) // �����ѭ��״̬,���¾Ͷ�ɫ
                {
                    ShowPar.lamp_gear = LAMP_STOP;
                }
                else
                {
                    if(ShowPar.lamp_gear > LAMP_CYCLE)
                    {
                        ShowPar.lamp_gear=ShowPar.lamp_gear-2;
                    }
                    else
                    {
                         ShowPar.lamp_gear--;
                         if(ShowPar.lamp_gear < LAMP_RED)
                         {
                             ShowPar.lamp_gear =LAMP_WHITE;
                         }
                    }
               }
               KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LED;  // ������:06
               KeyCmd.req.dat[DAT_COLOUR]=ShowPar.lamp_gear;
            }
        }
    }
}

void AIR_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown &AIR_VALVE)
        {
            KeyPressDown = 0;
            min_time = 0;  //˯��ʱ������
            switch_time_count = 0;
            if(lcd_flag ==1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.air_state ^= 0x01;
            if( ShowPar.air_state == STATE_ON)
            {
                LED_AIR_ON;
            }
            else
            {
               LED_AIR_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // �����룺07
            KeyCmd.req.dat[DAT_MASSAGE] = ShowPar.air_state+(ShowPar.water_state<<1);
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.air_state);
        }
    }

}

void WATER_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&WATER_VALVE)
        {
            KeyPressDown = 0;
            min_time = 0;  //˯��ʱ������
            switch_time_count = 0;
            if(lcd_flag ==1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.water_state ^= 0x01;
            if( ShowPar.water_state == STATE_ON)
            {
                LED_WATER_ON;
            }
            else
            {
               LED_WATER_OFF;
            }
            KeyCmd.req.dat[DAT_FUN_CMD]= FUN_MASSAGE;            // �����룺03
            KeyCmd.req.dat[DAT_MASSAGE] = ShowPar.air_state+(ShowPar.water_state<<1);
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.water_state);
        }
    }

}

void LAMP_EventHandler(void)
{
    if(work_state == WORK_STATE_IDLE)
    {
        if(KeyPressDown&LAMP_VALVE)
        {
            KeyPressDown = 0;
            min_time = 0;
            switch_time_count = 0;
            if(lcd_flag ==1)
            {
                lcd_flag = 0;
                show_tempture( ShowPar.temp_val);
                return ;
            }
            ShowPar.lamp_state ^= 0x01;
            if(ShowPar.lamp_state == STATE_ON) //�򿪵ƣ������ж�����+ -����
            {
                LED_LAMP_ON;
               KeyCmd.req.dat[DAT_COLOUR] = LAMP_CYCLE; //��Ĭ��ѭ����
               ShowPar.lamp_gear = LAMP_CYCLE;
               add(LAMP_VALVE);
            }
            else  //�ص�off
            {
                LED_LAMP_OFF;
                KeyCmd.req.dat[DAT_COLOUR] = LAMP_OFF;
                ShowPar.lamp_gear = LAMP_OFF;
                del(LAMP_VALVE);
            }
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_LED;  // ������:04
            ShowPar.switch_flg = STATE_ON;
            show_state(ShowPar.lamp_state);
        }
    }
}



void LOCK_EventHandler(void) //10ms
{
    if(work_state == WORK_STATE_IDLE)
    {
        work_state = WORK_STATE_LOCK;
        ShowPar.shower_state = OFF;
        ShowPar.tap_state = OFF;
        LED_TAP_OFF;
        KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;            // �����룺��ˮ���ظı�
        KeyCmd.req.dat[DAT_STATE] = ((ShowPar.val&0x01)<<5)+((ShowPar.val&0x02)<<3);
    }
    else if(work_state == WORK_STATE_LOCK)
    {
       show_tempture( ShowPar.temp_val);
       work_state = WORK_STATE_IDLE;
    }
}

void IDLE_EventHandler(void) //10ms
{
    if( work_state == WORK_STATE_LOCK)
    {
        if((min_time%100)==40)//400ms
        {
            show_lock();
        }
        if((min_time%100)==90)//900ms
        {
           show_clean();
        }
    }
    // ����չ�����ˮ���򿪺�6s���Զ��ر� 2017-11-22
    /*
    static uint16 time_drain =0;
    if(ShowPar.drain_state == ON)
    {
        if((time_drain++)>=600) //6sʱ�䵽
        {
            time_drain = 0;
            ShowPar.drain_state = OFF;
            LED_DRAIN_OFF;
            KeyCmd.req.dat[DAT_FUN_CMD] =FUN_DRAINAGE;
            KeyCmd.req.dat[DAT_DRAINAGE] = (ShowPar.val&0x04)<<5;
        }
    }else
    {
        time_drain = 0;
    }
    */
    if(min_time++ >=6000) // 1min��ʱ�䵽  ʱ��10ms
    {
       min_time = 0;
       min_cnt++;
       //����չ��Ʒȡ��������� 2017-11-22

       if(work_state == WORK_STATE_IDLE)
      {
         if(ShowPar.val == OFF)
         {
            show_clean();
            lcd_flag = 1;  //��λ�رձ�־λ
         }
      }

    }
    if(work_state == WORK_STATE_IDLE)
    {
        if(min_cnt == 30)  //�¶ȱ���30min�� ���л���38��
        {
            if((ShowPar.shower_state== OFF)&&(ShowPar.tap_state == OFF)) //��ͷ���ٽ�ˮʱ
            {
                ShowPar.temp_val = 380;
                KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;        // �����룺ˮ��ͷ��ˮ�¶ȸı�
                KeyCmd.req.dat[DAT_TEMP_HIGH] = ShowPar.temp_val >> 8;            // �¶ȸ�
                KeyCmd.req.dat[DAT_TEMP_LOW]  =  ShowPar.temp_val;
            }
        }
        if(min_cnt == 60) //60min�ӵ��Զ��ر���ͷ������
        {
            if((ShowPar.shower_state== ON)||(ShowPar.tap_state == ON)) //��ͷ��ˮʱ
            {
                KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;// �����룺��ˮ���ظı�
                ShowPar.tap_state = OFF;
                ShowPar.shower_state = OFF;
                KeyCmd.req.dat[DAT_STATE] = ((ShowPar.val&0x01)<<5)+((ShowPar.val&0x02)<<3);
                show_tempture( ShowPar.temp_val);
            }
            min_cnt = 0;
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
    if(  1 == ShowPar.switch_flg  )
    {
         switch_time_count ++ ;
         if( switch_time_count >= 15 ) // 2sʱ�䵽
         {
            switch_time_count = 0;
            ShowPar.switch_flg = 0;
            LED_DEC_OFF;
            LED_INC_OFF;
            show_tempture( ShowPar.temp_val);
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
            uint16 pre_tem;
            //pre_tem = KeyCmd.req.dat[DAT_TEM_OUT]*10;
            if((pre_tem < ShowPar.temp_val-20)||((pre_tem > ShowPar.temp_val+20))) //�ﲻ��Ԥ���¶�
            {
                if(tim_cnt<300)
                {
                    tim_cnt++;
                }
                if(tim_cnt >= 200)// 20s
                {
                    static uint8 cnt =0;
                    if((cnt%24)==0)
                    {
                      show_tempture(pre_tem);
                    }
                    if((cnt%24)==10)
                    {
                        show_clean();
                    }
                    if((cnt%24)==12)
                    {
                      show_tempture( ShowPar.temp_val);
                    }
                    if((cnt%24)==22)
                    {
                        show_clean();
                    }
                   cnt++;
                   if(cnt>=24)
                   {
                        cnt=0;
                   }
                }
            }
            else
            {
                 tim_cnt = 0;
            }
        }
        else
        {
             tim_cnt = 0;
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
 �� �� ��  : key_state
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
void key_state(void)
{
/*
    //ˮ��ͷ������״̬����
    if(KeyCmd.rsp.dat[DAT_INFO]!=KeyCmd.req.dat[DAT_INFO])
    {
        if((KeyCmd.rsp.dat[DAT_INFO]&0x10)== 0x10) //tap �� 0010 0000
        {
            ShowPar.tap_state = STATE_ON;
        }
        else
        {
            ShowPar.tap_state = STATE_OFF;
        }
        if((KeyCmd.rsp.dat[DAT_INFO]&0x20) == 0x20) //shower �� 0001 0000
        {
            ShowPar.shower_state = STATE_ON;
        }
        else
        {
            ShowPar.shower_state = STATE_OFF;
        }
        if((KeyCmd.rsp.dat[DAT_INFO]&0x40)== 0X40) //drain ��
        {
            ShowPar.drain_state = STATE_ON;
        }
        else
        {
            ShowPar.drain_state = STATE_OFF;
        }
        KeyCmd.req.dat[DAT_INFO] = KeyCmd.rsp.dat[DAT_INFO];
        show_tempture( ShowPar.temp_val);
    }
    */
    // ������ӻ��¶Ȳ�һ�£��ӻ�������¶�
    if((KeyCmd.rsp.dat[DAT_TEMP_HIGH]!=KeyCmd.req.dat[DAT_TEMP_HIGH])
        ||(KeyCmd.rsp.dat[DAT_TEMP_LOW]!=KeyCmd.req.dat[DAT_TEMP_LOW]))
    {
        uint16 tem = 0;
        tem = (KeyCmd.rsp.dat[DAT_TEMP_HIGH]<<8)+ KeyCmd.rsp.dat[DAT_TEMP_LOW];
        if((tem >= TEMPERATURE_MIN)&&(tem <= TEMPERATURE_MAX))
        {
            KeyCmd.req.dat[DAT_TEMP_HIGH]=KeyCmd.rsp.dat[DAT_TEMP_HIGH];
            KeyCmd.req.dat[DAT_TEMP_LOW] =KeyCmd.rsp.dat[DAT_TEMP_LOW];
            ShowPar.temp_val= (KeyCmd.req.dat[DAT_TEMP_HIGH]<<8)+ KeyCmd.req.dat[DAT_TEMP_LOW];
            if(work_state == WORK_STATE_IDLE)
            {
                if(lcd_flag == 0) //����״̬����ʾ����
                {
                    show_tempture( ShowPar.temp_val);
                    min_time = 0; //�������µ�һ����
                }
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
    if(work_state != WORK_STATE_LOCK)
    {
        if((Send_Buf[DAT_TEMP_HIGH+3] == KeyCmd.req.dat[DAT_TEMP_HIGH])
            && (Send_Buf[DAT_TEMP_LOW+3] == KeyCmd.req.dat[DAT_TEMP_LOW]))  //��ǰû�а�������
        {
           key_state();
        }
    }
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.spare1, crc_len);
    send_dat(&KeyCmd.req, BUF_SIZE);
    memcpy(&Send_Buf,&KeyCmd.req,sizeof(KeyCmd.req));
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
    Recv_Buf[Recv_Len] = ui8Data;
    if(Recv_Buf[0]==0x32)
    {
        Recv_Len++;
        if(Recv_Len >= BUF_SIZE) //���յ�16byte������
        {
            if((Recv_Buf[15]== 0x34)&&(Recv_Buf[2] == 0x01)&&(Recv_Buf[1] == 0xB3))
            {
                if(CRC8_SUM(&Recv_Buf[1], crc_len) == Recv_Buf[14])
                {
                    memcpy(&KeyCmd.rsp,&Recv_Buf,sizeof(KeyCmd.rsp));
                    frame_ok_fag =1;
                    Recv_Len = 0;
                }
                else
                {
                    memset(&Recv_Buf,0,sizeof(Recv_Buf));
                    Recv_Len = 0;
                }
            }
            else  //��������ߵ�ַ����
            {
                memset(&Recv_Buf,0,sizeof(Recv_Buf));
                Recv_Len = 0;
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
    min_time = 0;
    min_cnt = 0;
    if( ShowPar.temp_val > TEMPERATURE_MIN)
    {
        ShowPar.temp_val -= val ;
        if(ShowPar.temp_val < TEMPERATURE_MIN)
        {
            ShowPar.temp_val = TEMPERATURE_MIN;
        }
    }
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;                              // �����룺ˮ��ͷ��ˮ�¶ȸı�
    KeyCmd.req.dat[DAT_TEMP_HIGH] = ShowPar.temp_val >> 8;            // �¶ȸ�
    KeyCmd.req.dat[DAT_TEMP_LOW] = ShowPar.temp_val;                 // �¶ȵ�
    show_tempture( ShowPar.temp_val);
#if (TEST)
    M485_EN_H;
    uart_send_str("DEC\r\n");
#endif
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
    min_time = 0;
    min_cnt = 0;
    if( ShowPar.temp_val < TEMPERATURE_MAX)
    {
        ShowPar.temp_val += val ;
        if(ShowPar.temp_val > TEMPERATURE_MAX)
        {
            ShowPar.temp_val = TEMPERATURE_MAX;
        }
    }
    KeyCmd.req.dat[DAT_FUN_CMD]= FUN_CHANNEL_SWITCH;        // �����룺ˮ��ͷ��ˮ�¶ȸı�
    KeyCmd.req.dat[DAT_TEMP_HIGH] = ShowPar.temp_val >> 8;            // �¶ȸ�
    KeyCmd.req.dat[DAT_TEMP_LOW] = ShowPar.temp_val;                 // �¶ȵ�
    show_tempture( ShowPar.temp_val);
#if (TEST)
    M485_EN_H;
    uart_send_str("INC\r\n");
#endif
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
    KeyCmd.req.sta_num = 0x32;
    KeyCmd.req.spare1  = 0x3B;
    ShowPar.temp_val = 380;
    ShowPar.air_gear = MASSAGE_GEAR_ON3;
    ShowPar.wat_gear = MASSAGE_GEAR_ON3;
    KeyCmd.req.dat[DAT_TEMP_HIGH] = ShowPar.temp_val >> 8;            // �¶ȸ�
    KeyCmd.req.dat[DAT_TEMP_LOW]  =  ShowPar.temp_val;
    KeyCmd.req.dat[DAT_FLOW_GEAR] = 0x64;
    KeyCmd.req.dat[DAT_PER_TEMP]  = 0x26;
    KeyCmd.req.dev_addr  = 0x01;
    KeyCmd.req.crc_num = CRC8_SUM(&KeyCmd.req.spare1, crc_len);
    KeyCmd.req.end_num = 0x34;
    memcpy(&Send_Buf,&KeyCmd.req,sizeof(KeyCmd.req));
    show_tempture( ShowPar.temp_val);
    work_state = WORK_STATE_IDLE;
    M485_EN_L;
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

