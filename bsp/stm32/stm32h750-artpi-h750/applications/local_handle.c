#include "local_handle.h"
#include "iot_parameter.h"

#define LOG_TAG "local cmd"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

//a5 a1 1a 5a 00 00 0e 00 06 fe fe 00 00 00 00 10 8e 5a 1a a1 a5


rt_mq_t network_command_queue = RT_NULL;
rt_mq_t local_rsp_queue = RT_NULL;


static void local_recv_thread(void *args);
static void local_handle_thread(void *args);
static struct rt_thread local_handle_handle;
static struct rt_thread local_recv_handle;

static void local_recv_command_handle(void);
static void local_recv_report_handle(void);
static void local_background_handle(void);



void local_reset_communicate(void);
void local_set_communicate_err(void);
uint8_t local_get_communicate_err(void);


void local_reset_communicate_err_cnt(void);
void local_set_communicate_err_cnt(void);
uint16_t local_get_communicate_err_cnt(void);



static const local_cmd_handle_t local_evt_handle_map[] = 
{
	
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_SYSTEM_TICK, .process_callback = cmd_system_tick_handle},
	//read cmd
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_RD_VERSION, .process_callback = cmd_read_version_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RD_VERSION, .process_callback = cmd_read_version_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RD_VERSION, .process_callback = cmd_read_version_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RD_VERSION, .process_callback = cmd_read_version_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RD_VERSION, .process_callback = cmd_read_version_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RD_VERSION, .process_callback = cmd_read_version_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RD_BOARD_RESRC_PARAM, .process_callback = cmd_read_resrc_param_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RD_BOARD_RESRC_PARAM, .process_callback = cmd_read_resrc_param_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RD_BOARD_RESRC_PARAM, .process_callback = cmd_read_resrc_param_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RD_BOARD_RESRC_PARAM, .process_callback = cmd_read_resrc_param_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RD_BOARD_RESRC_PARAM, .process_callback = cmd_read_resrc_param_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RD_BOARD_POS_PARAM, .process_callback = cmd_read_pos_param_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RD_BOARD_POS_PARAM, .process_callback = cmd_read_pos_param_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RD_BOARD_POS_PARAM, .process_callback = cmd_read_pos_param_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RD_BOARD_POS_PARAM, .process_callback = cmd_read_pos_param_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RD_BOARD_POS_PARAM, .process_callback = cmd_read_pos_param_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RD_BOARD_FSW_PARAM, .process_callback = cmd_read_fsw_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RD_BOARD_FSW_PARAM, .process_callback = cmd_read_fsw_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RD_BOARD_FSW_PARAM, .process_callback = cmd_read_fsw_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RD_BOARD_FSW_PARAM, .process_callback = cmd_read_fsw_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RD_BOARD_FSW_PARAM, .process_callback = cmd_read_fsw_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RD_BOARD_OPT_STATUS, .process_callback = cmd_read_opt_status_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RD_BOARD_OPT_STATUS, .process_callback = cmd_read_opt_status_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RD_BOARD_OPT_STATUS, .process_callback = cmd_read_opt_status_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RD_BOARD_OPT_STATUS, .process_callback = cmd_read_opt_status_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RD_BOARD_OPT_STATUS, .process_callback = cmd_read_opt_status_handle},

	
	
	//write cmd
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_BOARD_RESRC_PARAM, .process_callback = cmd_write_resrc_param_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_BOARD_RESRC_PARAM, .process_callback = cmd_write_resrc_param_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_BOARD_RESRC_PARAM, .process_callback = cmd_write_resrc_param_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_BOARD_RESRC_PARAM, .process_callback = cmd_write_resrc_param_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_BOARD_RESRC_PARAM, .process_callback = cmd_write_resrc_param_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_BOARD_POS_PARAM, .process_callback = cmd_write_pos_param_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_BOARD_POS_PARAM, .process_callback = cmd_write_pos_param_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_BOARD_POS_PARAM, .process_callback = cmd_write_pos_param_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_BOARD_POS_PARAM, .process_callback = cmd_write_pos_param_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_BOARD_POS_PARAM, .process_callback = cmd_write_pos_param_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_BOARD_FSW_PARAM, .process_callback = cmd_write_fsw_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_BOARD_FSW_PARAM, .process_callback = cmd_write_fsw_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_BOARD_FSW_PARAM, .process_callback = cmd_write_fsw_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_BOARD_FSW_PARAM, .process_callback = cmd_write_fsw_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_BOARD_FSW_PARAM, .process_callback = cmd_write_fsw_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_UPDATE_RESRC_PARAM, .process_callback = cmd_write_resrc_param_update_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_UPDATE_RESRC_PARAM, .process_callback = cmd_write_resrc_param_update_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_UPDATE_RESRC_PARAM, .process_callback = cmd_write_resrc_param_update_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_UPDATE_RESRC_PARAM, .process_callback = cmd_write_resrc_param_update_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_UPDATE_RESRC_PARAM, .process_callback = cmd_write_resrc_param_update_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_UPDATE_POS_PARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_UPDATE_POS_PARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_UPDATE_POS_PARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_UPDATE_POS_PARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_UPDATE_POS_PARAM, .process_callback = cmd_write_pos_param_update_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_UPDATE_FSW_RARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_UPDATE_FSW_RARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_UPDATE_FSW_RARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_UPDATE_FSW_RARAM, .process_callback = cmd_write_pos_param_update_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_UPDATE_FSW_RARAM, .process_callback = cmd_write_pos_param_update_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_RESET_RESRC_PARAM, .process_callback = cmd_write_resrc_param_reset_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_RESET_RESRC_PARAM, .process_callback = cmd_write_resrc_param_reset_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_RESET_RESRC_PARAM, .process_callback = cmd_write_resrc_param_reset_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_RESET_RESRC_PARAM, .process_callback = cmd_write_resrc_param_reset_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_RESET_RESRC_PARAM, .process_callback = cmd_write_resrc_param_reset_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_RESET_POS_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_RESET_POS_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_RESET_POS_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_RESET_POS_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_RESET_POS_PARAM, .process_callback = cmd_write_pos_param_reset_handle},

	
	{.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM, .command = NET_CMD_RW_RESET_FSW_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_SAMPLE_STATUS, .command = NET_CMD_RW_RESET_FSW_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_EMER_TRACK, .command = NET_CMD_RW_RESET_FSW_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_NORMAL_TRACK, .command = NET_CMD_RW_RESET_FSW_PARAM, .process_callback = cmd_write_pos_param_reset_handle},
	{.dest_addr = CAN_ADDR_RECOVERY_TRACK, .command = NET_CMD_RW_RESET_FSW_PARAM, .process_callback = cmd_write_pos_param_reset_handle},

	
	
	//cyc cmd
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_DEV_RESET, .process_callback = cmd_dev_reset_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_DEV_STOP, .process_callback = cmd_dev_stop_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_MOVE_CACHE_REG, .process_callback = cmd_push_cache_store_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_PUSH_SHELF, .process_callback = cmd_push_shelf_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_MOVE_SHELF, .process_callback = cmd_move_shelf_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_MOVE_SHELF, .process_callback = cmd_move_shelf_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_POP_SHELF, .process_callback = cmd_pop_shelf_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_POP_SHELF, .process_callback = cmd_pop_shelf_handle},
	
	
	//multicast cmd
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_RD_GET_IOT_INFO, .process_callback = cmd_get_iot_info_handle},
	{.dest_addr = LOCAL_NET_ADDR, .command = NET_CMD_RD_SET_IOT_INFO, .process_callback = cmd_set_iot_info_handle},

};


extern void req_basket_start_test_handle(void *args);
extern void req_basket_pause_test_handle(void *args);
extern void req_basket_remove_sample_handle(void *args);
extern void req_basket_inser_sample_handle(void *args);
extern void req_basket_update_handle(void *args);

extern void req_emer_start_test_handle(void *args);
extern void req_emer_pause_test_handle(void *args);
extern void req_emer_remove_sample_handle(void *args);
extern void req_emer_inser_sample_handle(void *args);


static const local_req_handle_t local_req_handle_map[] = 
{
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_BASKET_START_TEST,  .process_callback = req_basket_start_test_handle},
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_BASKET_PASUSE_TEST, .process_callback = req_basket_pause_test_handle},
	
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_EMERGY_START_TEST, .process_callback = req_emer_start_test_handle},
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_EMERGY_PASUSE_TEST, .process_callback = req_emer_pause_test_handle},
	
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_BASKET_UPDATE_STATUS, .process_callback = req_basket_update_handle},
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_BASKET_INSER_SAMPLE, .process_callback = req_basket_inser_sample_handle},
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_BASKET_REMOVE_SAMPLE, .process_callback = req_basket_remove_sample_handle},
	
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_EMERGY_INSER_SAMPLE, .process_callback = req_emer_inser_sample_handle},
	{.src_addr = CAN_ADDR_SAMPLE_STATUS, .req = REQ_EMERGY_REMOVE_SAMPLE, .process_callback = req_emer_remove_sample_handle},
};


void create_local_handle_thread(void)
{
	rt_err_t err;

	err = rt_thread_init(&local_handle_handle, "local_handle_thread", 
											 local_handle_thread, 
											 RT_NULL,
											 (void *)LOCAL_HANDLE_THREAD_STACK_BASE,
											 LOCAL_HANDLE_THREAD_STACK_SIZE,
											 LOCAL_HANDLE_THREAD_PRIORITY, 
											 LOCAL_HANDLE_THREAD_TIME_SLINCE);
	if (err == RT_EOK)
	{
		rt_kprintf("local_handle_thread done!\n");
		rt_thread_startup(&local_handle_handle);
	}


}


void create_local_recv_thread(void)
{
	rt_err_t err;

	err = rt_thread_init(&local_recv_handle, "local_recv", 
											 local_recv_thread, 
											 RT_NULL,
											 (void *)LOCAL_RECV_THREAD_STACK_BASE, 
											 LOCAL_RECV_THREAD_STACK_SIZE, 
											 LOCAL_RECV_THREAD_PRIORITY, 20); 
	if(err == RT_EOK) 
	{
		rt_kprintf("local_recv_thread done!\n");
		rt_thread_startup(&local_recv_handle);
	}


}




void local_handle_thread(void *args)
{
	
	while(1)
	{
		//1.recv and handle network command
		local_recv_command_handle();	
		
		//2.recv and handle of lower report
		local_recv_report_handle();	
		
		//3.local background process
		local_background_handle();
		
		local_reset_communicate_err_cnt();
		local_reset_communicate();
		rt_thread_delay(1);
	}
	

}



void local_recv_thread(void *args)
{
	uint32_t dp = 0;
	struct rt_can_msg msg = {0};
	local_pack_t *pack = RT_NULL;
	
	while(1)
	{
		if(can_dev_get_msg(&msg) == RT_EOK)
		{

			switch(GET_CAN_PACK_TYPE(msg.id))
			{
				case PACK_TYPE_RESULT:
				case PACK_TYPE_DATA:
				case PACK_TYPE_REPORT:
				{
					pack = rt_malloc(sizeof(local_pack_t));
					pack->buf = rt_malloc(6);
				
					pack->len = msg.len;
					pack->serial = GET_CAN_PACK_serial(msg.id);
					pack->type = GET_CAN_PACK_TYPE(msg.id);
					pack->src_addr = GET_CAN_PACK_saddr(msg.id);
					pack->dest_addr = CAN_LOCAL_ADDR;
					pack->cmd = *((uint16_t*)&msg.data[0]);
					rt_memcpy(pack->buf, &msg.data[2], pack->len);
					
					dp = (uint32_t)pack;
					rt_mq_send(can_report_queue, &dp, sizeof(void*));	
					break;
				}
					
			}
			
			
		}
		
		rt_thread_delay(1);
	}
	
}

void local_free_rsp_frame(void)
{
	uint32_t dp = 0;
	local_pack_t *pack = RT_NULL;
	
	
	while(rt_mq_recv(can_response_queue, &dp, sizeof(void*), 0) == RT_EOK)
	{
		pack = (local_pack_t*)dp;
		if(pack->buf)
		{
			rt_free(pack->buf);
			pack->buf = RT_NULL;
		}
		
		if(pack)
		{
			rt_free(pack);
			pack = RT_NULL;
		}
	}
	

}

static uint8_t dev_communicate_err = 0;
uint8_t local_get_communicate_err(void)
{
	return dev_communicate_err;
}

void local_set_communicate_err(void)
{
	dev_communicate_err = 1;
}

void local_reset_communicate(void)
{
	dev_communicate_err = 0;
}


static uint16_t communicate_err_cnt = 0;
uint16_t local_get_communicate_err_cnt(void)
{
	return communicate_err_cnt;
}

void local_set_communicate_err_cnt(void)
{
	communicate_err_cnt++;
}

void local_reset_communicate_err_cnt(void)
{
	if(communicate_err_cnt)
	{
		LOG_E("can communicate err num %d\n", communicate_err_cnt);
	}
	
	communicate_err_cnt = 0;
}

static uint8_t can_frame_serial = 0; 
rt_err_t local_send_ctrl_frame(uint16_t cmd, uint8_t *buf, uint8_t dest_addr, uint8_t wait_rsp_falsg)
{
	rt_err_t err = 1;
	uint8_t rsp_type = 0;
	uint8_t send_cnt = 0;
	static uint8_t tem_buf[6] = {0};
	
	
	if(local_get_communicate_err())
	{
		LOG_E("can communicate err\n");
		return err;
	
	}
	
	if(wait_rsp_falsg)
	{
		can_frame_serial++;
	}

	while(1)
	{
		can_device_send_frame(cmd, buf, CAN_LOCAL_ADDR, dest_addr, can_frame_serial, PACK_TYPE_CMD, 6);
		if(can_device_get_rsp_frame(cmd, buf, dest_addr, can_frame_serial, PACK_TYPE_ACK, 1000) == RT_EOK)
		{
			if(wait_rsp_falsg & CAN_WAIT_FLAG_DATA)
			{
				if(can_device_get_rsp_frame(cmd, buf, dest_addr, can_frame_serial, PACK_TYPE_DATA, CAN_WAIT_FOR_RESPONSE) == RT_EOK)
				{
					rsp_type |= CAN_WAIT_FLAG_DATA;
				}
				
				//can_device_get_rsp_frame(cmd, tem_buf, dest_addr, can_frame_serial, PACK_TYPE_RESULT, CAN_WAIT_FOR_RESPONSE);
				
			}
			
			if(wait_rsp_falsg & CAN_WAIT_FLAG_RES)
			{
				if(can_device_get_rsp_frame(cmd, buf, dest_addr, can_frame_serial, PACK_TYPE_RESULT, CAN_WAIT_FOR_RESPONSE) == RT_EOK)
				{
					rsp_type |= CAN_WAIT_FLAG_RES;
				}
				
			}
			
			if(rsp_type == wait_rsp_falsg)
			{
				err = 0;
				
			}
			
			break;

		}
		else
		{
			rt_thread_delay(30);
			send_cnt++;
			if(send_cnt > 3)
			{
				err = 1;
				break;
			}
		}
	
	}
	
	if(err)
	{
		local_set_communicate_err_cnt();
		local_set_communicate_err();
		//local_free_rsp_frame();
		LOG_E("can device %d recv frame %d ack failure!!!\n", dest_addr, can_frame_serial);
	}
	
	
	return err;

}

rt_err_t local_wait_rsp_frame(uint16_t cmd, uint8_t *buf, uint8_t dest_addr, uint8_t wait_rsp_falsg)
{
	rt_err_t rsp_type = 0;

	if(wait_rsp_falsg & CAN_WAIT_FLAG_RES)
	{
		if(can_device_get_rsp_frame(cmd, buf, dest_addr, can_frame_serial, PACK_TYPE_RESULT, CAN_WAIT_FOR_RESPONSE) == RT_EOK)
		{
			rsp_type |= CAN_WAIT_FLAG_RES;
		}
		
	}
	
	if(wait_rsp_falsg & CAN_WAIT_FLAG_DATA)
	{
		if(can_device_get_rsp_frame(cmd, buf, dest_addr, can_frame_serial, PACK_TYPE_DATA, CAN_WAIT_FOR_RESPONSE) == RT_EOK)
		{
			rsp_type |= CAN_WAIT_FLAG_DATA;
		}
	}
	
	if(rsp_type == wait_rsp_falsg)
	{
		return RT_EOK;
	}
	
	return 1;

}



extern void set_net_response(local_pack_t *pack);
extern uint16_t network_pack_serial;
extern int net_udp_send(local_pack_t *pack);
uint8_t local_transmit_response(local_pack_t *pack, uint8_t *buf, uint8_t len, uint8_t types)
{
	
	uint32_t dp = 0;
	rt_err_t err = RT_EOK;
	local_pack_t *rsp_pack = RT_NULL;
	
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(len);
	rsp_pack->cmd = pack->cmd;
	rsp_pack->src_addr = pack->dest_addr;
	rsp_pack->dest_addr = pack->src_addr;
	rsp_pack->type = types;
	rsp_pack->len = len;
	rsp_pack->serial = pack->serial;
	rt_memcpy(&rsp_pack->buf[0], buf, rsp_pack->len);

	//net_udp_send(rsp_pack);
	
	if(rsp_pack->type == PACK_TYPE_REPORT)
	{
		network_pack_serial++;
		rsp_pack->serial = network_pack_serial;
		
	}

	dp = (uint32_t)rsp_pack;
	err = rt_mq_send(local_rsp_queue, &dp, sizeof(void*));
	if(err == RT_EOK)
	{
		return 1;
	}
	
	return 0;

}

uint8_t udp_transmit_response(local_pack_t *pack, uint8_t *buf, uint8_t len, uint8_t types)
{
	uint32_t dp = 0;
	rt_err_t err = RT_EOK;
	local_pack_t *rsp_pack = RT_NULL;
	
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(len);
	rsp_pack->cmd = pack->cmd;
	rsp_pack->src_addr = pack->dest_addr;
	rsp_pack->dest_addr = pack->src_addr;
	rsp_pack->type = types;
	rsp_pack->len = len;
	rsp_pack->serial = pack->serial;
	rt_memcpy(&rsp_pack->buf[0], buf, rsp_pack->len);
	
	//net_udp_send(rsp_pack);
	rt_free(rsp_pack->buf);
	rt_free(rsp_pack);
	
	
	return 0;

}


//network send and local recv
extern Iot_parameter_t iot_para;
static void local_recv_command_handle(void)
{
	uint32_t err = 0;
	uint32_t dp = 0;
	local_pack_t *pack = RT_NULL;
	
	
	
	if(rt_mq_recv(network_command_queue, &dp, sizeof(void*), 0) == RT_EOK)
	{
		pack = (local_pack_t*)dp;
		uint16_t num = sizeof(local_evt_handle_map) / sizeof(local_cmd_handle_t);
		
		for(uint16_t i = 0; i < num; i++)
		{
			
			if(local_evt_handle_map[i].command == (pack->cmd & 0x0000FFFF))
			{
				//local_evt_handle_map[i].dest_addr = pack->dest_addr;
				if(  (local_evt_handle_map[i].dest_addr == pack->dest_addr) || 
				     (local_evt_handle_map[i].dest_addr == LOCAL_NET_ADDR && pack->dest_addr == iot_para.net_addr) ||
					 (pack->cmd == NET_CMD_RD_GET_IOT_INFO) )
				{

					 if(pack->type == PACK_TYPE_CMD)
					 {
						//local_transmit_response(pack, (uint8_t*)&err, 4, PACK_TYPE_ACK);
						local_evt_handle_map[i].process_callback(pack);
						break;
					 }
					 else if(pack->cmd == NET_CMD_SYSTEM_TICK)
					 {
						local_evt_handle_map[i].process_callback(pack);
						break; 
					 }
					 
					
				}
				else
				{
					LOG_D("err net address %d\n", pack->dest_addr);
				}
			}


		}
		
		if(pack->buf)
		{
			rt_free(pack->buf);
			pack->buf = RT_NULL;
		}
		
		if(pack)
		{
			rt_free(pack);
			pack = RT_NULL;
		}
			
		
	
	}
	
}



	

static void local_recv_report_handle(void)
{
	uint16_t num = 0;
	uint32_t dp = 0;
	local_pack_t *pack = RT_NULL;
	
	
	num = sizeof(local_req_handle_map) / sizeof(local_req_handle_t);
	while(rt_mq_recv(can_report_queue, &dp, sizeof(void*), 0) == RT_EOK) 
	{
		pack = (local_pack_t*)dp;
		LOG_D("%04x %02x %02x %02x %02x %02x %02x\n", pack->cmd, pack->buf[0], pack->buf[1], pack->buf[2], pack->buf[3], pack->buf[4], pack->buf[5]);
		for(uint16_t i = 0; i < num; i++)
		{
			if( local_req_handle_map[i].req == pack->cmd && 
				local_req_handle_map[i].src_addr == pack->src_addr &&
				pack->type == PACK_TYPE_REPORT )
			{
				local_req_handle_map[i].process_callback(pack);
				break;
			}
		}
			
		if(pack->buf)
		{
			rt_free(pack->buf);
		}
		
		if(pack)
		{
			rt_free(pack);
		}
	
	}
	
}




extern void basket_sample_scan_handle(uint8_t basket_num);
extern void emer_store_sample_scan_handle(uint8_t basket_num);
static void local_background_handle(void)
{
	emer_store_sample_scan_handle(1);
	emer_store_sample_scan_handle(2);
	
	basket_sample_scan_handle(1);
	basket_sample_scan_handle(2);
	basket_sample_scan_handle(3);
	
}





