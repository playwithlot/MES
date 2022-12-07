#include "write_cmd_handle.h"

#define LOG_TAG "rw cmd"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

void cmd_write_resrc_param_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	uint32_t cmd = 0;
	uint32_t *sp = RT_NULL;
	uint32_t *dp = RT_NULL;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *net_pack = (local_pack_t*)args;

	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd);
	

	buf[1] = 1;  //save
	dp = (uint32_t*)&buf[2];
	sp = (uint32_t*)&net_pack->buf[0];
	
	for(uint8_t i = 0; i < 39; i++)
	{
		*dp = sp[i];
		cmd = MODULE_RW_MT_PARAM_BASE + (i*MODULE_RW_CMD_OFFSET);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
		{
			err = buf[1];
		}

	}
	

	//local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_ACK);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
}





void cmd_write_pos_param_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t i = 0;
	uint8_t buf[6] = {0};
	uint32_t *sp = RT_NULL;
	uint32_t *dp = RT_NULL;
	uint32_t cmd = 0;
	local_pack_t cmd_pack = {0};
	local_pack_t *net_pack = (local_pack_t*)args;

	cmd_pack.buf = buf;
	dp = (uint32_t*)&buf[2];
	sp = (uint32_t*)&net_pack->buf[0];
	
	if(net_pack->dest_addr == CAN_ADDR_SAMPLE_PLTAFROM) //MODULE_RW_POS_PARAM_BASE
	{
		for(i = 0; i < 2; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 2; i < 4; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + MODULE_RW_CMD_OFFSET;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 4; i < 6; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*2);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 6; i < 19; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*3);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 19; i < 33; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*4);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 33; i < 35; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*5);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		

		for(i = 35; i < 37; i++)
		{
			buf[0] = i;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*7);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 37; i < 39; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*8);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 39; i < 41; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*8);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(uint8_t i = 0; i < 39; i++)
		{
			LOG_I("%d \n", sp[i]);
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_EMER_TRACK)
	{
		for(i = 0; i < 4; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 4; i < 6; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + MODULE_RW_CMD_OFFSET;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 6; i < 8; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*2);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 8; i < 12; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*3);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 12; i < 14; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*4);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		
		//param = dp[14];	
		buf[0] = 14;
		buf[1] = MODULE_SAVE_FALSGS;  //save
		*dp = sp[14];
		cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*5);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
		{
			err = buf[1];
		}
		
		
		for(uint8_t i = 0; i < 14; i++)
		{
			LOG_I("%d \n", sp[i]);
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_NORMAL_TRACK)
	{
		for(i = 0; i < 2; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 2; i < 4; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + MODULE_RW_CMD_OFFSET;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 4; i < 8; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*2);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 8; i < 10; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*3);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 10; i < 13; i++)
		{
			buf[0] = i;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*4);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 13; i < 15; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*4);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(uint8_t i = 0; i < 15; i++)
		{
			LOG_I("%d \n", sp[i]);
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_RECOVERY_TRACK)
	{
		for(i = 0; i < 2; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 2; i < 4; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + MODULE_RW_CMD_OFFSET;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 4; i < 8; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*2);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(i = 8; i < 10; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS;  //save
			*dp = sp[i];
			cmd = MODULE_RW_POS_PARAM_BASE + (MODULE_RW_CMD_OFFSET*3);
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
		for(uint8_t i = 0; i < 10; i++)
		{
			LOG_I("%d \n", sp[i]);
		}
		
	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	
	
}


void cmd_write_fsw_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t i = 0;
	uint8_t buf[8] = {0};
	uint8_t *dp = RT_NULL;
	uint32_t cmd = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	dp = (uint8_t*)&net_pack->buf[0];
	
	if(net_pack->dest_addr == CAN_ADDR_SAMPLE_PLTAFROM) 
	{
		for(i = 0; i < 12; i++)
		{
			buf[0] = i+1;
			buf[1] = 0; //save == false //MODULE_SAVE_FALSGS
			buf[2] = dp[i];  //sw status 
			cmd = MODULE_RW_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
	
	}
	else if(net_pack->dest_addr == CAN_ADDR_SAMPLE_STATUS) 
	{
		for(i = 0; i < 44; i++)
		{
			buf[0] = i+1;
			buf[1] = 0; //MODULE_SAVE_FALSGS; //save == false
			buf[2] = dp[i];  //sw status 
			cmd = MODULE_RW_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
	
	}
	else if(net_pack->dest_addr == CAN_ADDR_EMER_TRACK)
	{
		for(i = 0; i < 9; i++)
		{
			buf[0] = i+1;
			buf[1] = 0; //MODULE_SAVE_FALSGS; //save == false
			buf[2] = dp[i];  //sw status 
			cmd = MODULE_RW_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_NORMAL_TRACK)
	{
		for(i = 0; i < 7; i++)
		{
			buf[0] = i+1;
			buf[1] = 0; //MODULE_SAVE_FALSGS; //save == false
			buf[2] = dp[i];  //sw status 
			cmd = MODULE_RW_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
	}
	else if(net_pack->dest_addr == CAN_ADDR_RECOVERY_TRACK)
	{
		for(i = 0; i < 5; i++)
		{
			buf[0] = i+1;
			buf[1] = MODULE_SAVE_FALSGS; //save == false
			buf[2] = dp[i];  //sw status 
			cmd = MODULE_RW_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
			{
				err = buf[1];
			}
		}
		
	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	


}


void cmd_write_resrc_param_update_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	uint32_t cmd = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	buf[0] = 2; //update
	buf[1] = 1; //resrc param
	NET_GET_INDEX_LEV3(buf[2], net_pack->cmd); //resrc id param[2] = 1; 
	cmd = MODULE_RW_FAC_PARAM;
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		err = buf[1];
	}
	
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);

}


void cmd_write_pos_param_update_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t param[3] = {0};
	uint8_t buf[8] = {0};
	uint32_t cmd = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	param[0] = 2; //update
	param[1] = 2; //resrc param
	NET_GET_INDEX_LEV3(param[2], net_pack->cmd); //resrc id
	cmd = MODULE_RW_FAC_PARAM;
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		err = buf[1];
	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);

}


void cmd_write_fsw_param_update_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[8] = {0};

	uint32_t cmd = 0;
	local_pack_t cmd_pack = {0};
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *net_pack = (local_pack_t*)args;

	buf[0] = 2; //update
	buf[1] = 3; //resrc param
	NET_GET_INDEX_LEV3(buf[2], net_pack->cmd); //resrc id
	cmd = MODULE_RW_FAC_PARAM;
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		err = buf[1];
	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);

}



void cmd_write_resrc_param_reset_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[8] = {0};
	uint32_t cmd = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	buf[0] = 1; //update
	buf[1] = 1; //resrc param
	NET_GET_INDEX_LEV3(buf[2], net_pack->cmd); //resrc id
	cmd = MODULE_RW_FAC_PARAM;
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		err = buf[1];
	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);

}


void cmd_write_pos_param_reset_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[8] = {0};
	uint32_t cmd = 0;
	local_pack_t *net_pack = (local_pack_t*)args;


	buf[0] = 1; //update
	buf[1] = 2; //resrc param
	NET_GET_INDEX_LEV3(buf[2], net_pack->cmd); //resrc id
	cmd = MODULE_RW_FAC_PARAM;
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		err = buf[1];
	}
	
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	

}


void cmd_write_fsw_param_reset_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[8] = {0};
	uint32_t cmd = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	buf[0] = 1; //update
	buf[1] = 3; //resrc param
	NET_GET_INDEX_LEV3(buf[2], net_pack->cmd); //resrc id
	cmd = MODULE_RW_FAC_PARAM;
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		err = buf[1];
	}
	
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);

}


void dev_write_mt_param(void)
{
	uint32_t param[39] = {0};
	uint8_t buf[6] = {0};
	local_pack_t net_pack = {0}; 
	net_pack.buf = (uint8_t*)&param;
	
	param[0] = 1000;
	param[1] = 1002;
	param[2] = 1008;
	param[3] = 1003;
	param[4] = 1005;
	param[5] = 1006;
	param[6] = 1008;
	
	net_pack.cmd = NET_CMD_RW_BOARD_RESRC_PARAM;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	
	net_pack.cmd |= 0x00010000; 
	

	
	cmd_write_resrc_param_handle(&net_pack);

}

void dev_write_pos_param(void)
{
	uint32_t param[41] = {0};
	uint8_t buf[6] = {0};
	local_pack_t net_pack = {0}; 
	net_pack.buf = (uint8_t*)&param[0];
	
	param[0] = 10;
	param[1] = 199;
	param[2] = 1008;
	param[3] = 1003;
	param[4] = 1005;
	param[5] = 1006;
	param[6] = 2008;
	
	net_pack.cmd = NET_CMD_RW_BOARD_POS_PARAM;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	
	net_pack.cmd |= 0x00010000; 
	

	
	cmd_write_pos_param_handle(&net_pack);

}

void dev_write_fsw_param(void)
{
	uint8_t param[12] = {0};
	uint8_t buf[6] = {0};
	local_pack_t net_pack = {0}; 
	net_pack.buf = (uint8_t*)&param[0];
	
	param[0] = 1;
	param[1] = 1;
	param[2] = 1;
	param[3] = 1;
	
	param[4] = 0;
	param[5] = 0;
	param[6] = 0;
	param[7] = 0;
	
	param[8] = 1;
	param[9] = 1;
	param[10] = 1;
	param[11] = 1;
	
	net_pack.cmd = NET_CMD_RW_BOARD_FSW_PARAM;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	
	//net_pack.cmd |= 0x00010000; 
	

	
	cmd_write_fsw_handle(&net_pack);

}



MSH_CMD_EXPORT(dev_write_mt_param, "dev_write_mt_param");
MSH_CMD_EXPORT(dev_write_pos_param, "dev_write_pos_param");
MSH_CMD_EXPORT(dev_write_fsw_param, "dev_write_fsw_param");


