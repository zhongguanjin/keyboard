
#include <stdio.h>
#include <stdarg.h>

#include "uart.h"

#define DEBUG_ENABLE

#ifndef DEBUG_ENABLE
#define my_printf(...)
#define dbg(...)
#else
void my_printf(const char *, ...);
void dbg(const char *, ...);
#endif


