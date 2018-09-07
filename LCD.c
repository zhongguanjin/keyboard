
#include "LCD.h"
#include "Task_Main.h"

uint8 digi_flg = 0;

uint8 tab_num[11]={
0x40,   //0
0x79,  // 1
0x24,  // 2
0x30,   // 3
0x19,  // 4
0x12,   // 5
0x02,   // 6
0x78,   //7
0x00,   //8
0x10, //9
0x7f,
};

uint8 tab_val[19]={
0xc0,   //0
0xf9,  // 1
0xa4,  // 2
0xb0,
0x99,  // 4
0x92,   // 5
0x82,   // 6
0xf8,
0x80,
0x90, //9
0xbf, // -
0xab, //11 N
0x8e, //12 F
0xff, // clear
0x8c, //14p-水按摩
0x86, //15e-qi按摩
0xc6, //16c-灯
0x98, //17 q-
0xC1, //18 u
};



uint8 digiBuf[3]; //数码管缓冲区



void led_scan(void)
{
    static uint8 digiPos = 0;

    if(digi_flg == 1) //清除小数点
    {
       LED_SEG_DAT =tab_val[digiBuf[digiPos]];
    }
    else //显示温度有小数点
    {
        LED_SEG_DAT =tab_num[digiBuf[digiPos]];
    }

    if(Flg.lcd_sleep_flg == 0)
    {
        switch(digiPos)
        {
            case 0:
            {
                LED_COM1_L;
                LED_COM2_H;
                LED_COM3_H;
                break; // 选择第一列数码管
            }
            case 1:
            {
                LED_COM1_H;
                LED_COM2_L;
                LED_COM3_H;
                break; // 选择第二列数码管
            }
            case 2:
            {
                LED_COM1_H;
                LED_COM2_H;
                LED_COM3_L;
                break; // 选择第三列数码管
            }
            default:
            {
                break;
            }
        }
    }
    digiPos++;
    if(digiPos >= 3)
    {
        digiPos = 0;
    }
}



void show_tempture( uint16 data)//温度显示
{
    uint8 dat =0;
    digi_flg = 0;
    Flg.lcd_sleep_flg = 0;
    digiBuf[0] = data/100;
    digiBuf[1] = (data%100)/10;
    dat = data%10;
    if(dat<5)
    {
       digiBuf[2] = 0;
    }
    else
    {
       digiBuf[2] = 5;
    }
}
void show_soft_version(uint16 dat)
{
    digi_flg = 0;
    Flg.lcd_sleep_flg = 0;
    digiBuf[0] = dat/100;
    if(digiBuf[0] == 0)
    {
        digiBuf[0] =10;
    }
    digiBuf[1] = (dat%100)/10;
    digiBuf[2] = dat%10;
}

void show_awaken()
{
    /*
    if(ShowPar.drain_state == ON)
    {
        LED_DRAIN_ON;
    }
    */
    show_tempture( ShowPar.temp_val);
}

void show_lock ()//童锁显示
{
    digi_flg = 1;
    Flg.lcd_sleep_flg = 0;
    digiBuf[0] = 0;
    digiBuf[1] = 13;
    digiBuf[2] = 0;
}


void show_sleep (uint8 dat )  //sleep
{
    digi_flg = 1;
    if(dat == ON)
    {
        Flg.lcd_sleep_flg = 1;
        LED_COM1_H;
        LED_COM2_H;
        LED_COM3_H;
    }
    else
    {
       Flg.lcd_sleep_flg = 0;
    }
    digiBuf[0] = 13;
    digiBuf[1] = 13;
    digiBuf[2] = 13;
}

void show_clean() //清洁显示
{
    digi_flg = 1;
    Flg.lcd_sleep_flg = 0;
    digiBuf[0] = 6;
    digiBuf[1] = 13;
    digiBuf[2] = 17;
}

void show_update(void)
{
    digi_flg = 1;
    Flg.lcd_sleep_flg = 0;
    digiBuf[0] = 18;
    digiBuf[1] = 13;
    digiBuf[2] = 14;
}
void show_wifi_pair(uint8 bai,uint8 shi,uint8 ge) //wifi pair 显示 ---
{
    digi_flg = 1;
    Flg.lcd_sleep_flg = 0;
    digiBuf[0] = bai;
    digiBuf[1] = shi;
    digiBuf[2] = ge;
}
void show_adj_key(uint8 id,uint8 dat) //档位调节显示
{
    digi_flg = 1;
    Flg.lcd_sleep_flg = 0;
    switch ( id )
    {
        case LAMP_VALVE:
            {
                digiBuf[0] = 16;
                digiBuf[1] = 0;
                digiBuf[2] = dat%10;
                break;
            }
        case AIR_VALVE:
            {
                digiBuf[0] = 15;
                digiBuf[1] = 0;
                digiBuf[2] = dat%10;
                break;
            }
        case WATER_VALVE:
            {
                digiBuf[0] = 14;
                digiBuf[1] = 0;
                digiBuf[2] = dat%10;
                break;
            }
        default:
            {
                break;
            }
    }
}

void write_err_num(uint8 dat) //故障码显示
{
     digi_flg = 1;
     Flg.lcd_sleep_flg = 0;
      switch (dat)
      {
          case ERR_F1:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 1; // 1
                  digiBuf[2] = 13;//
                  break;
              }
          case ERR_F2:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 2; // 2
                  digiBuf[2] = 13;//
                  break;
              }
          case ERR_F6:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 6; // 6
                  digiBuf[2] = 13;//
                  break;
              }
          case ERR_F7:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 7; // 7
                  digiBuf[2] = 13;//
                  break;
              }
          case ERR_F8:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 8; // 8
                  digiBuf[2] = 13;//
                  break;
              }
          case ERR_F9:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 9; // 9
                  digiBuf[2] = 13;//
                  break;
              }
      }

}

void show_state(uint8 state) //状态显示
{
    digi_flg = 1;
    Flg.lcd_sleep_flg = 0;
    ShowPar.on_off_flg = STATE_ON;
    if(state == STATE_ON)
    {
        digiBuf[0] = 0;//clear
        digiBuf[1] = 11; //o
        digiBuf[2] = 13;//n
    }
    else
    {
         digiBuf[0] = 0;//o
         digiBuf[1] =12; //f
         digiBuf[2] = 12;//f
    }
}
void LED_INIT(void)
{
    LED_SGIO_OUT; //IO输出模式
    LED_SEIO_OUT;
    LED_SFIO_OUT; //RF口设置为输出模式
    LED_SCIO_OUT;

    LED_COM1_L;
    LED_COM2_L;
    LED_COM3_L;
    LED_SEIO_OUT_L;
    LED_TAP_ON;
    LED_SHOWER_ON;
    LED_DRAIN_ON;
    LED_INC_ON;
    LED_DEC_ON;
    LED_WATER_ON;
    LED_AIR_ON;
    LED_LAMP_ON;
    delay_ms(2000);
    LED_COM1_H;
    LED_COM2_H;
    LED_COM3_H;
    LED_SEIO_OUT_H;
    LED_TAP_OFF;
    LED_SHOWER_OFF;
    LED_DRAIN_OFF;
    LED_INC_OFF;
    LED_DEC_OFF;
    LED_WATER_OFF;
    LED_AIR_OFF;
    LED_LAMP_OFF;
}


