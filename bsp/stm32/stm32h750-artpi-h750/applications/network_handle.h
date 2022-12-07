#ifndef __NETWORK_HNADLE_H
#define __NETWORK_HNADLE_H

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



//tcp protorol param
#define TCP_PACK_MAX_LEN  1500

//nenwork pack param
#define NET_PACK_HEAD        0x5a1aa1a5
#define NET_PACK_END         0xa5a11a5a
#define NET_PACK_HEAD_LEN   15 //len of network cmd pack befor buf[]


#define NETWORK_WAIT_ACK_TIME  (1000)


typedef struct network_rsp
{
	uint32_t cmd;
	uint16_t serial;
    uint8_t type; 
    uint8_t src_addr;
    uint8_t dest_addr;

   
} network_rsp_t;




extern void create_tcp_recv_thread(void);
extern void create_tcp_send_thread(void);



#endif 
