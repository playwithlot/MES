#include "mtdebug_cmd_handle.h"



void cmd_mt_enable_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	local_pack_t *net_pack = (local_pack_t*)args;

	//read software version
	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd); //get mt index
	buf[1] = net_pack->buf[0];
	err = local_send_ctrl_frame(MODULE_MT_EN, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES);
	

	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	
	
}

void cmd_mt_stop_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	local_pack_t *net_pack = (local_pack_t*)args;
	
	//read software version
	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd); //get mt index
	buf[1] = net_pack->buf[0];
	err = local_send_ctrl_frame(MODULE_MT_STOP, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES);
	
	//local response
	
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
}


void cmd_mt_reset_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	local_pack_t *net_pack = (local_pack_t*)args;

	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd); //get mt index
	buf[1] = net_pack->buf[0];
	err = local_send_ctrl_frame(MODULE_MT_RESET, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES);
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
}


void cmd_mt_runing_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	uint8_t type = 0;
	uint32_t cmd = 0;
	uint32_t *sp = 0;
	uint32_t *dp = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd);
	buf[1] = 1;  //not save
	dp = (uint32_t*)&buf[2];
	sp = (uint32_t*)&net_pack->buf[1];
	for(uint8_t i = 0; i < 8; i++)
	{
		*dp = sp[i];
		cmd = MODULE_MT_SET_PARAM_BASE + (i*MODULE_MT_PARAM_OFFSET);		
		err = local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES);
		
		
	}
	
	
	if(net_pack->buf[1])
	{
		cmd = MODULE_MT_MOVE_REL_POS;
	}
	else
	{
		cmd = MODULE_MT_MOVE_ABS_POS;
	}
	
	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd); //mt id
	buf[1] = 0;
	dp = (uint32_t*)&buf[2];
	sp = (uint32_t*)net_pack->buf;
	*dp = sp[8];
	err = local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES);
	
	//local response
	
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
}

