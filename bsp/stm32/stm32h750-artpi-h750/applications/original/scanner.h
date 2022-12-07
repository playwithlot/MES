#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "rtthread.h"
#include "connect.h"
#include "exe_deal.h"

#define SCANNER_CMD_start   0x01
#define SCANNER_CMD_stop    0x02
#define SCANNER_CMD_pwr_on  0x03
#define SCANNER_CMD_pwr_off  0x04
#define SCANNER_CMD_start_10   0x05

#define SCANNER_CNT_MAX_TIMES 0x03
typedef struct Scanner_config
{
    uint8_t type; //扫码枪类型
    uint8_t mode; //触发方式
} Scanner_config_t;

typedef struct Scanner_ctr_cmd
{
    Obj_class_t ob;
    uint8_t cmd;
    uint32_t timeout_tick;
    uint32_t para;
    Exe_cls_t *exe;
}Scanner_ctr_cmd_t;

typedef struct Scanner_info
{
    Scanner_config_t config;
    Scanner_ctr_cmd_t current_cmd;
    uint8_t is_busy;
    uint8_t cnt;
    uint8_t last_alarm;
    uint8_t is_online;
}Scanner_info_t;

void scanner_deal_thread(void *p);
Exe_cls_t *scanner_creat_exe(uint8_t cmd,uint32_t para, void *data, uint8_t data_len);
uint8_t scanner_get_data(uint8_t *buf, uint16_t *len);
void scanner_alarm_deal(void);

extern Scanner_info_t scanner_info;

#endif
