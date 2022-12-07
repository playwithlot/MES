#include "mes_flow.h"




rt_mq_t basket1_sample_shelf_queue = RT_NULL;
rt_mq_t basket2_sample_shelf_queue = RT_NULL;
rt_mq_t basket3_sample_shelf_queue = RT_NULL;
rt_mq_t emer_sample_shelf_queue = RT_NULL;


static uint8_t mes_recv_buf[100] = {0};
static cache_region_t cache_reg_table[MES_CACHE_REGION_NUM] = {0};

extern uint8_t local_wait_for_command(local_pack_t *pack, uint32_t cmd, rt_tick_t timeout);
extern uint8_t local_wait_for_slave_req(local_pack_t *pack, uint32_t cmd, rt_tick_t timeout);
extern void local_create_cmd_pack(local_pack_t *pack, uint32_t cmd, uint8_t addr, uint8_t *param, uint8_t len);



void cmd_get_basket_status(uint8_t basket_num)
{
	uint8_t res = 0;
	uint32_t dp = 0;
	uint16_t basket_stasut = 0;
	local_pack_t can_pack = {0};
	sample_shelf_t *sample_shelf = RT_NULL;
	rt_mq_t sample_shelf_queue = RT_NULL;
	
	can_pack.buf = mes_recv_buf;
	if(basket_num == 1)
	{
		sample_shelf_queue = basket1_sample_shelf_queue;
	}
	else if(basket_num == 2)
	{
		sample_shelf_queue = basket2_sample_shelf_queue;
	}
	else if(basket_num == 3)
	{
		sample_shelf_queue = basket3_sample_shelf_queue;
	}
	
	if(sample_shelf_queue == RT_NULL)
	{
		return;
	}
	
	
	while(rt_mq_recv(sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)  //clean basket
	{
		sample_shelf = (sample_shelf_t*)dp;
		rt_free(sample_shelf);
	}

	local_create_cmd_pack(&can_pack, MODULE_CMD_CHECK_BASKET, CAN_ADDR_SAMPLE_STATUS, &basket_num, sizeof(basket_num));
	can_device_send_pack(&can_pack);
	

	res = can_device_get_rsp_pack(&can_pack, PACK_TYPE_DATA, 300);
	if(res)
	{
		basket_stasut = *((uint16_t*)&can_pack.buf[2]);  //cmd: D0 D1 D3 == basket D4D5 == status
	}
	
	
	basket_stasut = 0x2001;   //
	for(uint8_t i = 0; i < 13; i ++)
	{
		if(basket_stasut & (0x01<<i))
		{
			sample_shelf = rt_malloc(sizeof(sample_shelf_t));
			sample_shelf->basket_pos = basket_num;
			sample_shelf->slot_pos = i+1;
			dp = (uint32_t)sample_shelf;
			rt_mq_send(sample_shelf_queue, &dp, sizeof(void*));

		
		}
	}	
		
	
}	



uint8_t flow_scan_sample(uint8_t *buf, uint8_t maxlen)
{
	uint8_t res = 0;
	local_pack_t res_pack = {0};
	res_pack.buf = mes_recv_buf;
	
	scanner_dev_ctrl(SCANNER_CMD_start, SCANNER_TYPE_leuze_BCL95, SCANNER_MODE_leuze_once);  //send start scan command
	//rt_thread_delay(3);   //wait for motor move to spacified position
	res = scanner_device_get_rsp_pack(&res_pack, 1000);
	if(res && (res_pack.len > 10))
	{
		res_pack.len %= maxlen;
		rt_memcpy(buf, res_pack.buf, res_pack.len); 
		
		rt_kprintf("recv[%d]: ", res_pack.len);
		for(uint8_t i = 0; i < res_pack.len; i++)
		{
			rt_kprintf("%c", res_pack.buf[i]);
		
		}
		
		return res_pack.len;
			
	}

	return 0;
	
}


void flow_create_cache_region(void)
{

	for(uint8_t i = 0; i < MES_CACHE_REGION_NUM; i++)
	{
		cache_reg_table[i].is_empty = 1;
		cache_reg_table[i].locate_pos = i+1;
		cache_reg_table[i].shape_code[0] = 0;
		cache_reg_table[i].shape_code[1] = 0;
		cache_reg_table[i].shape_code[2] = 0;
	}

}

int flow_push_cache_region(sample_shelf_t *sample_shelf)
{
	for(uint8_t i = 0; i < MES_CACHE_REGION_NUM; i++)
	{
		if(cache_reg_table[i].is_empty)
		{
			cache_reg_table[i].shape_code[0] = sample_shelf->shape_code[0];
			cache_reg_table[i].shape_code[1] = sample_shelf->shape_code[1];
			cache_reg_table[i].shape_code[2] = sample_shelf->shape_code[2];
			cache_reg_table[i].is_empty = 0;
			return cache_reg_table[i].locate_pos;
		}
		
	}
	
	return 0;
	
}




int flow_pop_cache_region(sample_shelf_t *sample_shelf)
{
	for(uint8_t i = 0; i < MES_CACHE_REGION_NUM; i++)
	{
		if( sample_shelf->shape_code[0] == cache_reg_table[i].shape_code[0] &&
			sample_shelf->shape_code[1] == cache_reg_table[i].shape_code[1] &&
			sample_shelf->shape_code[2] == cache_reg_table[i].shape_code[2] )
		{
			cache_reg_table[i].is_empty = 1;
			return cache_reg_table[i].locate_pos;
		}
		
	}
	
	return 0;
	
}









static sample_shelf_t current_sample_shelf = {0};
void flow_scan_sample_shelf(sample_shelf_t *sample_shelf)
{
	uint8_t i = 0;
	uint8_t *pbuf = RT_NULL;
	uint8_t sample_num = 0;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t can_pack = {0};
	can_pack.buf = mes_recv_buf;
	
	

	//if(rt_mq_recv(sample_location_queue, &dp, sizeof(void*), 0) == RT_EOK)
	
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf =  rt_malloc(210);
	
	//sample_shelf = (sample_shelf_t*)dp;
	//rt_kprintf("=============[bastet %d] [slot %d]=============\n", sample_shelf->basket_pos, sample_shelf->slot_pos);
	
	//2.set led status 
	rt_kprintf("\n===============step2.set indicate led===============\n");
	mes_recv_buf[0] = sample_shelf->basket_pos;
	mes_recv_buf[1] = sample_shelf->slot_pos;
	mes_recv_buf[2] = 0x02;  //led status: wait for test, color: red
	local_create_cmd_pack(&can_pack, 0x0601, CAN_ADDR_SAMPLE_STATUS, &mes_recv_buf[0], 3);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 500);  //wait rof res
	
	//rt_thread_delay(5);
	
	//3.set motor position: get sample shelf
	rt_kprintf("\n===============step3.get sample shelf===============\n");
	mes_recv_buf[0] = sample_shelf->basket_pos;
	mes_recv_buf[1] = sample_shelf->slot_pos;
	local_create_cmd_pack(&can_pack, 0x0a01, CAN_ADDR_SAMPLE_PLTAFROM, &mes_recv_buf[0], 2);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res
	
	sample_num = 0;
	rt_memset(rsp_pack->buf, 0, 210);
	rt_kprintf("\n===============step4.scan sample[%d] info===============\n", i+1);
	for(i = 0; i < 10; i++)
	{	
		//4.set sample scan position: set sample scan position
		rt_kprintf("4.1 set sample[%d] pos\n", i+1);
		mes_recv_buf[0] = 0x06; 
		mes_recv_buf[1] = i+1;
		local_create_cmd_pack(&can_pack, 0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &mes_recv_buf[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 1000);  //wait rof res
		
		//5.check the sanmple is exist
		rt_kprintf("4.2 check is sample [%d] exist?\n", i+1);
		mes_recv_buf[0] = 0x06; 
		local_create_cmd_pack(&can_pack, 0x2904, CAN_ADDR_SAMPLE_PLTAFROM, &mes_recv_buf[0], 1);  //0x41
		can_device_send_pack(&can_pack);
		can_pack.buf[3] = 0;
		
		
		if(can_device_get_rsp_pack(&can_pack, PACK_TYPE_DATA, 1000))
		{
			//6.scan smple and save shape code
			if(can_pack.buf[3])
			{
				rt_kprintf("4.3 got and scan sample [%d]... \n\n\n", i+1);
				pbuf = &rsp_pack->buf[20*i+5];
				pbuf[0] = 0;
				pbuf[1] = 0;
				pbuf[1] = flow_scan_sample(&pbuf[2], 20); //2.scan sample
				if(pbuf[1])
				{
					pbuf[0] = 1;
					sample_num++;
				}
			}	
		}
		
		if(!can_pack.buf[3])
		{
			rt_kprintf("4.3 fail to get sample [%d]!!!\n\n\n", i+1);
		}
		
		
				
	}
	
	rt_kprintf("found total sample num: %d \n", sample_num);
	
	//6.scan sample shelf shape code: set sample scan position
	mes_recv_buf[0] = 7;
	local_create_cmd_pack(&can_pack, 0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &mes_recv_buf[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_DATA, 500);  //wait rof res
	flow_scan_sample(sample_shelf->shape_code, 3);   //2.scan sample shelf	shape code
	
	

	//7.set motor position: push all sample to car :cmd16
	mes_recv_buf[0] = 8;
	local_create_cmd_pack(&can_pack, 0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &mes_recv_buf[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_DATA, 500);  //wait rof res
	
	
	//8.upload result to network, report frame
	if(rsp_pack && rsp_pack->buf)  
	{
		rsp_pack->buf[0] = sample_shelf->basket_pos;
		rsp_pack->buf[1] = sample_shelf->slot_pos;
		rt_memcpy(&rsp_pack->buf[2], sample_shelf->shape_code, 3);

		rsp_pack->cmd = 0x0405;
		rsp_pack->len = 207;
		rsp_pack->type = PACK_TYPE_REPORT;
		local_send_response(rsp_pack);  				
		
	}
	

	//9.wait for network command: push shelf of push to cache region
		
}

void cmd_push_cache_reg_handle(void *args)
{
		//9.wait for network command: push shelf of push to cache region
		int cache_reg = 0;
		local_pack_t *rsp_pack = RT_NULL;
		local_pack_t can_pack = {0};
		
		can_pack.buf = mes_recv_buf;
		rsp_pack = rt_malloc(sizeof(local_pack_t));
		rsp_pack->buf =  rt_malloc(4);
		

		//rt_kprintf("mes: push_shelf command: %0x4X\n", cmd_pack->cmd);
		rt_kprintf("\n===============step7.push sample shelf to cache reg===============\n");
		
		//10~11.push shelf to cache region
		cache_reg = flow_push_cache_region(&current_sample_shelf);
		if(cache_reg)
		{
			mes_recv_buf[0] = 4;
			mes_recv_buf[1] = cache_reg;
			local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &mes_recv_buf[0], 2);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, PACK_TYPE_DATA, 500);  //wait rof res
		}
		
		rsp_pack->cmd = 0x0205;
		rsp_pack->len = 4;
		rsp_pack->buf[3] = 0;  //res ok
		rsp_pack->type = PACK_TYPE_RESULT;
		local_send_response(rsp_pack);

}


void cmd_push_shelf_handle(void *args)
{
	uint16_t cmd = 0;
	int cache_reg = 0;
	uint8_t sel_track = 0;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = RT_NULL;
	local_pack_t can_pack = {0};
	sample_shelf_t pop_sample_shelf = {0};
	
	cmd_pack = (local_pack_t*)args;
	can_pack.buf = mes_recv_buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	
	pop_sample_shelf.shape_code[0] = cmd_pack->buf[0];
	pop_sample_shelf.shape_code[1] = cmd_pack->buf[1];
	pop_sample_shelf.shape_code[2] = cmd_pack->buf[2];

	//rt_kprintf("mes: push_shelf command: %0x4X\n", cmd_pack->cmd);
	rt_kprintf("\n===============step7.push sample shelf to cache reg===============\n");
	
	//12.push shelf to other track
	//(push shelf from basket)
	//push sample from basket
	if( pop_sample_shelf.shape_code[0] == current_sample_shelf.shape_code[0] &&
		pop_sample_shelf.shape_code[1] == current_sample_shelf.shape_code[1] &&
		pop_sample_shelf.shape_code[2] == current_sample_shelf.shape_code[2] )
	{
	
	
	}
	//push shelf from cache reg
	else if( (cache_reg = flow_pop_cache_region(&pop_sample_shelf)) != 0 )
	{
		//1.get sample shelf from cache reg
		can_pack.buf[0] = 4;
		can_pack.buf[1] = cache_reg;
		local_create_cmd_pack(&can_pack, 0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &can_pack.buf[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res
		
	}
	//not found sample shelf
	else 
	{
		//response 
		rsp_pack->cmd = cmd_pack->cmd;
		rsp_pack->len = 4;
		rsp_pack->buf[0] = 1;  //rsp		err
		rsp_pack->buf[1] = 1;  
		rsp_pack->buf[2] = 1;  
		rsp_pack->buf[3] = 1;  
		rsp_pack->type = PACK_TYPE_RESULT;
		local_send_response(rsp_pack);
		return;
		
	}
		
	//2.push sample shelf to track cache position: step1
	can_pack.buf[0] = 6; //cmd_pack->buf[0] ~ [2] shape code
	can_pack.buf[1] = cmd_pack->buf[4];  //1: emer track 2: normal track 3:recyc track
	local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &can_pack.buf[0], 2);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res
	
#if 0	
	//3.push sample shelf to track cache position: step2 
	if(cmd_pack->buf[4] == 0x01)   //1: emer track 2: normal track 3:recyc track
	{
		sel_track = CAN_ADDR_EMER_TRACK;
		cmd = 0x0701;
		can_pack.buf[0] = 1;
	}
	else if(cmd_pack->buf[4] == 0x02)
	{
		sel_track = CAN_ADDR_EMER_TRACK;
		cmd = 0x0601;
		can_pack.buf[0] = 1;
	}
	local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res
	
	
	//4.push sample shelf to addtion sample position
	if(cmd_pack->buf[4] == 0x01)
	{
		sel_track = CAN_ADDR_EMER_TRACK;
		cmd = 0x0701;
		can_pack.buf[0] = 2;
	}
	else if(cmd_pack->buf[4] == 0x02)
	{
		sel_track = CAN_ADDR_NORMAL_TRACK;
		cmd = 0x0601;
		can_pack.buf[0] = 1;
	}


	if(cmd_pack->buf[5] == 0x01)  
	{ 
		local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res

	}
	else if(cmd_pack->buf[5] == 0x03)
	{  
		local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res
		
		if(cmd_pack->buf[4] == 0x01)
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			can_pack.buf[0] = 3;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_NORMAL_TRACK;
			cmd = 0x0601;
			can_pack.buf[0] = 2;
		}
		local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait rof res
	}
	
#endif
	
	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->len = 4;
	rsp_pack->buf[0] = 0;  //res ok
	rsp_pack->buf[1] = 0;  
	rsp_pack->buf[2] = 0;  
	rsp_pack->buf[3] = 0;  
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);

			
}
	

void cmd_move_shelf_handle(void *args)
{
	uint8_t sel_track = 0;
	uint16_t cmd = 0;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = (local_pack_t*)args;
	local_pack_t can_pack = {0};

	can_pack.buf = mes_recv_buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	
	//1.move test tube of sample shelf
	//sel_tube = cmd_pack->buf[3];
	if(cmd_pack->buf[4] == 0x01)
	{
		sel_track = CAN_ADDR_EMER_TRACK;
		cmd = 0x0801;
		can_pack.buf[0] = cmd_pack->buf[3];
	}
	else if(cmd_pack->buf[4] == 0x02)
	{
		sel_track = CAN_ADDR_NORMAL_TRACK;
		cmd = 0x0901;
		can_pack.buf[0] = cmd_pack->buf[3];
	}
	local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 500);  
	
	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->len = 4;
	rsp_pack->buf[0] = 0;  //res ok
	rsp_pack->buf[1] = 0;  
	rsp_pack->buf[2] = 0;  
	rsp_pack->buf[3] = 0;  
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
		
}


void cmd_pop_shelf_handle(void *args)
{
	uint8_t sel_track = 0;
	uint16_t cmd = 0;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = (local_pack_t*)args;
	local_pack_t can_pack = {0};
	
	can_pack.buf = mes_recv_buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	
	//1.pop shelf step1, move sample shelf to track export
	if(cmd_pack->buf[4] == 0x01)
	{
		sel_track = CAN_ADDR_EMER_TRACK;
		cmd = 0x0701;
		can_pack.buf[0] = 3;
	}
	else if(cmd_pack->buf[4] == 0x02)
	{
		sel_track = CAN_ADDR_NORMAL_TRACK;
		cmd = 0x0601;
		can_pack.buf[0] = 2;
	}
	else if(cmd_pack->buf[4] == 0x03)
	{
		sel_track = CAN_ADDR_RECOVERY_TRACK;
	}
	local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 500);  //wait rof res
	
	
	//2.move change track machin to track export
	if(cmd_pack->buf[4] == 0x01) //CAN_ADDR_EMER_TRACK
	{
		can_pack.buf[0] = 1;
	}
	else if(cmd_pack->buf[4] == 0x02)
	{
		can_pack.buf[0] = 2;
	}
	local_create_cmd_pack(&can_pack, 0x0701, CAN_ADDR_RECOVERY_TRACK, &can_pack.buf[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 500);
	
	
	//3.push sample shelf to change track machin
	if(cmd_pack->buf[4] == 0x01)
	{
		sel_track = CAN_ADDR_EMER_TRACK;
		cmd = 0x0701;
		can_pack.buf[0] = 4;
	}
	else if(cmd_pack->buf[4] == 0x02)
	{
		sel_track = CAN_ADDR_NORMAL_TRACK;
		cmd = 0x0601;
		can_pack.buf[0] = 3;
	}
	else if(cmd_pack->buf[4] == 0x03)
	{
		sel_track = CAN_ADDR_RECOVERY_TRACK;
	}
	local_create_cmd_pack(&can_pack, cmd, sel_track, &can_pack.buf[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 500);
	
	//4.move change track machin to recover track entry
	can_pack.buf[0] = 3;
	local_create_cmd_pack(&can_pack, 0x0701, CAN_ADDR_RECOVERY_TRACK, &can_pack.buf[0], 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, 500);
	
	
	//5.push sample shelf to mini car from recover track
	
	
	//6.move mini car and push sample shelf to cache reg
	
	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->len = 4;
	rsp_pack->buf[0] = 0;  //res ok
	rsp_pack->buf[1] = 0;  
	rsp_pack->buf[2] = 0;  
	rsp_pack->buf[3] = 0;  
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);

			

}



void req_basket_start_test_handle(void *args)
{
	uint32_t dp = 0;
	uint8_t res = 0;
	local_pack_t *req_pack = RT_NULL;
	sample_shelf_t *sample_shelf = RT_NULL;
	local_pack_t rsp_pack = {0};
	
	rsp_pack.buf = mes_recv_buf;
	req_pack = (local_pack_t*)args;
	
	//1.get basket status
	rt_kprintf("\n===============step1.get basket status===============\n");
	cmd_get_basket_status(req_pack->buf[0]);
	
	//2.get sample shelf and scan sample info
	while(1)
	{
		if(rt_mq_recv(basket1_sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
		{
		
		}
		else if (rt_mq_recv(basket2_sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
		{
		
		}
		else if (rt_mq_recv(basket3_sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
		{
		
		}
		else
		{
			break;
		}
		
		sample_shelf = (sample_shelf_t*)dp;
		flow_scan_sample_shelf(sample_shelf);
		
		//3.wait for push shelf or push cache shelf cmd
		rt_kprintf("\n===============step5.wait for command...===============\n");
		while(1)
		{	
			if( local_wait_for_command(&rsp_pack, NET_CMD_MOVE_CACHE_REG, 0) ||  
				local_wait_for_command(&rsp_pack, NET_CMD_PUSH_SHELF, 0)) 
			{
				break;
			}
		}
		
		current_sample_shelf.slot_pos = sample_shelf->slot_pos;
		current_sample_shelf.basket_pos = sample_shelf->basket_pos;
		current_sample_shelf.shape_code[0] = sample_shelf->shape_code[0];
		current_sample_shelf.shape_code[1] = sample_shelf->shape_code[1];
		current_sample_shelf.shape_code[2] = sample_shelf->shape_code[2];
		
		current_sample_shelf.shape_code[0] = 0xEE;
		current_sample_shelf.shape_code[1] = 0x66;
		current_sample_shelf.shape_code[2] = 0x88;
		
		//4.push cache region or push shelf
		if(rsp_pack.cmd == NET_CMD_MOVE_CACHE_REG)
		{
			cmd_push_cache_reg_handle(sample_shelf);
		}
		else if(rsp_pack.cmd == NET_CMD_PUSH_SHELF)
		{
			cmd_push_shelf_handle(&rsp_pack);
			
//			//5.wait for move tube cmd
//			res = local_wait_for_command(&rsp_pack, NET_CMD_MOVE_SHELF, (rt_tick_t)RT_WAITING_FOREVER);
//			if(res)
//			{
//				cmd_move_shelf_handle(&rsp_pack);
//			}
//			
//			//6.wait for pop shelf cmd
//			res = local_wait_for_command(&rsp_pack, NET_CMD_POP_SHELF, (rt_tick_t)RT_WAITING_FOREVER);
//			if(res)
//			{
//				cmd_pop_shelf_handle(&rsp_pack);
//			}
		}
		
		current_sample_shelf.shape_code[0] = 0;
		current_sample_shelf.shape_code[1] = 0;
		current_sample_shelf.shape_code[2] = 0;
		if(sample_shelf)
		{
			rt_free(sample_shelf);
		}
		
	}
	
	
	
}


void req_basket_pause_test_handle(void *args)
{

	local_pack_t rsp_pack = {0};
	rsp_pack.buf = mes_recv_buf;

	local_wait_for_slave_req(&rsp_pack, REQ_EMERGY_START_TEST, (rt_tick_t)RT_WAITING_FOREVER);

}

void req_basket_update_handle(void *args)
{
	local_pack_t *req_pack = RT_NULL;
	req_pack = (local_pack_t*)args;
	
	cmd_get_basket_status(req_pack->buf[0]);

}

void req_basket_inser_sample_handle(void *args)
{
	uint32_t dp = 0;
	local_pack_t *req_pack = RT_NULL;
	sample_shelf_t *sample_shelf = RT_NULL;
	rt_mq_t sample_shelf_queue = RT_NULL;
	
	
	if(sample_shelf->basket_pos == 1)
	{
		sample_shelf_queue = basket1_sample_shelf_queue;
	}
	else if(sample_shelf->basket_pos == 2)
	{
		sample_shelf_queue = basket2_sample_shelf_queue;
	}
	else if(sample_shelf->basket_pos == 3)
	{
		sample_shelf_queue = basket3_sample_shelf_queue;
	}
	
	if(sample_shelf_queue == RT_NULL)
	{
		return;
	}
	
	req_pack = (local_pack_t*)args;
	sample_shelf = rt_malloc(sizeof(sample_shelf_t));
	sample_shelf->basket_pos = req_pack->buf[0];
	sample_shelf->slot_pos = req_pack->buf[1];
	dp = (uint32_t)sample_shelf;
	rt_mq_send(sample_shelf_queue, &dp, sizeof(void*));

}


void req_basket_remove_sample_handle(void *args)
{
	uint32_t dp = 0;
	local_pack_t *req_pack = RT_NULL;
	sample_shelf_t *sample_shelf = RT_NULL;
	rt_mq_t sample_shelf_queue = RT_NULL;
	
	
	if(sample_shelf->basket_pos == 1)
	{
		sample_shelf_queue = basket1_sample_shelf_queue;
	}
	else if(sample_shelf->basket_pos == 2)
	{
		sample_shelf_queue = basket2_sample_shelf_queue;
	}
	else if(sample_shelf->basket_pos == 3)
	{
		sample_shelf_queue = basket3_sample_shelf_queue;
	}
	
	if(sample_shelf_queue == RT_NULL)
	{
		return;
	}
	
	req_pack = (local_pack_t*)args;
	//basket2_sample_shelf_queue
	while(rt_mq_recv(sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
	{
		sample_shelf = (sample_shelf_t*)dp;
		if( sample_shelf->basket_pos == req_pack->buf[0] &&
		    sample_shelf->slot_pos == req_pack->buf[1])
		{
			rt_free(sample_shelf);
			break;
		}
		else
		{
			rt_mq_send(sample_shelf_queue, &dp, sizeof(void*));
		}
	
	}


}



void req_emer_start_test_handle(void *args)
{
	uint32_t dp = 0;
	uint8_t res = 0;
	local_pack_t *req_pack = RT_NULL;
	sample_shelf_t *sample_shelf = RT_NULL;
	local_pack_t rsp_pack = {0};
	
	rsp_pack.buf = mes_recv_buf;
	req_pack = (local_pack_t*)args;
	
	//1.get basket status
	cmd_get_basket_status(req_pack->buf[0]);
	
	//2.get sample shelf and scan sample info
	while(rt_mq_recv(emer_sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
	{
		sample_shelf = (sample_shelf_t*)dp;
		flow_scan_sample_shelf(sample_shelf);
		
		//3.wait for push shelf or push cache shelf cmd
		while(1)
		{	
			if( local_wait_for_command(&rsp_pack, NET_CMD_MOVE_CACHE_REG, 0) ||  
				local_wait_for_command(&rsp_pack, NET_CMD_PUSH_SHELF, 0)) 
			{	
				current_sample_shelf.slot_pos = sample_shelf->slot_pos;
				current_sample_shelf.basket_pos = sample_shelf->basket_pos;
				current_sample_shelf.shape_code[0] = sample_shelf->shape_code[0];
				current_sample_shelf.shape_code[1] = sample_shelf->shape_code[1];
				current_sample_shelf.shape_code[2] = sample_shelf->shape_code[2];
				break;
			}
		}
		
		//4.push cache region or push shelf
		if(rsp_pack.cmd == NET_CMD_MOVE_CACHE_REG)
		{
			cmd_push_cache_reg_handle(sample_shelf);
		}
		else if(rsp_pack.cmd == NET_CMD_PUSH_SHELF)
		{
			cmd_push_shelf_handle(&rsp_pack);
			
			//5.wait for move tube cmd
			res = local_wait_for_command(&rsp_pack, NET_CMD_MOVE_SHELF, (rt_tick_t)RT_WAITING_FOREVER);
			if(res)
			{
				cmd_move_shelf_handle(&rsp_pack);
			}
			
			//6.wait for pop shelf cmd
			res = local_wait_for_command(&rsp_pack, NET_CMD_POP_SHELF, (rt_tick_t)RT_WAITING_FOREVER);
			if(res)
			{
				cmd_pop_shelf_handle(&rsp_pack);
			}
		}
		
		if(sample_shelf)
		{
			rt_free(sample_shelf);
		}
		
	}
	
	
	
}


void req_emer_pause_test_handle(void *args)
{

	local_pack_t rsp_pack = {0};
	rsp_pack.buf = mes_recv_buf;

	local_wait_for_slave_req(&rsp_pack, REQ_EMERGY_START_TEST, (rt_tick_t)RT_WAITING_FOREVER);

}


void req_emer_inser_sample_handle(void *args)
{
	uint32_t dp = 0;
	local_pack_t *req_pack = RT_NULL;
	sample_shelf_t *sample_shelf = RT_NULL;

	
	req_pack = (local_pack_t*)args;
	sample_shelf = rt_malloc(sizeof(sample_shelf_t));
	sample_shelf->basket_pos = req_pack->buf[0];
	sample_shelf->slot_pos = req_pack->buf[1];
	dp = (uint32_t)sample_shelf;
	rt_mq_send(emer_sample_shelf_queue, &dp, sizeof(void*));

}


void req_emer_remove_sample_handle(void *args)
{
	uint32_t dp = 0;
	local_pack_t *req_pack = RT_NULL;
	sample_shelf_t *sample_shelf = RT_NULL;

	
	req_pack = (local_pack_t*)args;
	//basket2_sample_shelf_queue
	while(rt_mq_recv(emer_sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
	{
		sample_shelf = (sample_shelf_t*)dp;
		if( sample_shelf->basket_pos == req_pack->buf[0] &&
		    sample_shelf->slot_pos == req_pack->buf[1])
		{
			rt_free(sample_shelf);
			break;
		}
		else
		{
			rt_mq_send(emer_sample_shelf_queue, &dp, sizeof(void*));
		}
	
	}


}







void dev_reset(void)
{

	local_pack_t can_pack = {0};
	mes_recv_buf[0] = 1;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_SAMPLE_PLTAFROM, mes_recv_buf, 1);  
	can_device_send_pack(&can_pack);
	
	if(can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER))  //wait for res
	{
		mes_recv_buf[0] = 2;
		local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_SAMPLE_PLTAFROM, mes_recv_buf, 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait for res
	}
	
	
	
}



void dev_scan_test(void)
{
	sample_shelf_t sample_shelf = {0};
	
	sample_shelf.basket_pos = 2;
	sample_shelf.slot_pos = 1;
	flow_scan_sample_shelf(&sample_shelf);
	
	
		

}


void dev_start_test(void)
{
	local_pack_t pack = {0};
	pack.buf = mes_recv_buf;
	flow_create_cache_region();
	
	pack.buf[0] = 0x02;
	req_basket_start_test_handle(&pack);
	
	

}



MSH_CMD_EXPORT(dev_reset, "dev_reset_cmd");
MSH_CMD_EXPORT(dev_scan_test, "dev_scan_test_cmd");
MSH_CMD_EXPORT(dev_start_test, "dev_start_test");
