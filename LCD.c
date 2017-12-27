
#include "lcd.h"


uint8 tap_val[14]={
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
0xbf, // -
0xab, //11 N
0x8e, //12 F

0xff, // clear
/*
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
*/
};



uint8 digiBuf[4]; //数码管缓冲区

void MatrixOutputData(void *p)
{
   uint8 *temp =p;
   //uint8 step=0;

   LED_SEG_DAT = *temp;

   /*
    for(uint8 i=0;i<8;i++)
    {
        step=((*temp)>>i)&0x01;
        switch(i)
        {
            case 0: //bit0
                {
                    if(step == OFF)
                    {
                       LED_SEGA_L;
                    }
                    else
                    {
                       LED_SEGA_H;
                    }
                    break;
                }
            case 1: //bit1
                {
                    if(step == OFF)
                    {
                       LED_SEGB_L;
                    }
                    else
                    {
                       LED_SEGB_H;
                    }
                    break;
                }

            case 2:
                {
                    if(step == OFF)
                    {
                       LED_SEGC_L;
                    }
                    else
                    {
                       LED_SEGC_H;
                    }
                    break;
                }
            case 3:
                {
                    if(step == OFF)
                    {
                       LED_SEGD_L;
                    }
                    else
                    {
                       LED_SEGD_H;
                    }
                    break;
                }
            case 4:
                {
                    if(step == OFF)
                    {
                       LED_SEGE_L;
                    }
                    else
                    {
                       LED_SEGE_H;
                    }
                    break;
                }
            case 5:
                {
                    if(step == OFF)
                    {
                       LED_SEGF_L;
                    }
                    else
                    {
                       LED_SEGF_H;
                    }
                    break;
                }
            case 6:
                {
                    if(step == OFF)
                    {
                       LED_SEGG_L;
                    }
                    else
                    {
                       LED_SEGG_H;
                    }
                    break;
                }
            case 7:
                {
                    if(step == OFF)
                    {
                       LED_SEGH_L;
                    }
                    else
                    {
                       LED_SEGH_H;
                    }
                    break;
                }
        }

    }  */
}

void led_scan(void)
{
    static uint8 digiPos = 0;
    static uint8 time_count = 0;
    LED_COM1_L;
    LED_COM2_L;
    LED_COM3_L;
    LED_SEIO_OUT_L;
    MatrixOutputData(&tap_val[digiBuf[digiPos]]);
    switch(digiPos)
    {
        case 0: LED_COM1_L;LED_COM2_H;LED_COM3_H; break; // 选择第一列数码管
        case 1: LED_COM1_H;LED_COM2_L;LED_COM3_H; break; // 选择第二列数码管
        case 2: LED_COM1_H;LED_COM2_H;LED_COM3_L; break; // 选择第三列数码管
    }
    digiPos++;
    if(digiPos == 3)
    {
        digiPos = 0;
    }

}


void show_tempture( uint16 data)
{
    digiBuf[0] = data/100;
    digiBuf[1] = (data%100)/10;
    digiBuf[2] = data%10;
}

void show_lock ()
{
    digiBuf[0] = 10;
    digiBuf[1] = 10;
    digiBuf[2] = 10;
}

void show_clean ( )
{
    digiBuf[0] = 13;
    digiBuf[1] = 13;
    digiBuf[2] = 13;
}

void write_err_num(uint8 dat)
{
      switch ( dat )
      {
          case ERR_F1:
              {
                  digiBuf[0] = 12;//F
                  digiBuf[1] = 1; // 1
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
void show_state(uint8 state)
{
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
 /*
    LED_SEGA_L;
    LED_SEGB_L;
    LED_SEGC_L;
    LED_SEGD_L;
    LED_SEGE_L;
    LED_SEGF_L;
    LED_SEGG_L;
    LED_SEGH_L;
    */
    LED_SEIO_OUT_L;
    LED_TAP_ON;
    LED_SHOWER_ON;
    LED_DRAIN_ON;
    LED_INC_ON;
    LED_DEC_ON;
    LED_WATER_ON;
    LED_AIR_ON;
    LED_LAMP_ON;
         delay_ms(3000);
    LED_COM1_H;
    LED_COM2_H;
    LED_COM3_H;
    /*
    LED_SEGA_H;
    LED_SEGB_H;
    LED_SEGC_H;
    LED_SEGD_H;
    LED_SEGE_H;
    LED_SEGF_H;
    LED_SEGG_H;
    LED_SEGH_H;
    */
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


