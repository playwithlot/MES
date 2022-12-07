#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "rtthread.h"
#include "rtthread.h"
#include "rtdevice.h"
#include "drv_gpio.h"
#include "common.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SCANNER_CNT_MAX_TIMES 0x03

typedef enum
{
	SCANNER_CMD_start = 0x01,
	SCANNER_CMD_stop = 0x02,
	SCANNER_CMD_pwr_on = 0x03,
	SCANNER_CMD_pwr_off = 0x04,
	SCANNER_CMD_start_10 = 0x05

} scanner_rev_cmd_t;


//static const scanner_ctrl_map_t scanner_ctrl_map[] = 
//{
//    {.cmd = SCANNER_CMD_pwr_on, .handle_callback = __scanner_pwr_on},
//    {.cmd = SCANNER_CMD_pwr_off, .handle_callback = __scanner_pwr_off},
//    {.cmd = SCANNER_CMD_start, .handle_callback = __scanner_start},
//    {.cmd = SCANNER_CMD_stop, .handle_callback = __scanner_stop},
//    {.cmd = SCANNER_CMD_start_10, .handle_callback = __scanner_start_10},
//};

extern void create_scanner_recv_thread(void);
extern void create_scanner_send_thread(void);
	

#endif
