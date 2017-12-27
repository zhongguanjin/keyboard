

#include "system.h"


static void system_clock_init(void)
{
	OSCSTAT = 0;
	#if(SYSCLK_FREQ_32MHz == 1)
		OSCCON = 0b11110010;		//使用内部8M晶振,使能PLL
	#elif(SYSCLK_FREQ_16MHz== 1)
		OSCCON = 0b01111010;		//使用内部16M晶振
	#elif(SYSCLK_FREQ_8MHz == 1)
		OSCCON = 0b01110010;		//使用内部8M晶振
	#elif(SYSCLK_FREQ_4MHz == 1)
		OSCCON = 0b01101010;		//使用内部4M晶振
    #elif( SYSCLK_FREQ_2MHz  == 1)
		OSCCON = 0b01100010; 		//使用内部2M晶振
    #else
		OSCCON = 0b01011010;		//使用内部1M晶振
	#endif
 	while(!OSCSTATbits.HFIOFR);		//等待晶振稳定
}

void Init_MCU(void)
{
	system_clock_init(); //使用内部16M晶振
	TRISA = 0x00;
	PORTA = 0xFF;
	ANSELA = 0x00;            //端A全部为输出

	TRISB = 0x00;             //端口B全部为输出
	PORTB = 0xFF;
	WPUB = 0x00;

	TRISC = 0x00;             //端口C全部为输出
	PORTC = 0x00;

	TRISD = 0x00;             //端口D全部为输出
	PORTD = 0xFF;

	TRISE = 0x00;             //端口E全部为输出
	PORTE = 0xFF;
	ANSELE = 0x00;

	TRISF = 0x00;			  //端口F全部为输出
	PORTF = 0x00;
	ANSELF = 0x00;

	TRISG = 0x00;
	PORTG = 0xFF;
	ANSELG = 0x00;
	WPUG = 0x00;

	IOCBP = 0x00;
	IOCBN = 0xFF;
	IOCBF = 0x00;

	FVREN = 0;				//禁止固定参考电

	DACEN = 0;
	C1ON = 0;
	C2ON = 0;
	C3ON = 0;
	SRLEN = 0;
	TMR1ON = 0;
	TMR2ON = 0;
	TMR4ON = 0;
	TMR6ON = 0;
	CCP3CON = 0x00;
	CCP4CON = 0x00;
	CCP5CON = 0x00;
	CPSON = 0;
	LCDEN = 0;

}


