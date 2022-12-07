#ifndef __READ_CMD_HANDLE_H
#define __READ_CMD_HANDLE_H


#include "rtthread.h"
#include "common.h"
#include "dev_can.h"
#include "dev_scanner.h"
#include "iot_parameter.h"



//board version
#define MODULE_RD_VERSION_BASE      (0x0401)
//motor param 
#define MODULE_RD_MT_PARAM_BASE     (0x0104)
//funtion sw
#define MODULE_RD_FUNTIONAL_SW      (0x2804)
//opt
#define MODULE_RD_OPT_STATUS        (0x2904)
// pos param
#define MODULE_RD_POS_PARAM_BASE    (0x2a04)

#define MODULE_RD_CMD_OFFSET        (0x0100)



typedef struct board_version
{
	uint8_t hardware[4];
	uint8_t software[4];
	uint8_t firmware[4];

} board_version_t;


typedef struct mt_param
{
	uint32_t reset_vstart;
	uint32_t reset_a1;
	uint32_t reset_node_v;
	uint32_t reset_a2;
	uint32_t reset_vruning;
	uint32_t reset_d1;
	uint32_t reset_d2;
	uint32_t reset_vstop;
	
	uint32_t sec1_runing_vstart;
	uint32_t sec1_a1;
	uint32_t sec1_runing_node_v;
	uint32_t sec1_runing_a2;
	uint32_t sec1_runing_v;
	uint32_t sec1_runing_d1;
	uint32_t sec1_runing_d2;
	uint32_t sec1_runing_vstop;
	
	uint32_t sec2_runing_vstart;
	uint32_t sec2_a1;
	uint32_t sec2_runing_node_v;
	uint32_t sec2_runing_a2;
	uint32_t sec2_runing_v;
	uint32_t sec2_runing_d1;
	uint32_t sec2_runing_d2;
	uint32_t sec2_runing_vstop;
	
	uint32_t sec3_runing_vstart;
	uint32_t sec3_a1;
	uint32_t sec3_runing_node_v;
	uint32_t sec3_runing_a2;
	uint32_t sec3_runing_v;
	uint32_t sec3_runing_d1;
	uint32_t sec3_runing_d2;
	uint32_t sec3_runing_vstop;
	
	uint32_t p0_opt_cps_len;
	uint32_t code_plate_opt_cps_len;
	uint32_t reset_opt_out_len;
	uint32_t encoder_rotate_cnt;
	uint32_t current_encoder_num;
	

} mt_param_t;


void cmd_read_version_handle(void *args); 
void cmd_read_resrc_param_handle(void *args); 
void cmd_read_pos_param_handle(void *args); 
void cmd_read_fsw_handle(void *args); 
void cmd_read_opt_status_handle(void *args); 

#endif 



