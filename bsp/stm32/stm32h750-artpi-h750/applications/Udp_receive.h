#ifndef __UDP_RECEIVE_H__
#define __UDP_RECEIVE_H__


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

#define NET_DEFAULT_HEAD_LEN 15 //??????????????



void udp_recv_thread(void *args);
//int net_udp_send(Net_pack_deal_t *deal);

#endif





