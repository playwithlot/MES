#ifndef __CAN_COMM_H
#define __CAN_COMM_H

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




//micro for id operation
#define GET_CAN_PACK_TYPE(x) (x & 0x07)
#define SET_CAN_PACK_TYPE(x, y)            \
    do                                     \
    {                                      \
        x = (x & 0x1ffffff8) | (y & 0x07); \
    } while (0)

#define GET_CAN_PACK_serial(x) ((x>>3)&0xff)
#define SET_CAN_PACK_serial(x, y)                 \
    do                                            \
    {                                             \
        x = (x & 0x1FFFF807) | ((y & 0xff) << 3); \
    } while (0)

#define GET_CAN_PACK_mserial(x) ((x>>11)&0xff)
#define SET_CAN_PACK_mserial(x, y)                 \
    do                                             \
    {                                              \
        x = (x & 0x1ff807ff) | ((y & 0xff) << 11); \
    } while (0)

#define GET_CAN_PACK_daddr(x) ((x>>19)&0x1f)
#define SET_CAN_PACK_daddr(x, y)                   \
    do                                             \
    {                                              \
        x = (x & 0x1f07ffff) | ((y & 0x1f) << 19); \
    } while (0)

#define GET_CAN_PACK_saddr(x) ((x>>24)&0x1f) //获取源目的地址
#define SET_CAN_PACK_saddr(x, y)                 \
    do                                           \
    {                                            \
        x = (x & 0xffffff) | ((y & 0x1f) << 24); \
    } while (0)

	
#define CAN_WAIT_FLAG_NONE (0x00)
#define CAN_WAIT_FLAG_CMD  (0x01)
#define CAN_WAIT_FLAG_ACK  (0x02)
#define CAN_WAIT_FLAG_RES  (0x04)
#define CAN_WAIT_FLAG_DATA (0x08) 

		
#define CAN_WAIT_FOR_RESPONSE  (30000)

	
typedef struct can_rsp
{
	uint16_t serial;
    uint8_t type; 
    uint8_t src_addr;
    uint8_t dest_addr;
	uint16_t cmd;
	uint8_t buf[6];
   
} can_rsp_t;
	
	
	

extern void can_device_init(void);
extern rt_err_t can_dev_get_msg(rt_can_msg_t msg);


void can_dev_free_rsp_pack(void);



extern void create_can_recv_thread(void);
extern size_t can_device_send_ack(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t dest_addr, uint16_t serial, uint8_t type, uint8_t len);
extern rt_err_t can_device_get_rsp_frame(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t serial, uint8_t type, rt_tick_t timeout);
extern size_t can_device_send_frame(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t dest_addr, uint8_t serial, uint8_t type, uint8_t len);


#endif 



