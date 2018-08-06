#include"eeprom.h"



/* FLASH ��������  */
void nv_erase(uint16 addr)
{
    GIE=0;

	while(WR);		//�ȴ������
	EEADR = (uint8)addr;		//д���ַ��Ϣ
	EEADRH = (uint8)(addr>>8);
	CFGS = 0;			// �����������洢�������� EEPROM �洢��
	EEPGD = 1;		//���� FLASH
	FREE = 1;
	WREN = 1; 			//дEEPROM����
	EECON2 = 0x55;	//д���ض�ʱ��
	EECON2 = 0xaa;
	WR = 1;
	/*
	do{
		WR = 1;
	}while(WR==0);
*/
    NOP();
    NOP();
    NOP();
    NOP();
	while(WR);	//�ȴ�
	NOP();
	NOP();
	NOP();
	NOP();
	WREN = 0;
	FREE = 0;
    GIE=1;

}

/* EEPROM / FLASH �����ݺ���  */
uint16 nv_read(uint8 type,uint16 addr)
{
	uint16 data;
    GIE=0;
	while(RD);		//�ȴ������

	if(type == 0) {	// EEPROM
		EEADR = addr;		//д��Ҫ����ַַ
	}	else	{
		EEADR = (uint8)addr;		//д���ַ��Ϣ
		EEADRH = (uint8)(addr>>8);
	}

	CFGS = 0;			// �����������洢�������� EEPROM �洢��
	EEPGD = type;	//����EEPROM
	RD = 1;				//ִ�ж�����
	NOP();
	NOP();
	while(RD);	//�ȴ������

	if(type == 1) { // FLASH
		data = EEDATH;
		data <<= 8;
		data |= EEDATA;
	}	else	{
		data = EEDATA;
	}
    GIE=1;

	return data;	//���ض�ȡ������
}

/*  EEPROM / FLASH д���ݺ��� */
void nv_write(uint8 type,uint16 addr,uint16 data)
{
    GIE=0;

	while(WR);	//�ȴ�д���
    WREN = 1;           //дEEPROM����
	if(type == 0)
	{
        CFGS = 0;               //�����������洢�������� EEPROM �洢��
        EEPGD = 0;       //����0-EEPROM;1-FLASH
		EEADR = (uint8)addr;		//д���ַ��Ϣ
		EEDATA = (uint8)data;		//д��������Ϣ
	}
	else
	{
	    //LWLO=1;
        CFGS = 0;               //�����������洢�������� EEPROM �洢��
        EEPGD = 1;       //����0-EEPROM;1-FLASH
		EEADR = (uint8)addr;		//д���ַ��Ϣ
		EEADRH = (uint8)(addr>>8);
		EEDATA = (uint8)data;		//д��������Ϣ
		EEDATH = (uint8)(data>>8);
	}
	EECON2 = 0x55;	//д���ض�ʱ��
	EECON2 = 0xaa;

	WR = 1;				//ִ�ж�����
	/*
	do{
		WR = 1;				//ִ�ж�����
	}while(WR==0);
	*/
	NOP();
	NOP();
	NOP();
	NOP();
	while(WR);	//�ȴ�д���
	NOP();
	NOP();
	NOP();
	NOP();
	WREN = 0;				//��ֹд��EEPROM
    GIE=1;

}


