#ifndef __UPDATE_CMD_HANDLE
#define __UPDATE_CMD_HANDLE

#include "ymodem.h"
#include "dev_flash.h"

void IAP_thread_init(void);
void ReadAppInfo(void);
void Jump2app(void);

#endif 