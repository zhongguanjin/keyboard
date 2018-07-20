#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <pic.h>




typedef char	int8;
typedef int		int16;
typedef long	int32;
typedef unsigned char		uint8;
typedef unsigned int		uint16;
typedef unsigned long		uint32;
typedef unsigned long		ulong;
typedef unsigned long long	ullong;


#define ON         1
#define OFF        0

#define ok         (0)
#define err        (1)

typedef union {
	void *	ptr;
	uint32		u;		// u=0x01020304
	int32		i;
	uint16		ush[2];
	int16		sh[2];
	uint8		uch[4];	// ch[0]=0x04 ch[1]=03
	int8		ch[4];
} UN32;
typedef union {
	uint16		ush;
	int16		sh;
	uint8		uch[2];
	int8		ch[2];
} UN16;



#endif

