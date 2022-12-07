#include "read_cmd_handle.h"

#define LOG_TAG "read cmd"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

extern int net_udp_send(local_pack_t *pack);

//a5 a1 1a 5a 00 00 0a 00 06 fe fe 00 00 00 00 22 4e 5a 1a a1 a5
void cmd_system_tick_handle(void *args)
{
	local_pack_t *net_pack = (local_pack_t*)args;

	local_transmit_response(net_pack, net_pack->buf, net_pack->len, PACK_TYPE_HEARTBEAT);
	udp_transmit_response(net_pack, net_pack->buf, net_pack->len, PACK_TYPE_HEARTBEAT);

}


void cmd_read_version_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	board_version_t m_board_ver = {0};
	local_pack_t *net_pack = (local_pack_t*)args;

	if(net_pack->dest_addr == iot_para.net_addr)
	{
		rt_memcpy(&m_board_ver.hardware, &iot_para.hardware_version, sizeof(m_board_ver.hardware));
		rt_memcpy(&m_board_ver.software, &iot_para.software_version, sizeof(m_board_ver.software));
		
		m_board_ver.firmware[0] = ID_CODE_A;
		m_board_ver.firmware[1] = ID_CODE_B;
		m_board_ver.firmware[2] = ID_CODE_C;
		m_board_ver.firmware[3] = ID_CODE_D;
		
	}
	else
	{
		buf[0] = 1;
		if(local_send_ctrl_frame(MODULE_RD_VERSION_BASE, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK) 
		{
			rt_memcpy(m_board_ver.software, &buf[2], 4);
		}
		else
		{
			err = 1;
		}
		
		//read hardware version
		buf[0] = 2;
		if(local_send_ctrl_frame(MODULE_RD_VERSION_BASE, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK) 
		{
			rt_memcpy(m_board_ver.hardware, &buf[2], 4);
		}
		else
		{
			err = 1;
		}
		
		//read firmware version
		buf[0] = 3;
		if(local_send_ctrl_frame(MODULE_RD_VERSION_BASE, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&m_board_ver.firmware, &buf[2], 4);
		}
		else
		{
			err = 1;
		}
	}
	
	
	LOG_D("hardware: V%d.%d.%d.%d\n", m_board_ver.hardware[0],m_board_ver.hardware[1], m_board_ver.hardware[2], m_board_ver.hardware[3]);
	LOG_D("software: V%d.%d.%d.%d\n", m_board_ver.software[0],m_board_ver.software[1], m_board_ver.software[2], m_board_ver.software[3]);
	LOG_D("firmware: V%d.%d.%d.%d\n", m_board_ver.firmware[0],m_board_ver.firmware[1], m_board_ver.firmware[2], m_board_ver.firmware[3]);
	
	
	local_transmit_response(net_pack, (uint8_t*)&m_board_ver, sizeof(board_version_t), PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&m_board_ver, sizeof(board_version_t), PACK_TYPE_DATA);

}




void cmd_read_resrc_param_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t buf[6] = {0};
	uint32_t m_param[39] = {0};
	local_pack_t *net_pack = (local_pack_t*)args;
	uint32_t *dp = (uint32_t*)m_param;
	uint16_t cmd = 0;

	NET_GET_INDEX_LEV3(buf[0], net_pack->cmd); //get mt id
	for(uint8_t i = 0; i < 39; i++)
	{
		cmd = MODULE_RD_MT_PARAM_BASE + (0x100 * i);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[i], &buf[2], 4);
		}
		else
		{
			err = 1;
		}
		
	}
	
	for(uint8_t i = 0; i < 39; i++)
	{
		LOG_I("%d", dp[i]);
	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&m_param, sizeof(m_param), PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&m_param, sizeof(m_param), PACK_TYPE_DATA);
	
	
}


static void get_m1_pos_param(local_pack_t *net_pack)
{
	rt_err_t err = 1;
	uint8_t i = 0;
	uint8_t param = 0;
	uint8_t buf[8] = {0};
	uint32_t *dp = RT_NULL;
	uint32_t recv_buf[41] = {0};
	
	dp = (uint32_t*)recv_buf;
	
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[0], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[1], &buf[2], 4);
	}
	
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE+0x100, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[2], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE+0x100, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[3], &buf[2], 4);
	}
	
	
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE+0x200, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[4], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE+0x200, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[5], &buf[2], 4);
	}
	
	//y axle motor
	for(i = 6; i < 19; i++)
	{
		buf[0] = i;
		if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*i), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[6+i], &buf[2], 4);
		}
	}
	
	//x axle motor
	for(i = 19; i < 33; i++)
	{
		buf[0] = i;
		if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*i), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK) 
		{
			rt_memcpy(&dp[19+i], &buf[2], 4);
		}
	}
	
	//z axle motor
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*33), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK) 
	{
		rt_memcpy(&dp[33], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*33), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[34], &buf[2], 4);
	}
	
	
	//scd car shift motor
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*35), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[35], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*35), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[36], &buf[2], 4);
	}
	
	//push rod motor
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*37), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[37], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*37), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[38], &buf[2], 4);
	}
	
	//push rod motor
	buf[0] = 1;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*39), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[39], &buf[2], 4);
	}
	
	buf[0] = 2;
	if(local_send_ctrl_frame(MODULE_RD_POS_PARAM_BASE + (0x100*39), buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[40], &buf[2], 4);
		err = 0;
	}
	
	for(uint8_t i = 0; i < 41; i++)
	{
		LOG_I("%d \n", dp[i]);
	}
	
	//local response
	uint16_t len = sizeof(recv_buf);
	local_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	

}



static void get_m3_pos_param(local_pack_t *net_pack)
{
	rt_err_t err = 1;
	uint8_t i = 0;
	uint32_t cmd = 0;
	uint8_t param = 0;
	uint8_t buf[8] = {0};
	uint32_t *dp = RT_NULL;
	uint32_t recv_buf[15] = {0};
	
	dp = (uint32_t*)recv_buf;

	for(i = 0; i < 4; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE;
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + MODULE_RD_CMD_OFFSET;
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[4+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 2);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[6+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 4; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 3);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[8+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 4);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[12+i], &buf[2], 4);
		}
	}
	
	buf[0] = 1;
	cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 5);
	if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
		rt_memcpy(&dp[14], &buf[2], 4);
		err = 0;
	}
	
	for(uint8_t i = 0; i < 15; i++)
	{
		LOG_I("%d \n", dp[i]);
	}

	
	//local response
	uint16_t len = sizeof(recv_buf);
	local_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	

}

static void get_m4_pos_param(local_pack_t *net_pack)
{
	rt_err_t err = 1;
	uint8_t i = 0;
	uint32_t cmd = 0;
	uint8_t buf[8] = {0};
	uint32_t *dp = RT_NULL;
	uint32_t recv_buf[15] = {0};
	dp = (uint32_t*)recv_buf;

	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE;
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + MODULE_RD_CMD_OFFSET;
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[2+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 4; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 2);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[4+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 3);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[8+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 3; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 4);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[10+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 5);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[13+i], &buf[2], 4);
			err = 0;
		}
	}

	for(uint8_t i = 0; i < 15; i++)
	{
		LOG_I("%d \n", dp[i]);
	}

	//local response
	uint16_t len = sizeof(recv_buf);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	

}


static void get_m5_pos_param(local_pack_t *net_pack)
{
	rt_err_t err = 1;
	uint8_t i = 0;
	uint32_t cmd = 0;
	uint8_t param = 0;
	uint8_t buf[8] = {0};
	uint32_t *dp = RT_NULL;
	uint32_t recv_buf[9] = {0};
	
	dp = (uint32_t*)recv_buf;

	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE;
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + MODULE_RD_CMD_OFFSET;
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[2+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 3; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 2);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[4+i], &buf[2], 4);
		}
	}
	
	for(i = 0; i < 2; i++)
	{
		buf[0] = i+1;
		cmd = MODULE_RD_POS_PARAM_BASE + (MODULE_RD_CMD_OFFSET * 3);
		if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
		{
			rt_memcpy(&dp[7+i], &buf[2], 4);
			err = 0;
		}
	}
	
	for(uint8_t i = 0; i < 9; i++)
	{
		LOG_I("%d \n", dp[i]);
	}
	
	//local response
	uint16_t len = sizeof(recv_buf);
	local_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	
	

}


void cmd_read_pos_param_handle(void *args)
{
	local_pack_t *net_pack = (local_pack_t*)args;
	
	if(net_pack->dest_addr == CAN_ADDR_SAMPLE_PLTAFROM)
	{
		get_m1_pos_param(net_pack);
	}
	else if(net_pack->dest_addr == CAN_ADDR_EMER_TRACK)
	{
		get_m3_pos_param(net_pack);
	}
	else if(net_pack->dest_addr == CAN_ADDR_NORMAL_TRACK)
	{
		get_m4_pos_param(net_pack);
	}
	else if(net_pack->dest_addr == CAN_ADDR_RECOVERY_TRACK)
	{
		get_m5_pos_param(net_pack);
	}
	
}


void cmd_read_fsw_handle(void *args)
{
	rt_err_t err = 0;
	uint8_t i = 0;
	uint32_t cmd = 0;
	uint8_t buf[8] = {0};
	uint16_t len = 0;
	uint8_t* recv_buf = RT_NULL;;
	local_pack_t *net_pack = (local_pack_t*)args;
	
	if(net_pack->dest_addr == CAN_ADDR_SAMPLE_PLTAFROM)
	{
		len = 12;
		recv_buf = rt_malloc(12);	
		for(i = 0; i < 12; i++)
		{
			buf[0] = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
			
			
			
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_SAMPLE_STATUS)
	{
		len = 44;
		recv_buf = rt_malloc(44);
		
		for(i = 0; i < 44; i++)
		{
			buf[0] = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_EMER_TRACK)
	{
		len = 9;
		recv_buf = rt_malloc(9);
		
		for(i = 0; i < 9; i++)
		{
			buf[0] = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_NORMAL_TRACK)
	{
		len = 7;
		recv_buf = rt_malloc(7);
		
		for(i = 0; i < 7; i++)
		{
			buf[0] = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_RECOVERY_TRACK)
	{
		len = 5;
		recv_buf = rt_malloc(5);
		
		for(i = 0; i < 5; i++)
		{
			buf[0] = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		

	}
	
	//local response
	local_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	
	for(i = 0; i < len; i++)
	{
		LOG_I("%d\n", recv_buf[i]);
	}
	
	
	if(recv_buf)
	{
		rt_free(recv_buf);
	}


}



void cmd_read_opt_status_handle(void *args)
{
	uint8_t i = 0;
	uint32_t cmd = 0;
	uint8_t param = 0;
	uint8_t buf[8] = {0};
	local_pack_t *net_pack = (local_pack_t*)args;
	
	rt_err_t err = 0;
	uint16_t len = 0;
	uint8_t *recv_buf = RT_NULL;
	


	if(net_pack->dest_addr == CAN_ADDR_SAMPLE_PLTAFROM)
	{
		len = 21;
		recv_buf = rt_malloc(21);
		
		for(i = 0; i < 21; i++)
		{
			param = i+1;
			cmd = MODULE_RD_OPT_STATUS;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_SAMPLE_STATUS)
	{
		len = 44;
		recv_buf = rt_malloc(44);
		
		for(i = 0; i < 44; i++)
		{
			param = i+1;
			cmd = MODULE_RD_OPT_STATUS;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		
	}
	else if(net_pack->dest_addr == CAN_ADDR_EMER_TRACK)
	{
		len = 13;
		recv_buf = rt_malloc(13);
		
		for(i = 0; i < 13; i++)
		{
			param = i+1;
			cmd = MODULE_RD_OPT_STATUS;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}

	}
	else if(net_pack->dest_addr == CAN_ADDR_NORMAL_TRACK)
	{
		len = 12;
		recv_buf = rt_malloc(12);
		
		for(i = 0; i < 12; i++)
		{
			param = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}

	}
	else if(net_pack->dest_addr == CAN_ADDR_RECOVERY_TRACK)
	{
		len = 8;
		recv_buf = rt_malloc(8);
		
		for(i = 0; i < 8; i++)
		{
			param = i+1;
			cmd = MODULE_RD_FUNTIONAL_SW;
			if(local_send_ctrl_frame(cmd, buf, net_pack->dest_addr, CAN_WAIT_FLAG_DATA) == RT_EOK)
			{
				recv_buf[i] = buf[2];
			}
			else
			{
				err = 1;
			}
		}
		

	}
	
	//local response	
	local_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&recv_buf[0], len, PACK_TYPE_DATA);
	
	if(recv_buf)
	{
		rt_free(recv_buf);
	}


}



void dev_read_version(void)
{
	local_pack_t net_pack = {0}; 
	net_pack.buf = RT_NULL;
	
	net_pack.cmd = NET_CMD_RD_VERSION;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	cmd_read_version_handle(&net_pack);
	
	net_pack.cmd = NET_CMD_RD_VERSION;
	net_pack.dest_addr = CAN_ADDR_EMER_TRACK;
	net_pack.type = PACK_TYPE_CMD;
	cmd_read_version_handle(&net_pack);

}

MSH_CMD_EXPORT(dev_read_version, "dev_read_version");


void dev_read_resourece_param(void)
{
	uint8_t buf[6] = {0};
	local_pack_t net_pack = {0}; 
	net_pack.buf = buf;
	
	net_pack.cmd = NET_CMD_RD_BOARD_RESRC_PARAM;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	
	net_pack.cmd |= 0x00010000; 
	
	cmd_read_resrc_param_handle(&net_pack);
	
}


void dev_read_pos_param(void)
{
	uint8_t buf[6] = {0};
	local_pack_t net_pack = {0}; 
	net_pack.buf = buf;
	
	net_pack.cmd = NET_CMD_RD_BOARD_POS_PARAM;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	
	//net_pack.cmd |= 0x00010000; 
	
	cmd_read_pos_param_handle(&net_pack);
	
}


void dev_read_fsw_param(void)
{
	uint8_t buf[6] = {0};
	local_pack_t net_pack = {0}; 
	net_pack.buf = buf;
	
	net_pack.cmd = NET_CMD_RD_BOARD_FSW_PARAM;
	net_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	net_pack.type = PACK_TYPE_CMD;
	
	//net_pack.cmd |= 0x00010000; 
	
	cmd_read_fsw_handle(&net_pack);
	
}


void read_mt_param(void)
{
	uint32_t *res = 0;
	uint8_t buf[6] = {0};
	buf[0] = 1;
	
	if(local_send_ctrl_frame(MODULE_RD_MT_PARAM_BASE, buf, 1, CAN_WAIT_FLAG_DATA) == RT_EOK)
	{
			res = (uint32_t*)&buf[2];
	}

	LOG_D("rd mt param: %d \n", *res);
	
}

void write_mt_param(void)
{
	uint32_t *rw = 0;
	uint8_t buf[6] = {0x01, 0x0, 0x10, 0x27, 00, 00};
	rw = (uint32_t*)&buf[2];
	LOG_D("rw mt param: %d \n", *rw);
	
	if(local_send_ctrl_frame(0x0103, buf, 1, CAN_WAIT_FLAG_RES) == RT_EOK)
	{
		LOG_D("rw ok!\n");
	}

}

MSH_CMD_EXPORT(read_mt_param, "read_mt_param");
MSH_CMD_EXPORT(write_mt_param, "write_mt_param");




MSH_CMD_EXPORT(dev_read_pos_param, "dev_read_pos_param");
MSH_CMD_EXPORT(dev_read_resourece_param, "dev_read_resourece_param");
MSH_CMD_EXPORT(dev_read_fsw_param, "dev_read_fsw_param");
