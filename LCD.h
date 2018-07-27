#ifndef LCD_H_
#define LCD_H_

#include    "config.h"
#include "Task_Main.h"

enum
{
    STATE_OFF = 0,
    STATE_ON,
    STATTE_MAX
};

#define     ERR_F1    0X01
#define     ERR_F6    0X02
#define     ERR_F9    0X04
#define     ERR_F8    0X08
#define     ERR_F2    0X20
#define     ERR_F7    0X10
#define     ERR_OK    0X00


//列 io
#define     LED_SGIO_OUT                        ( TRISG = 0 )
#define     LED_COM3_H                          ( LATG3 = 1 )
#define     LED_COM3_L                          ( LATG3 = 0 )
#define     LED_COM2_H                          ( LATG2 = 1 )
#define     LED_COM2_L                          ( LATG2 = 0 )
#define     LED_COM1_H                          ( LATG1 = 1 )
#define     LED_COM1_L                          ( LATG1 = 0 )

//行 io
#define     LED_SEIO_OUT                        ( TRISE = 0 )

#define     LED_SEGH_H                          ( LATE7 = 1 )
#define     LED_SEGH_L                          ( LATE7 = 0 )
#define     LED_SEGG_H                          ( LATE6 = 1 )
#define     LED_SEGG_L                          ( LATE6 = 0 )
#define     LED_SEGF_H                          ( LATE5 = 1 )
#define     LED_SEGF_L                          ( LATE5 = 0 )
#define     LED_SEGE_H                          ( LATE4 = 1 )
#define     LED_SEGE_L                          ( LATE4 = 0 )
#define     LED_SEGD_H                          ( LATE3 = 1 )
#define     LED_SEGD_L                          ( LATE3 = 0 )
#define     LED_SEGC_H                          ( LATE2 = 1 )
#define     LED_SEGC_L                          ( LATE2 = 0 )
#define     LED_SEGB_H                          ( LATE1 = 1 )
#define     LED_SEGB_L                          ( LATE1 = 0 )
#define     LED_SEGA_H                          ( LATE0 = 1 )
#define     LED_SEGA_L                          ( LATE0 = 0 )

#define     LED_SEIO_OUT_H                      (LATE=0XFF)
#define     LED_SEIO_OUT_L                      (LATE=0X00)
#define     LED_SEG_DAT                         (LATE)

#define     LED_SCIO_OUT                        ( TRISC = 0 )
#define     M485_EN_H                           ( LATC2 = 1 )
#define     M485_EN_L                           ( LATC2 = 0 )


//按键灯
#define     LED_SFIO_OUT                        ( TRISF = 0 )

#if  key_5
#define     LED_TAP_OFF                         ( LATF5 = 0 )
#define     LED_TAP_ON                          ( LATF5 = 1 )
#define     LED_SHOWER_OFF                      ( LATF6 = 0 )
#define     LED_SHOWER_ON                       ( LATF6 = 1 )
#define     LED_DRAIN_OFF                       ( LATF7 = 0 )
#define     LED_DRAIN_ON                        ( LATF7 = 1 )
#define     LED_INC_OFF                         ( LATF3 = 0 )
#define     LED_INC_ON                          ( LATF3 = 1 )
#define     LED_DEC_OFF                         ( LATF4 = 0 )
#define     LED_DEC_ON                          ( LATF4 = 1 )
#else
#define     LED_TAP_OFF                         ( LATF3 = 0 )
#define     LED_TAP_ON                          ( LATF3 = 1 )
#define     LED_SHOWER_OFF                      ( LATF4 = 0 )
#define     LED_SHOWER_ON                       ( LATF4 = 1 )
#define     LED_DRAIN_OFF                       ( LATF5 = 0 )
#define     LED_DRAIN_ON                        ( LATF5 = 1 )
#define     LED_INC_OFF                         ( LATF6 = 0 )
#define     LED_INC_ON                          ( LATF6 = 1 )
#define     LED_DEC_OFF                         ( LATF7 = 0 )
#define     LED_DEC_ON                          ( LATF7 = 1 )
#endif



#define     LED_WATER_OFF                       ( LATC5 = 0 )
#define     LED_WATER_ON                        ( LATC5 = 1 )

#define     LED_AIR_OFF                         ( LATC4 = 0 )
#define     LED_AIR_ON                          ( LATC4 = 1 )

#define     LED_LAMP_OFF                        ( LATC3 = 0 )
#define     LED_LAMP_ON                         ( LATC3 = 1 )



#define  _XTAL_FREQ   ((double)16000000)

#define delay_us(x) __delay_us(x)

#define delay_ms(x) __delay_ms(x)


extern void     show_tempture( uint16 data);
extern void     LED_INIT();
extern void     led_scan(void);
extern void     show_lock ();
extern void     show_state(uint8 state);
extern void     show_sleep (uint8 dat);
extern void     write_err_num(uint8 dat);
extern void     show_adj_key(uint8 id,uint8 dat);
extern void     show_clean(); //清洁显示
extern void     show_wifi_pair(uint8 bai,uint8 shi,uint8 ge);
extern void     show_awaken();

#endif /* CAPT_APP_H_ */
