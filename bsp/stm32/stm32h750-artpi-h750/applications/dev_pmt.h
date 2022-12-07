#ifndef __DEV_PMT_H__
#define __DEV_PMT_H__

#include "rtthread.h"
#include "common.h"

#define PMT_DEV_NAME "uart8" //uart8
#define PMT_DEV_BAUD BAUD_RATE_19200


typedef struct
{
    uint8_t version[20];
    uint16_t resolution; 
    uint32_t gating_time; 
    uint16_t crc;      

} pmt_param_t;


typedef struct
{
	uint8_t is_online;
	uint8_t current_cmd;
	uint16_t pulse_count;

} pmt_status_t;

typedef struct
{
	pmt_param_t param;
    pmt_status_t status;    
	
} pmt_info_t;

typedef enum
{
	PMT_CTR_CMD_none = 0, 
	PMT_CTR_CMD_get_version,
	PMT_CTR_CMD_set_resolution,
	PMT_CTR_CMD_set_gating_time,
	PMT_CTR_CMD_test_comm,
	PMT_CTR_CMD_start_continue,
	PMT_CTR_CMD_start_once,
	PMT_CTR_CMD_re_start_once,
	PMT_CTR_CMD_stop_and_clean

} PMT_DEV_COMMAND_TYPES;


extern uint8_t pmt_dev_init(void);
extern void pmt_dev_control(uint8_t cmd, pmt_info_t *info);
extern void pmt_dev_recv_parse(pmt_info_t *info, uint8_t *buf, uint8_t len);
extern uint16_t pmt_dev_get_data(uint8_t *buf, uint16_t maxlen);
extern void pmt_param_get(pmt_param_t *dev_param);
extern void pmt_param_save(pmt_param_t *dev_param);




#endif
