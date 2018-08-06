
#ifndef _EEPROM_H_
#define _EEPROM_H_
#include "config.h"

extern void nv_write(uint8 type,uint16 addr,uint16 data);
extern uint16 nv_read(uint8 type,uint16 addr);
extern void nv_erase(uint16 addr);


#endif
