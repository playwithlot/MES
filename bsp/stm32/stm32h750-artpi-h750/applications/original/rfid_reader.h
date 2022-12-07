#ifndef __RFID_READER_H__
#define __RFID_READER_H__

#include "rtthread.h"
#include "connect.h"
#include "exe_deal.h"
#include "fifo.h"

#define RFID_CN_NUM 1
#define RFID_CH_1 0x00
#define RFID_CH_2 0x01
#define RFID_CH_3 0x02

typedef struct Rfid_config
{
    uint8_t channel;
    char dev_name[10];
    uint32_t baud;
    rt_device_t dev; //串口驱动
} Rfid_config_t;

typedef struct Rfid_Info
{
    Rfid_config_t *con; //串口配置
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
} Rfid_Info_t;



extern Rfid_Info_t rfid_info[RFID_CN_NUM];

void rfid_recv_thread(void *p);
void rfid_deal_thread(void *p);
Exe_cls_t *rfid_creat_exe(uint8_t channel);
void rfid_alarm_deal(void);

#endif
