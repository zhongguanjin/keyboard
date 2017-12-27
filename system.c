

#include "system.h"


static void system_clock_init(void)
{
	OSCSTAT = 0;
	#if(SYSCLK_FREQ_32MHz == 1)
		OSCCON = 0b11110010;		//ʹ���ڲ�8M����,ʹ��PLL
	#elif(SYSCLK_FREQ_16MHz== 1)
		OSCCON = 0b01111010;		//ʹ���ڲ�16M����
	#elif(SYSCLK_FREQ_8MHz == 1)
		OSCCON = 0b01110010;		//ʹ���ڲ�8M����
	#elif(SYSCLK_FREQ_4MHz == 1)
		OSCCON = 0b01101010;		//ʹ���ڲ�4M����
    #elif( SYSCLK_FREQ_2MHz  == 1)
		OSCCON = 0b01100010; 		//ʹ���ڲ�2M����
    #else
		OSCCON = 0b01011010;		//ʹ���ڲ�1M����
	#endif
 	while(!OSCSTATbits.HFIOFR);		//�ȴ������ȶ�
}

void Init_MCU(void)
{
	system_clock_init(); //ʹ���ڲ�16M����
	TRISA = 0x00;
	PORTA = 0xFF;
	ANSELA = 0x00;            //��Aȫ��Ϊ���

	TRISB = 0x00;             //�˿�Bȫ��Ϊ���
	PORTB = 0xFF;
	WPUB = 0x00;

	TRISC = 0x00;             //�˿�Cȫ��Ϊ���
	PORTC = 0x00;

	TRISD = 0x00;             //�˿�Dȫ��Ϊ���
	PORTD = 0xFF;

	TRISE = 0x00;             //�˿�Eȫ��Ϊ���
	PORTE = 0xFF;
	ANSELE = 0x00;

	TRISF = 0x00;			  //�˿�Fȫ��Ϊ���
	PORTF = 0x00;
	ANSELF = 0x00;

	TRISG = 0x00;
	PORTG = 0xFF;
	ANSELG = 0x00;
	WPUG = 0x00;

	IOCBP = 0x00;
	IOCBN = 0xFF;
	IOCBF = 0x00;

	FVREN = 0;				//��ֹ�̶��ο���

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


