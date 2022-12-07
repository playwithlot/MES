#ifndef __REPORT_H__
#define __REPORT_H__

#include "rtthread.h"
#include "iot_parameter.h"

#define REP_MES_master_reboot ((uint32_t)(0x00 | iot_para.net_addr))
#define REP_MES_master_pmt_online ((uint32_t)(0x0100 | iot_para.net_addr))
#define REP_MES_master_pmt_offline ((uint32_t)(0x01000100 | iot_para.net_addr))
#define REP_MES_master_scanner_online ((uint32_t)(0x0200 | iot_para.net_addr))
#define REP_MES_master_scanner_offline ((uint32_t)(0x01000200 | iot_para.net_addr))
#define REP_MES_master_reagent_rfid_online ((uint32_t)(0x0300 | iot_para.net_addr))
#define REP_MES_master_reagent_rfid_offline ((uint32_t)(0x01000300 | iot_para.net_addr))
#define REP_MES_master_rfid_data_err ((uint32_t)(0x02000300 | iot_para.net_addr))

#define REP_MES_substrate_1_normal ((uint32_t)(0x00040000 ))
#define REP_MES_substrate_1_err ((uint32_t)(0x1b040000 ))
#define REP_MES_substrate_2_normal ((uint32_t)(0x00050000 ))
#define REP_MES_substrate_2_err ((uint32_t)(0x1c050000 ))
#define REP_MES_trash_liquid_normal ((uint32_t)(0x00020000 ))
#define REP_MES_trash_liquid_err ((uint32_t)(0x19020000 ))

typedef struct
{
    uint32_t rec_time; //´´½¨±¨¸æµÄÊ±¼ä
    uint32_t mes;      //±¨¸æÄÚÈÝ
#define REPORT_SENDING_STATE_init 0
#define REPORT_SENDING_STATE_sending 1
#define REPORT_SENDING_STATE_sended 2
    uint8_t sending_state; //ÉÏ±¨×´Ì¬
} Report_t;

void report_deal_thread(void *p);
uint8_t report_mes(uint32_t mes);
uint8_t report_mes_inser(uint32_t mes, uint32_t index);
uint8_t report_make(uint8_t board, uint8_t mtr, uint8_t sensor, uint8_t mes);
void report_clear(void);

#endif
