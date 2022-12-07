#ifndef __MULTICAST_CMD_ANDLE_H
#define __MULTICAST_CMD_ANDLE_H


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
//#include "bool.h"


#define TRANSFER_TYPE_tcp 0               //??tcp??
#define TRANSFER_TYPE_udp 1               //??udp ??socket??
#define TRANSFER_TYPE_udp_broadcast 2     //??udp ??socket??
#define TRANSFER_TYPE_udp_mutibroadcast 3 //??udp??socket??


void multicast_handle_thread(void *args);
void create_multicast_handle_thread(void);

#endif

