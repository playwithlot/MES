#ifndef __TCP_COMM_H
#define __TCP_COMM_H

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
#define NET_PACK_HEAD_LEN 15 //len of network cmd pack befor buf[]

//network machine address 
#define NETWORK_MACHINE_ADDRESS 0



#define NET_CMD_temp_get_version ((uint32_t)0x0101)

#define NET_CMD_temp_get_mt_para ((uint32_t)0x0201)
#define NET_CMD_temp_get_temp_ctr ((uint32_t)0x0201)
#define NET_CMD_temp_get_position_para ((uint32_t)0x0301)
#define NET_CMD_temp_get_temp_val ((uint32_t)0x0a09)
#define NET_CMD_temp_get_temp_val2 ((uint32_t)0x0601)
#define NET_CMD_temp_get_pressure_val ((uint32_t)0x0701)
#define NET_CMD_temp_get_LV_detect_AD_val ((uint32_t)0x0801)
#define NET_CMD_temp_get_LV_detect_para ((uint32_t)0x0901)
#define NET_CMD_temp_get_needle_para ((uint32_t)0x0a01)
#define NET_CMD_temp_get_pressure_detect_para ((uint32_t)0x0b01)

#define NET_CMD_temp_get_switch_ctr ((uint32_t)0x0401)

#define NET_CMD_temp_set_mt_para ((uint32_t)0x0102)
#define NET_CMD_temp_set_temp_ctr ((uint32_t)0x0102)
#define NET_CMD_temp_set_position_para ((uint32_t)0x0202)
#define NET_CMD_temp_set_switch_ctr ((uint32_t)0x0302)
#define NET_CMD_temp_resources_update_to_factory ((uint32_t)0x0402)
#define NET_CMD_temp_positions_update_to_factory ((uint32_t)0x0502)
#define NET_CMD_temp_func_switch_update_to_factory ((uint32_t)0x0602)
#define NET_CMD_temp_resources_restore_to_factory ((uint32_t)0x0702)
#define NET_CMD_temp_positions_restore_to_factory ((uint32_t)0x0802)
#define NET_CMD_temp_func_switch_restore_to_factory ((uint32_t)0x0902)
#define NET_CMD_temp_set_LV_detect_para ((uint32_t)0x0a02)
#define NET_CMD_temp_LV_detect_para_update_to_factory ((uint32_t)0x0b02)
#define NET_CMD_temp_LV_detect_para_restore_to_factory ((uint32_t)0x0c02)
#define NET_CMD_temp_set_needle_para ((uint32_t)0x0d02)
#define NET_CMD_temp_needle_para_update_to_factory ((uint32_t)0x0e02)
#define NET_CMD_temp_needle_para_restore_to_factory ((uint32_t)0x0f02)
#define NET_CMD_temp_set_pressure_detect_para ((uint32_t)0x1002)
#define NET_CMD_temp_pressure_detect_para_update_to_factory ((uint32_t)0x1102)
#define NET_CMD_temp_pressure_detect_para_restore_to_factory ((uint32_t)0x1202)

//调试设置
#define NET_CMD_temp_motor_test_enable ((uint32_t)0x0104)
#define NET_CMD_temp_motor_test_break ((uint32_t)0x0204)
#define NET_CMD_temp_motor_test_reset ((uint32_t)0x0304)
#define NET_CMD_temp_motor_test_run ((uint32_t)0x0404)
#define NET_CMD_temp_pump_valve_test ((uint32_t)0x0105)


//#define NET_CMD_temp_get_ignore_switch ((uint32_t)0x0301)
#define NET_CMD_temp_get_limit ((uint32_t)0x0601)
//#define NET_CMD_temp_set_ignore_switch ((uint32_t)0x0202)
#define NET_CMD_temp_set_limit ((uint32_t)0x0602)
#define NET_CMD_temp_get_temp ((uint32_t)0x0208)
#define NET_CMD_temp_get_opto_status ((uint32_t)0x0501)




//typedef enum
//{
//	NET_PACK_TYPE_CMD = 0x01,      //network to can
//	NET_PACK_TYPE_ACK = 0x02,      //bidirectional 
//	NET_PACK_TYPE_RESULT = 0x03,   //can to network
//	NET_PACK_TYPE_DATA = 0x04,     //can to net work
//	NET_PACK_TYPE_REPORT = 0x05,   //can to net work
//	NET_PACK_TYPE_HEARTBEAT =0x06,  //can to net work
//	NET_PACK_TYPE_HEARTBEAT_UPGRATE = 7

//} network_frame_types;






extern rt_mq_t local_handle_queue;
extern rt_mq_t local_ack_queue;
extern rt_mq_t scanner_handle_queue;
extern void create_tcp_recv_thread(void);
extern void create_tcp_send_thread(void);


extern void create_can_frame(uint8_t src_addr, uint8_t dest_addr, uint16_t serial, uint8_t frame_type, uint16_t cmd, uint8_t *buf, uint16_t len);
extern void create_network_frame(uint8_t src_addr, uint8_t dest_addr, uint16_t serial, uint8_t frame_type, uint16_t cmd, uint8_t *buf, uint16_t len);


#endif 
