#ifndef __MTDEBUG_CMD_HANDLE_H
#define __MTDEBUG_CMD_HANDLE_H

#include "rtthread.h"
#include "common.h"
#include "dev_can.h"
#include "dev_scanner.h"


#define MODULE_MT_RESET          (0x0102)
#define MODULE_MT_EN             (0x0204)
#define MODULE_MT_STOP           (0x0304)
#define MODULE_MT_MOVE_ABS_POS   (0x0400)
#define MODULE_MT_MOVE_REL_POS   (0x0500)
#define MODULE_MT_ROTATE         (0x0600)
#define MODULE_MT_SET_PARAM_BASE (0x0103)
#define MODULE_MT_PARAM_OFFSET   (0x0100)

#endif 