#ifndef __PMT_H__
#define __PMT_H__

#include "rtthread.h"
#include "connect.h"
#include "exe_deal.h"

#define PMT_LOST_CNT_MAX 3 // PMT无反应累计次数，超过此次数则认为PMT不存在

// PMT控制命令
#define PMT_CTR_CMD_nop 0 // pmt空闲, 无实际对应的命令
#define PMT_CTR_CMD_get_version 1
#define PMT_CTR_CMD_set_resolution 2
#define PMT_CTR_CMD_set_gating_time 3
#define PMT_CTR_CMD_test_comm 4
#define PMT_CTR_CMD_start_continue 5
#define PMT_CTR_CMD_start_once 6
#define PMT_CTR_CMD_return_once 7
#define PMT_CTR_CMD_stop_and_clean 8

typedef struct Pmt
{
    //基本参数
    char version[20];
    uint16_t resolution;  //脉冲分辨时间，ns（由PMT硬件决定，每个PMT固定）
    uint32_t gating_time; //门控时间,ms（可设置，可以理解为多长时间产生一个PMT数据）
    uint16_t crc;         //此字段应该在所有需要保存的基本参数后面
    //状态
    uint8_t current_cmd; //当前正在执行中的命令
    uint8_t cnt;         //在线与否计数
    uint8_t last_alarm;
    uint8_t is_online;
    uint8_t is_exe : 4;
    uint8_t is_need_stop : 4;
} Pmt_t;

typedef struct Pmt_data
{
    uint32_t tick; //收到时的系统节拍数
    uint32_t data;
} Pmt_data_t;

typedef struct Pmt_ctr_cmd
{
    Obj_class_t ob;
    uint8_t cmd;
    uint32_t para;
    Exe_cls_t *exe;
} Pmt_ctr_cmd_t;

extern Pmt_t pmt_info;
void pmt_deal_thread(void *p);
void pmt_send_ctr(Pmt_ctr_cmd_t *ctr);
void pmt_send_ctr_urgent(Pmt_ctr_cmd_t *ctr);
uint8_t pmt_get_data(uint32_t *d);
void pmt_version_format(uint32_t *v);
uint8_t pmt_get_data_with_time(Pmt_data_t *d);
void pmt_clear_data(void);
Exe_cls_t *pmt_creat_exe(uint8_t cmd, void *data, uint8_t data_len);
uint16_t pmt_data_cnt(void);

#endif
