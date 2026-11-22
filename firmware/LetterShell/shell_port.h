#ifndef __SHELL_PORT__
#define __SHELL_PORT__

#include "stm32f10x.h"
#include "sys.h"
#include "shell.h"

void User_Shell_Init(uint32_t baudrate);
extern Shell shell;
#define my_shell_print(fmt,...) shellPrint(&shell,fmt,##__VA_ARGS__)

#endif
