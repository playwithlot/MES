#ifndef __RFID_READER_H__
#define __RFID_READER_H__

#include "rtthread.h"
#include "common.h"


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


//#define RFID_CN_NUM 1
#define RFID_CH_1 0x00
#define RFID_CH_2 0x01
#define RFID_CH_3 0x02


#define YZ_CMD_null 0x00
#define YZ_CMD_get_hardversion 0x16
#define YZ_CMD_scan_card 0x46
#define YZ_CMD_anticoll 0x47
#define YZ_CMD_select_card 0x48
#define YZ_CMD_vef_keyA 0x4a
#define YZ_CMD_read_3_block 0x22
#define YZ_CMD_read_1_block 0x4B

#define YZ_FRAM_head 0x02
#define YZ_FRAM_esc 0x10
#define YZ_FRAM_end 0x03

#define REC_STATE_null 0X00
#define REC_STATE_wait_head 0X01
#define REC_STATE_rec_esc 0X02
#define REC_STATE_wait_data 0X03
#define REC_STATE_done 0X05

#define CMD_STATE_null 0x00
#define CMD_STATE_wait_ack 0x01
#define CMD_STATE_end 0x02

typedef struct rfid_config_t
{
    uint8_t channel;
    char dev_name[10];
    uint32_t baud;
    rt_device_t dev_handle; //串口驱动
	
} rfid_config_t;

typedef struct rfid_info_t
{
    rfid_config_t *cfg; //串口配置
    uint8_t hard_vesion[4];
    uint8_t cnt:7;
    uint8_t is_offline:1;
    uint8_t last_alarm;
    //收发包处理
    uint8_t rec_state : 4;
    uint8_t cmd_state : 4;
    uint8_t buf_len;
    uint8_t buf[150];
    uint8_t cmd_len;
    uint8_t cmd_buf[50];
    rt_tick_t send_tick;
    rt_tick_t timeout_tick;
    //对上层参数
    uint8_t is_exe;
    uint8_t curent_cmd;
    uint32_t para;
    uint32_t err;
    uint8_t data_len;
    uint8_t data[1024];
		
} rfid_info_t;

#pragma pack(1)
typedef struct Yz_frame
{
    uint16_t addr;
    uint8_t len;
    uint8_t cmd;
    uint8_t buf[0];
} Yz_frame_t;
#pragma pack()

//extern rfid_info_t rfid_info;

//void rfid_recv_thread(void *p);
//void rfid_deal_thread(void *p);
//Exe_cls_t *rfid_creat_exe(uint8_t channel);
void rfid_alarm_deal(void);


//static Rfid_cmd_map_t cmd_map[] =
//    {
//        {YZ_CMD_get_hardversion, _get_version_recv_ack_deal},
//        {YZ_CMD_scan_card, _def_recv_ack_deal},
//        {YZ_CMD_anticoll, _def_recv_ack_deal},
//        {YZ_CMD_select_card, _def_recv_ack_deal},
//        {YZ_CMD_vef_keyA, _vef_keya_recv_ack_deal},
//        {YZ_CMD_read_1_block, _read_1_block_recv_ack_deal},
//        {YZ_CMD_read_3_block, _read_3_block_recv_ack_deal},
//};



void create_rfid_recv_thread(void);
void create_rfid_send_thread(void);

#endif
