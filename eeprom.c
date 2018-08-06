#include"eeprom.h"



/* FLASH 擦除函数  */
void nv_erase(uint16 addr)
{
    GIE=0;

	while(WR);		//等待读完成
	EEADR = (uint8)addr;		//写入地址信息
	EEADRH = (uint8)(addr>>8);
	CFGS = 0;			// 访问闪存程序存储器或数据 EEPROM 存储器
	EEPGD = 1;		//操作 FLASH
	FREE = 1;
	WREN = 1; 			//写EEPROM允许
	EECON2 = 0x55;	//写入特定时序
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
	while(WR);	//等待
	NOP();
	NOP();
	NOP();
	NOP();
	WREN = 0;
	FREE = 0;
    GIE=1;

}

/* EEPROM / FLASH 读数据函数  */
uint16 nv_read(uint8 type,uint16 addr)
{
	uint16 data;
    GIE=0;
	while(RD);		//等待读完成

	if(type == 0) {	// EEPROM
		EEADR = addr;		//写入要读的址址
	}	else	{
		EEADR = (uint8)addr;		//写入地址信息
		EEADRH = (uint8)(addr>>8);
	}

	CFGS = 0;			// 访问闪存程序存储器或数据 EEPROM 存储器
	EEPGD = type;	//操作EEPROM
	RD = 1;				//执行读操作
	NOP();
	NOP();
	while(RD);	//等待读完成

	if(type == 1) { // FLASH
		data = EEDATH;
		data <<= 8;
		data |= EEDATA;
	}	else	{
		data = EEDATA;
	}
    GIE=1;

	return data;	//返回读取的数据
}

/*  EEPROM / FLASH 写数据函数 */
void nv_write(uint8 type,uint16 addr,uint16 data)
{
    GIE=0;

	while(WR);	//等待写完成
    WREN = 1;           //写EEPROM允许
	if(type == 0)
	{
        CFGS = 0;               //访问闪存程序存储器或数据 EEPROM 存储器
        EEPGD = 0;       //操作0-EEPROM;1-FLASH
		EEADR = (uint8)addr;		//写入地址信息
		EEDATA = (uint8)data;		//写入数据信息
	}
	else
	{
	    //LWLO=1;
        CFGS = 0;               //访问闪存程序存储器或数据 EEPROM 存储器
        EEPGD = 1;       //操作0-EEPROM;1-FLASH
		EEADR = (uint8)addr;		//写入地址信息
		EEADRH = (uint8)(addr>>8);
		EEDATA = (uint8)data;		//写入数据信息
		EEDATH = (uint8)(data>>8);
	}
	EECON2 = 0x55;	//写入特定时序
	EECON2 = 0xaa;

	WR = 1;				//执行读操作
	/*
	do{
		WR = 1;				//执行读操作
	}while(WR==0);
	*/
	NOP();
	NOP();
	NOP();
	NOP();
	while(WR);	//等待写完成
	NOP();
	NOP();
	NOP();
	NOP();
	WREN = 0;				//禁止写入EEPROM
    GIE=1;

}


