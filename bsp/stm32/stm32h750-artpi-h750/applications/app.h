#ifndef __APP_H
#define __APP_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "rtconfig.h"
#include "dev_flash.h"
#include "rtconfig.h"
#include "sockets.h"
#include "CRC16.h"
#include "rtthread.h"
#include "common.h" 

#include "dev_can.h"
#include "monitor.h"
#include "common.h"
#include "tcp_log.h"
#include "dev_scanner.h"
#include "dev_pmt.h"
#include "dev_rfid.h"
#include "local_handle.h"
#include "cyc_cmd_handle.h"
#include "multicast_cmd_handle.h"


void app_initailze(void);



#endif 

