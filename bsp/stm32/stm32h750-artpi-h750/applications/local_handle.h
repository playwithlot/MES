#ifndef __LOCAL_HNADLE_H
#define __LOCAL_HNADLE_H

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
#include "dev_can.h"
#include "dev_scanner.h"



#pragma pack(1)
typedef struct
{
	uint8_t dest_addr;
	uint32_t command;
	void (*process_callback)(void *args);  

} local_cmd_handle_t;

typedef struct
{
	uint8_t src_addr;
	uint32_t req;
	void (*process_callback)(void *args);  

} local_req_handle_t;


#pragma pack()



extern void create_local_recv_thread(void);
extern void create_local_handle_thread(void);


//import
//system tick cmd
extern void cmd_system_tick_handle(void *args);

//read cmd handle
extern void cmd_read_version_handle(void *args);
extern void cmd_read_resrc_param_handle(void *args);
extern void cmd_read_pos_param_handle(void *args);
extern void cmd_read_fsw_handle(void *args);
extern void cmd_read_opt_status_handle(void *args);

//write cmd handle
extern void cmd_write_fsw_param_reset_handle(void *args);
extern void cmd_write_pos_param_reset_handle(void *args);
extern void cmd_write_resrc_param_reset_handle(void *args);
extern void cmd_write_fsw_param_update_handle(void *args);
extern void cmd_write_pos_param_update_handle(void *args);
extern void cmd_write_resrc_param_update_handle(void *args);
extern void cmd_write_fsw_handle(void *args);
extern void cmd_write_pos_param_handle(void *args);
extern void cmd_write_resrc_param_handle(void *args);

//cyc cmd handle
extern void cmd_push_cache_store_handle(void *args);
extern void cmd_push_shelf_handle(void *args);
extern void cmd_move_shelf_handle(void *args);
extern void cmd_pop_shelf_handle(void *args);
extern void cmd_dev_reset_handle(void *args);
extern void cmd_dev_stop_handle(void *args);

//multicast cmd handle
extern void cmd_get_iot_info_handle(void *args);
extern void cmd_set_iot_info_handle(void *args);



//designa user code for below application intterface

extern uint8_t local_get_communicate_err(void);
extern uint16_t local_get_communicate_err_cnt(void);

#endif 
