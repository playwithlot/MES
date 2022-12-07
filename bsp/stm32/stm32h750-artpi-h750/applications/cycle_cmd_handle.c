#include "cycle_cmd_handle.h"




rt_mq_t basket1_sample_shelf_queue = RT_NULL;
rt_mq_t basket2_sample_shelf_queue = RT_NULL;
rt_mq_t basket3_sample_shelf_queue = RT_NULL;
rt_mq_t emer_sample_shelf_queue = RT_NULL;



static uint8_t is_min_car_busy = 0;
static cache_region_t cache_reg_table[MES_CACHE_REGION_NUM] = {0};



extern uint8_t local_wait_for_command(local_pack_t *pack, uint32_t cmd, rt_tick_t timeout);
extern uint8_t local_wait_for_slave_req(local_pack_t *pack, uint32_t cmd, rt_tick_t timeout);
extern void local_create_cmd_pack(local_pack_t *pack, uint32_t cmd, uint8_t addr, uint8_t *param, uint8_t len);



void cmd_get_basket_status(uint8_t basket_num)
{
	uint8_t res = 0;
	uint32_t dp = 0;
	uint8_t buf[6] = {0};
	uint16_t basket_stasut = 0;
	local_pack_t can_pack = {0};
	sample_shelf_t *sample_shelf = RT_NULL;
	rt_mq_t sample_shelf_queue = RT_NULL;
	
	can_pack.buf = buf;
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
	res = can_device_get_rsp_pack(&can_pack, can_pack.cmd,  PACK_TYPE_DATA, 300);
	
	if(res)
	{
		basket_stasut = *((uint16_t*)&can_pack.buf[2]);  //cmd: D0 D1 D3 == basket D4D5 == status
	}
	
	
	basket_stasut = 0x001F;   
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
	uint8_t recv_len = 0;

	scanner_dev_clean_stream();
	scanner_dev_ctrl(SCANNER_CMD_start, SCANNER_TYPE_leuze_BCL95, SCANNER_MODE_leuze_once);  //send start scan command
	recv_len = scanner_dev_get_data(buf, maxlen, 30);


	return recv_len;
	
} 


void flow_create_cache_region(void)
{

	for(uint8_t i = 0; i < MES_CACHE_REGION_NUM; i++)
	{
		cache_reg_table[i].is_empty = 1;
		cache_reg_table[i].locate_pos = i;
		
		cache_reg_table[i].sample.basket_pos = 0;
		cache_reg_table[i].sample.slot_pos = 0;
		cache_reg_table[i].sample.is_done = 0;
		cache_reg_table[i].sample.shape_code[0] = 0;
		cache_reg_table[i].sample.shape_code[1] = 0;
		cache_reg_table[i].sample.shape_code[2] = 0;

	}

}

int push_cache_store(sample_shelf_t *src_smp)
{
	for(uint8_t i = 0; i < MES_CACHE_STORE_NUM; i++)
	{
		if(cache_reg_table[i].is_empty)
		{
			cache_reg_table[i].is_empty = 0;
			rt_memcpy(&cache_reg_table[i].sample, src_smp, sizeof(sample_shelf_t));
			rt_memset(src_smp, 0, sizeof(sample_shelf_t));
			return cache_reg_table[i].locate_pos;
		}
		
	}
	
	return -1;
	
}

int push_cache_pos(sample_shelf_t *src_smp, uint8_t pos)
{
	if(pos >= MES_CACHE_REGION_NUM)
	{
		return -1;
	}
	
	cache_reg_table[pos].is_empty = 0;
	cache_reg_table[pos].locate_pos = pos;
	rt_memcpy(&cache_reg_table[pos].sample, src_smp, sizeof(sample_shelf_t));
	rt_memset(src_smp, 0, sizeof(sample_shelf_t));
	
	return cache_reg_table[pos].locate_pos;

}





int search_cache_region(sample_shelf_t *dest_smp)
{
	for(uint8_t i = 0; i < MES_CACHE_REGION_NUM; i++)
	{
		if( dest_smp->shape_code[0] == cache_reg_table[i].sample.shape_code[0] &&
			dest_smp->shape_code[1] == cache_reg_table[i].sample.shape_code[1] &&
			dest_smp->shape_code[2] == cache_reg_table[i].sample.shape_code[2] && 
			cache_reg_table[i].sample.is_done == 0)
		{
			return cache_reg_table[i].locate_pos;
		}
		
	}
	
	return -1;
	
}





static uint8_t smp_shape_code_buf[256] = {};
void flow_scan_sample_shelf(sample_shelf_t *sample_shelf)
{
	uint8_t res_cnt = 0;
	uint8_t i = 0;
	uint8_t len = 0;
	uint8_t index = 0;
	uint8_t sample_num = 0;
	uint8_t buf[6] = {0};
	uint8_t param[3] = {0};
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t can_pack = {0};
	can_pack.buf = buf;
	
	
	rt_memset(smp_shape_code_buf, 0, 256);
	//if(rt_mq_recv(sample_location_queue, &dp, sizeof(void*), 0) == RT_EOK)
	

	//sample_shelf = (sample_shelf_t*)dp;
	//rt_kprintf("=============[bastet %d] [slot %d]=============\n", sample_shelf->basket_pos, sample_shelf->slot_pos);
	
	//2.set led status 
	rt_kprintf("\n===============step2.set indicate led===============\n");
	param[0] = sample_shelf->basket_pos;
	param[1] = sample_shelf->slot_pos;
	param[2] = 0x02;  //led status: wait for test, color: red
	local_create_cmd_pack(&can_pack, 0x0601, CAN_ADDR_SAMPLE_STATUS, &param[0], 3);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)100);  //wait for res
	
	//rt_thread_delay(5);
	
	//3.set motor position: get sample shelf 
	rt_kprintf("\n===============step3.get sample shelf===============\n");
	param[0] = sample_shelf->basket_pos;
	param[1] = sample_shelf->slot_pos;
	local_create_cmd_pack(&can_pack, 0x0a01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
	
	
	//6.scan sample shelf shape code: set sample tube scan pos
	param[0] = 7;
	local_create_cmd_pack(&can_pack, 0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait for res
	rt_thread_delay(80);
	len = flow_scan_sample(smp_shape_code_buf, 20);   //2.scan sample shelf	shape code

	//sample_shelf->shape_code
	if(len)
	{
		rt_kprintf("smp shelf size %d: %s\n", len, smp_shape_code_buf);
	}
	
	/*
	sample_shelf->shape_code[0] = 0xEE;
	sample_shelf->shape_code[1] = 0x66;
	sample_shelf->shape_code[2] = sample_shelf->slot_pos+0x87;
	*/
	
	sample_num = 0;
	index = 0;
	
	rt_kprintf("\n===============step4.scan sample[%d] info===============\n", i+1);
	for(i = 0; i < 10; i++)
	{	
		#if 0
		//5.check the sanmple is exist
		rt_kprintf("4.2 check is sample [%d] exist?\n", i+1);
		param[0] = 0x06; 
		local_create_cmd_pack(&can_pack, 0x2904, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);  //0x41
		can_device_send_pack(&can_pack);
		if(can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)50))
		{
			//6.scan smple and save shape code
			if(!can_pack.buf[1])
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
		#else

		
		//4.set sample scan position: set sample tube scan pos
		rt_kprintf("4.1 set sample[%d] pos\n", i+1);
		param[0] = 0x06; 
		param[1] = i+1;
		local_create_cmd_pack(&can_pack, 0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		res_cnt += can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		rt_thread_delay(80);
		len = flow_scan_sample(&smp_shape_code_buf[index], 20); //2.scan sample
		
		
		if(len)
		{
			index += len;
			sample_num++;
		}
		#endif
		

	
		
		
		
		
				
	}
	
		
	
	
	//7.push all sample to car : call mini car pos
	param[0] = 8;
	local_create_cmd_pack(&can_pack, 0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);  //get basket status cmd
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
	
	
	//8.upload result to network, report frame
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(index+5);
	rsp_pack->len = index + 5;
	if(rsp_pack && rsp_pack->buf)  
	{
		rsp_pack->cmd = 0x0405;
		rsp_pack->type = PACK_TYPE_REPORT;
		rsp_pack->buf[0] = sample_shelf->basket_pos;
		rsp_pack->buf[1] = sample_shelf->slot_pos;
		rt_memcpy(&rsp_pack->buf[2], sample_shelf->shape_code, 3);
		rt_memcpy(&rsp_pack->buf[5], smp_shape_code_buf, rsp_pack->len);
		rt_kprintf("total sam num: %d rsp: %d len: %d\n%s\ntotal size: %d\n", sample_num, res_cnt, len, &rsp_pack->buf[5], rsp_pack->len);
		local_send_response(rsp_pack); 
			
	}
	

	//9.wait for network command: push shelf of push to cache region
		
}

void cmd_push_cache_reg_handle(void *args)
{
	//9.wait for network command: push shelf of push to cache region
	int smp_pos = -1;
	uint8_t err = 1;
	int cache_reg = 0;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t can_pack = {0};
	uint8_t buf[6] = {0};
	uint8_t param[3] = {0};
	can_pack.buf = buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf =  rt_malloc(4);
	
	
	if(cache_reg_table[MES_MIN_CAR_CACHE_POS].sample.slot_pos)
	{
		//rt_kprintf("mes: push_shelf command: %0x4X\n", cmd_pack->cmd);
		rt_kprintf("\n===============step7.push sample shelf to cache reg===============\n");
		rsp_pack->buf[3] = 1;  //res err
		
		//10~11.push shelf to cache region
		cache_reg_table[MES_MIN_CAR_CACHE_POS].is_empty = 1;
		cache_reg = push_cache_store(&cache_reg_table[MES_MIN_CAR_CACHE_POS].sample);
		
		if(cache_reg >= MES_NIM_CACHE_STORE_POS)
		{
			rsp_pack->buf[3] = 0;  //res ok
			param[0] = 4;
			param[1] = cache_reg + 1;
			local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)5000);  //wait rof res
			err = 0;
			is_min_car_busy = 0;
		}
	
	}

	rsp_pack->cmd = NET_CMD_MOVE_CACHE_REG;
	rsp_pack->len = 4;
	rsp_pack->buf[3] = err;
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
	
}


#if 0
void cmd_push_shelf_handle(void *args)
{
	uint8_t err = 1;
	uint16_t cmd = 0;
	int cache_reg = 0;
	uint8_t sel_track = 0;
	uint8_t buf[6] = {0};
	uint8_t param[3] = {0};
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = RT_NULL;
	local_pack_t can_pack = {0};
	sample_shelf_t dest_sample_shelf = {0};
	
	cmd_pack = (local_pack_t*)args;
	can_pack.buf = buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	
	dest_sample_shelf.shape_code[0] = cmd_pack->buf[0];
	dest_sample_shelf.shape_code[1] = cmd_pack->buf[1];
	dest_sample_shelf.shape_code[2] = cmd_pack->buf[2];
 
	//rt_kprintf("mes: push_shelf command: %0x4X\n", cmd_pack->cmd);
	rt_kprintf("\n===============step7.push sample shelf track entry===============\n");
	
	//12.push shelf to other track
	//sample on track cache pos
	if( dest_sample_shelf.shape_code[0] == track_cache_pos_sample.shape_code[0] &&
		dest_sample_shelf.shape_code[1] == track_cache_pos_sample.shape_code[1] &&
		dest_sample_shelf.shape_code[2] == track_cache_pos_sample.shape_code[2] &&
	    track_add_smp_pos_sample.basket_pos == 0 && track_add_smp_pos_sample.slot_pos == 0)
	{
		//5.push sample shelf to addtion sample pos [04: 01 06 01]
		//cmd: normal track enter sample shelf
		if(cmd_pack->buf[5] == 0x01)  //add sample pos
		{ 
			if(cmd_pack->buf[4] == 0x01) //get track index
			{
				sel_track = CAN_ADDR_EMER_TRACK;
				cmd = 0x0701;
				param[0] = 2;
			}
			else if(cmd_pack->buf[4] == 0x02)
			{
				sel_track = CAN_ADDR_NORMAL_TRACK;
				cmd = 0x0601;
				param[0] = 1;
			}
			
			local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
			
			rt_memcpy(&track_add_smp_pos_sample, &track_cache_pos_sample, sizeof(sample_shelf_t));
			rt_memset(&track_cache_pos_sample, 0, sizeof(sample_shelf_t));
			err = 0;
		}
		
	
	}
	//sample on mini car 
	else if( dest_sample_shelf.shape_code[0] == mini_car_pos_sample.shape_code[0] &&
		     dest_sample_shelf.shape_code[1] == mini_car_pos_sample.shape_code[1] &&
		     dest_sample_shelf.shape_code[2] == mini_car_pos_sample.shape_code[2] &&
			 track_cache_pos_sample.basket_pos == 0 && track_cache_pos_sample.slot_pos == 0)
	{
		//2.push sample shelf to track cache pos: step1
		//cmd:call min car trans sample shelf
		param[0] = 6; //cmd_pack->buf[0] ~ [2] shape code
		param[1] = cmd_pack->buf[4];  //1: emer track 2: normal track 3:recyc track
		local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		

		//3.push sample shelf to track cache pos: step2 
		//cmd: normal track prepare sample shelf
		if(cmd_pack->buf[4] == 0x01)   //1: emer track 2: normal track 3:recyc track
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			param[0] = 1;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0601;
			param[0] = 1;
		}
		//cmd.push sample shelf to track cache pos [03: 01 06 01]
		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		rt_memcpy(&track_cache_pos_sample, &mini_car_pos_sample, sizeof(sample_shelf_t));
		
		
		
		//5.push sample shelf to addtion sample pos [04: 01 06 01]
		//cmd: normal track enter sample shelf
		if(cmd_pack->buf[5] == 0x01)  //add sample pos
		{ 
			if(cmd_pack->buf[4] == 0x01) //get track index
			{
				sel_track = CAN_ADDR_EMER_TRACK;
				cmd = 0x0701;
				param[0] = 2;
			}
			else if(cmd_pack->buf[4] == 0x02)
			{
				sel_track = CAN_ADDR_NORMAL_TRACK;
				cmd = 0x0601;
				param[0] = 1;
			}
			
			local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
			
			rt_memcpy(&track_add_smp_pos_sample, &track_cache_pos_sample, sizeof(sample_shelf_t));
			rt_memset(&track_cache_pos_sample, 0, sizeof(sample_shelf_t));
			
			
		}
		
		err = 0;
	
	}
	//push shelf from cache reg
	else if( track_cache_pos_sample.basket_pos == 0 && track_cache_pos_sample.slot_pos == 0)
	{
		cache_reg = pop_cache_region(&dest_sample_shelf);
		if(cache_reg)
		{
			rt_memcpy(&mini_car_pos_sample, &dest_sample_shelf, sizeof(sample_shelf_t));
			//1.get sample shelf from cache reg
			param[0] = 4;
			param[1] = cache_reg;
			local_create_cmd_pack(&can_pack, 0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
			
			//2.push sample shelf to track cache pos: step1
			//cmd:call min car trans sample shelf
			param[0] = 6; //cmd_pack->buf[0] ~ [2] shape code
			param[1] = cmd_pack->buf[4];  //1: emer track 2: normal track 3:recyc track
			local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
			

			//3.push sample shelf to track cache pos: step2 
			//cmd: normal track prepare sample shelf
			if(cmd_pack->buf[4] == 0x01)   //1: emer track 2: normal track 3:recyc track
			{
				sel_track = CAN_ADDR_EMER_TRACK;
				cmd = 0x0701;
				param[0] = 1;
			}
			else if(cmd_pack->buf[4] == 0x02)
			{
				sel_track = CAN_ADDR_EMER_TRACK;
				cmd = 0x0601;
				param[0] = 1;
			}
			//cmd.push sample shelf to track cache pos [03: 01 06 01]
			local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
			rt_memcpy(&track_cache_pos_sample, &mini_car_pos_sample, sizeof(sample_shelf_t));
			
			
			
			//5.push sample shelf to addtion sample pos [04: 01 06 01]
			//cmd: normal track enter sample shelf
			if(cmd_pack->buf[5] == 0x01)  //add sample pos
			{ 
				if(cmd_pack->buf[4] == 0x01) //get track index
				{
					sel_track = CAN_ADDR_EMER_TRACK;
					cmd = 0x0701;
					param[0] = 2;
				}
				else if(cmd_pack->buf[4] == 0x02)
				{
					sel_track = CAN_ADDR_NORMAL_TRACK;
					cmd = 0x0601;
					param[0] = 1;
				}
				
				local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  //get basket status cmd
				can_device_send_pack(&can_pack);
				can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
				
				rt_memcpy(&track_add_smp_pos_sample, &track_cache_pos_sample, sizeof(sample_shelf_t));
				rt_memset(&track_cache_pos_sample, 0, sizeof(sample_shelf_t));
				
			}

			err = 0;
		}
		
	}

	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->len = 4;
	rsp_pack->buf[3] = err;
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
				
}
	
#endif 




uint8_t push_to_cache_pos(local_pack_t* cmd_pack)
{
	uint8_t err = 1;
	int smp_pos = -1;
	int cache_store = -1;
	uint8_t sel_track = 0;
	uint16_t cmd = 0;
	uint8_t param[3] = {0};
	local_pack_t can_pack = {0};
	uint8_t buf[6] = {0};
	sample_shelf_t dest_samp = {0};
	
	can_pack.buf = buf;
	dest_samp.shape_code[0] = cmd_pack->buf[0];
	dest_samp.shape_code[1] = cmd_pack->buf[1];
	dest_samp.shape_code[2] = cmd_pack->buf[2];
	
	
	if(cache_reg_table[MES_TRACK_CACHE_POS].sample.slot_pos) //if tr cache pos not idle
	{
		return err;
	}
	
	smp_pos  = search_cache_region(&dest_samp);
	if(smp_pos == MES_MIN_CAR_CACHE_POS)
	{
		//2.push sample shelf to track cache pos: step1
		//cmd:call min car trans sample shelf
		param[0] = 6; //cmd_pack->buf[0] ~ [2] shape code
		param[1] = cmd_pack->buf[4];  //1: emer track 2: normal track 3:recyc track
		local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		

		//3.push sample shelf to track cache pos: step2 
		//cmd: normal track prepare sample shelf
		if(cmd_pack->buf[4] == 0x01)   //1: emer track 2: normal track 3:recyc track
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			param[0] = 1;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0601;
			param[0] = 1;
		}
		//cmd.push sample shelf to track cache pos [03: 01 06 01]
		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		
		cache_reg_table[smp_pos].is_empty = 1;
		push_cache_pos(&cache_reg_table[smp_pos].sample, MES_TRACK_CACHE_POS);
		err = 0;
		
	}
	else if(smp_pos >= MES_NIM_CACHE_STORE_POS && smp_pos < MES_CACHE_STORE_NUM)
	{
		if(is_min_car_busy)
		{
			//1.push shelf to cache region
			cache_reg_table[MES_MIN_CAR_CACHE_POS].is_empty = 1;
			cache_store = push_cache_store(&cache_reg_table[MES_MIN_CAR_CACHE_POS].sample);   //
			if(cache_store == -1)
			{
				return err;
			}
			
			param[0] = 4;
			param[1] = cache_store+1;
			local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)5000);  //wait rof res
			is_min_car_busy = 0;
			
		}
		
		//1.get sample shelf from cache reg
		param[0] = 4;
		param[1] = smp_pos+1;
		local_create_cmd_pack(&can_pack, 0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		
		
		//2.push sample shelf to track cache pos: step1
		//cmd:call min car trans sample shelf
		param[0] = 6; //cmd_pack->buf[0] ~ [2] shape code
		param[1] = cmd_pack->buf[4];  //1: emer track 2: normal track 3:recyc track
		local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		

		//3.push sample shelf to track cache pos: step2 
		//cmd: normal track prepare sample shelf
		if(cmd_pack->buf[4] == 0x01)   //1: emer track 2: normal track 3:recyc track
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			param[0] = 1;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0601;
			param[0] = 1;
		}
		//cmd.push sample shelf to track cache pos [03: 01 06 01]
		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		
		cache_reg_table[smp_pos].is_empty = 1;
		push_cache_pos(&cache_reg_table[smp_pos].sample, MES_TRACK_CACHE_POS);
		
		err = 0;

	}
	
	return err;
	

}


uint8_t push_to_add_smp_pos(local_pack_t* cmd_pack)
{
	uint8_t err = 1;
	int smp_pos = -1;
	uint8_t sel_track = 0;
	uint16_t cmd = 0;
	uint8_t param[3] = {0};
	local_pack_t can_pack = {0};
	uint8_t buf[6] = {0};
	sample_shelf_t dest_samp = {0};
	
	can_pack.buf = buf;
	dest_samp.shape_code[0] = cmd_pack->buf[0];
	dest_samp.shape_code[1] = cmd_pack->buf[1];
	dest_samp.shape_code[2] = cmd_pack->buf[2];
	
	
	smp_pos  = search_cache_region(&dest_samp);
	if(smp_pos == MES_TRACK_CACHE_POS)
	{
		//5.push sample shelf to addtion sample pos [04: 01 06 01]
		//cmd: normal track enter sample shelf
		if(cmd_pack->buf[4] == 0x01) //get track index
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			param[0] = 2;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_NORMAL_TRACK;
			cmd = 0x0601;
			param[0] = 1;
		}

		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		
		cache_reg_table[smp_pos].is_empty = 1;
		push_cache_pos(&cache_reg_table[smp_pos].sample, MES_TRACK_ADD_SMP_POS);
		err = 0;
			
	}
	
	return err;

}



void cmd_push_shelf_handle(void *args)
{
	uint8_t err = 1;
	uint16_t cmd = 0;
	uint8_t cache_reg = 0;
	uint8_t sel_track = 0;

	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = RT_NULL;
	
	cmd_pack = (local_pack_t*)args;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	

 
	//rt_kprintf("mes: push_shelf command: %0x4X\n", cmd_pack->cmd);
	rt_kprintf("\n===============step7.push sample shelf track entry===============\n");
	

	if(cmd_pack->buf[5] == 0x01)  //push shelf to add sample pos
	{
		if(cache_reg_table[MES_TRACK_ADD_SMP_POS].sample.slot_pos == 0) //1.add smp idle
		{
			err = push_to_cache_pos(cmd_pack);
			err = push_to_add_smp_pos(cmd_pack);
		}
		
	}
	else if(cmd_pack->buf[5] == 0x02) //push shelf to cache pos
	{
		err = push_to_cache_pos(cmd_pack);
	
	}
	
	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->len = 4;
	rsp_pack->buf[3] = err;
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
	
	if(err == 0)
	{
		is_min_car_busy = 0;
	}
		
		
}
	

void cmd_move_shelf_handle(void *args)
{
	uint8_t err = 1;
	uint8_t sel_track = 0;
	uint16_t cmd = 0;
	uint8_t buf[6] = {0};
	uint8_t param[2] = {0};
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = (local_pack_t*)args;
	local_pack_t can_pack = {0};

	can_pack.buf = buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	
	//1.move test tube of sample shelf
	//sel_tube = cmd_pack->buf[3];
	//cmd:
	rt_kprintf("\n===============step9.move sample shelf===============\n");
	if(cache_reg_table[MES_TRACK_ADD_SMP_POS].sample.slot_pos)
	{
		if(cmd_pack->buf[1] == 0x01)  //get track type: 0x01:emer 0x02:normal 0x03:recover
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0801;
			param[0] = cmd_pack->buf[3];
			
		}
		else if(cmd_pack->buf[1] == 0x02) //0x02:normal
		{
			sel_track = CAN_ADDR_NORMAL_TRACK;
			cmd = 0x0901;
			param[0] = cmd_pack->buf[3];

		}
		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  
		err = 0;
	
	}
	
	
	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->buf[3] = err;  
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
		
}



void cmd_pop_shelf_handle(void *args)
{
	int smp_pos = -1;
	int cache_store = -1;
	uint8_t err = 1;
	uint8_t sel_track = 0;
	uint16_t cmd = 0;
	uint8_t buf[6] = {0};
	uint8_t param[3] = {0};
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = (local_pack_t*)args;
	local_pack_t can_pack = {0};
	sample_shelf_t dest_samp = {0};
	
	can_pack.buf = buf;
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(4);
	
	dest_samp.shape_code[0] = cmd_pack->buf[0];
	dest_samp.shape_code[1] = cmd_pack->buf[1];
	dest_samp.shape_code[2] = cmd_pack->buf[2];
	
	rt_kprintf("\n===============step11.pop sample shelf===============\n");
	
	smp_pos  = search_cache_region(&dest_samp);
	if(smp_pos == MES_TRACK_ADD_SMP_POS)
	{
		if(is_min_car_busy)
		{
			//1.push shelf to cache region
			cache_reg_table[MES_MIN_CAR_CACHE_POS].is_empty = 1;
			cache_store = push_cache_store(&cache_reg_table[MES_MIN_CAR_CACHE_POS].sample);
			if(cache_store == -1)
			{
				return;
			}
			
			rsp_pack->buf[3] = 0;  //res ok
			param[0] = 4;
			param[1] = cache_store+1;
			local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  //get basket status cmd
			can_device_send_pack(&can_pack);
			can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)5000);  //wait rof res
			is_min_car_busy = 0;
			
		}
		
		cache_reg_table[MES_TRACK_ADD_SMP_POS].sample.is_done = 1;
		cache_reg_table[MES_TRACK_ADD_SMP_POS].is_empty = 1;
		cache_store = push_cache_store(&cache_reg_table[MES_TRACK_ADD_SMP_POS].sample);
		if(cache_store == -1)
		{
			return;
		}
		
		
		//1.pop shelf step1, move sample shelf to track export
		//cmd: track out sample shelf
		if(cmd_pack->buf[4] == 0x01)
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			param[0] = 3;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_NORMAL_TRACK;
			cmd = 0x0601;
			param[0] = 2;
		}

		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait rof res
		
		
		//2.move change track machin to track export
		//cmd: change track pos moto, change track to x track
		if(cmd_pack->buf[4] == 0x01) //CAN_ADDR_EMER_TRACK
		{
			param[0] = 1;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			param[0] = 2;
		}
		local_create_cmd_pack(&can_pack, 0x0701, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		
		//3.push sample shelf to change track machine
		//cmd: track move sample shelf to change track machine
		if(cmd_pack->buf[4] == 0x01)
		{
			sel_track = CAN_ADDR_EMER_TRACK;
			cmd = 0x0701;
			param[0] = 4;
		}
		else if(cmd_pack->buf[4] == 0x02)
		{
			sel_track = CAN_ADDR_NORMAL_TRACK;
			cmd = 0x0601;
			param[0] = 3;
		}
		local_create_cmd_pack(&can_pack, cmd, sel_track, &param[0], 1);  //get basket status cmd
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		//4.move change track machine to recover track entry pos
		//cmd:change track pos moto change track to recovery pos
		param[0] = 3;
		local_create_cmd_pack(&can_pack, 0x0701, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		
		
		//5.push sample shelf to mini car from recover track
		param[0] = 1;
		local_create_cmd_pack(&can_pack, 0x0601, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		#if 1
		//6.call mini char trans back sample shelf to basket 
		//cmd: call mini car Y moto to basket3 num13
		param[0] = 6;
		param[1] = 3;
		local_create_cmd_pack(&can_pack, 0x0e01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		#endif
		
		
		//7.move min car to recovery entry to prepare get sample shelf
		//cmd: call min car get sample shelf from recovery track 
		param[0] = 6;
		local_create_cmd_pack(&can_pack, 0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);


		//8.
		param[0] = 2;
		local_create_cmd_pack(&can_pack, 0x0601, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		//9.
		param[0] = 3;
		param[1] = 13;
		local_create_cmd_pack(&can_pack, 0x0e01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		//10.
		param[0] = 1;
		local_create_cmd_pack(&can_pack, 0x1101, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
		
		//11. goback cache store
		param[0] = 4;             //go back cache reg
		param[1] = cache_store+1;  
		local_create_cmd_pack(&can_pack, 0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);  
		can_device_send_pack(&can_pack);
		can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);

		err = 0;
		
		
	}
	
	//response 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->len = 4;
	rsp_pack->buf[3] = err;  
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
	
	
}



void req_basket_start_test_handle(void *args)
{
	uint8_t buf[6] = {0};
	local_pack_t *req_pack = RT_NULL;
	local_pack_t rsp_pack = {0};
	rsp_pack.buf = buf;
	req_pack = (local_pack_t*)args;
	

	//1.get basket status
	rt_kprintf("\n===============step1.get basket status===============\n");
 	cmd_get_basket_status(req_pack->buf[0]);
	

}


void scan_sample_handle(void)
{
	uint8_t res = 0;
	uint32_t dp = 0;
	uint8_t param[3] = {0};
	sample_shelf_t *sample_shelf = RT_NULL;
	
	//1.check is exsit sample shelf in call car cache pos
	if(is_min_car_busy)
	{
		return;
	}
	
	//2.get sample shelf and scan sample info
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
		return;
	}
		
	is_min_car_busy = 1;
	sample_shelf = (sample_shelf_t*)dp;
	flow_scan_sample_shelf(sample_shelf);
	
	
	sample_shelf->is_done = 0;
	
	push_cache_pos(sample_shelf, MES_MIN_CAR_CACHE_POS);
	
	if(sample_shelf)
	{
		rt_free(sample_shelf);
	}
	

		

}




void req_basket_pause_test_handle(void *args)
{
	uint8_t buf[6] = {0};
	local_pack_t rsp_pack = {0};
	rsp_pack.buf = buf;

	local_wait_for_slave_req(&rsp_pack, REQ_EMERGY_START_TEST, (rt_tick_t)MES_WAITING_MAX);

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


static uint8_t cycle_cmd_tem_buf[50];
void req_emer_start_test_handle(void *args)
{
//	uint32_t dp = 0;
//	uint8_t res = 0;
//	uint8_t buf[6] = {0};
//	uint8_t param[3] = {0};
//	local_pack_t *req_pack = RT_NULL;
//	sample_shelf_t *sample_shelf = RT_NULL;
//	local_pack_t rsp_pack = {0};
//	
//	rsp_pack.buf = cycle_cmd_tem_buf;
//	req_pack = (local_pack_t*)args;
//	
//	//1.get basket status
//	cmd_get_basket_status(req_pack->buf[0]);
//	
//	//2.get sample shelf and scan sample info
//	while(rt_mq_recv(emer_sample_shelf_queue, &dp, sizeof(void*), 0) == RT_EOK)
//	{
//		sample_shelf = (sample_shelf_t*)dp;
//		flow_scan_sample_shelf(sample_shelf);
//		
//		//3.wait for push shelf or push cache shelf cmd
//		while(1)
//		{	
//			if( local_wait_for_command(&rsp_pack, NET_CMD_MOVE_CACHE_REG, 0) ||  
//				local_wait_for_command(&rsp_pack, NET_CMD_PUSH_SHELF, 0)) 
//			{	
//				mini_car_pos_sample.slot_pos = sample_shelf->slot_pos;
//				mini_car_pos_sample.basket_pos = sample_shelf->basket_pos;
//				mini_car_pos_sample.shape_code[0] = sample_shelf->shape_code[0];
//				mini_car_pos_sample.shape_code[1] = sample_shelf->shape_code[1];
//				mini_car_pos_sample.shape_code[2] = sample_shelf->shape_code[2];
//				break;
//			}
//		}
//		
//		//4.push cache region or push shelf
//		if(rsp_pack.cmd == NET_CMD_MOVE_CACHE_REG)
//		{
//			cmd_push_cache_reg_handle(sample_shelf);
//		}
//		else if(rsp_pack.cmd == NET_CMD_PUSH_SHELF)
//		{
//			cmd_push_shelf_handle(&rsp_pack);
//			
//			//5.wait for move tube cmd
//			res = local_wait_for_command(&rsp_pack, NET_CMD_MOVE_SHELF, (rt_tick_t)MES_WAITING_MAX);
//			if(res)
//			{
//				cmd_move_shelf_handle(&rsp_pack);
//			}
//			
//			//6.wait for pop shelf cmd
//			res = local_wait_for_command(&rsp_pack, NET_CMD_POP_SHELF, (rt_tick_t)MES_WAITING_MAX);
//			if(res)
//			{
//				cmd_pop_shelf_handle(&rsp_pack);
//			}
//		}
//		
//		if(sample_shelf)
//		{
//			rt_free(sample_shelf);
//		}
//		
//	}
//	
//	
	
}


void req_emer_pause_test_handle(void *args)
{

	local_pack_t rsp_pack = {0};
	rsp_pack.buf = cycle_cmd_tem_buf;

	local_wait_for_slave_req(&rsp_pack, REQ_EMERGY_START_TEST, (rt_tick_t)MES_WAITING_MAX);

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







void cmd_dev_reset_handle(void *args)
{
	uint8_t buf[6] = {0};
	uint8_t param[2] = {0};
	local_pack_t can_pack = {0};
	can_pack.buf = buf;

#if 1	
	//1.recover track
	param[0] = 1;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_RECOVERY_TRACK, param, 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)2000);
	
	//2.sample table
	param[0] = 1;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_SAMPLE_PLTAFROM, param, 1);  
	can_device_send_pack(&can_pack);
	
	//3.emer track
	param[0] = 1;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_EMER_TRACK, param, 1);  
	can_device_send_pack(&can_pack);
	
	//4.normal track
	param[0] = 1;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_NORMAL_TRACK, param, 1);  
	can_device_send_pack(&can_pack);
	
	
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX); 
	
	param[0] = 2;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_SAMPLE_PLTAFROM, param, 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait for res

	param[0] = 2;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_EMER_TRACK, param, 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait for res

	param[0] = 2;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_NORMAL_TRACK, param, 1);  
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)MES_WAITING_MAX);  //wait for res

	param[0] = 2;
	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_RECOVERY_TRACK, param, 1); 
	can_device_send_pack(&can_pack);
	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)2000);  //wait for res
	
#endif	
	
//	//2.sample table
//	param[0] = 1;
//	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_SAMPLE_PLTAFROM, param, 1);  
//	can_device_send_pack(&can_pack);
//	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait for res
//	
//	//2.sample table
//	param[0] = 2;
//	local_create_cmd_pack(&can_pack, 0x0301, CAN_ADDR_SAMPLE_PLTAFROM, param, 1);  
//	can_device_send_pack(&can_pack);
//	can_device_get_rsp_pack(&can_pack, can_pack.cmd, PACK_TYPE_RESULT, (rt_tick_t)RT_WAITING_FOREVER);  //wait for res
	
	
}


void dev_reset_cmd(void)
{
	cmd_dev_reset_handle(RT_NULL);
	
}



void dev_check_bastet_cmd(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	flow_create_cache_region();
	
	pack.buf[0] = 0x02;
	req_basket_start_test_handle(&pack);
	
}

void dev_push_cache_cmd(void)
{

	//cmd_push_cache_reg_handle(RT_NULL);
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(4);
	dp = (uint32_t)local_pack;
	
	local_pack->cmd = NET_CMD_MOVE_CACHE_REG;
	local_pack->dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = 4;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


static uint16_t test_shap_code = 0;
void dev_push_shelf_cmd(void)
{
//	local_pack_t cmd_pack = {0};
//	
//	cmd_pack.cmd = NET_CMD_PUSH_SHELF; //EE 66 88 01 02 01
//	cmd_pack.buf[0] = 0xee; //shape code
//	cmd_pack.buf[1] = 0x66;
//	cmd_pack.buf[2] = 0x88;
//	cmd_pack.buf[3] = 0x01; //dev id
//	cmd_pack.buf[4] = 0x02; //track id
//	cmd_pack.buf[5] = 0x01; //track pos id (1:add sample pos 2: cache pos)
//	cmd_pack.src_addr = CAN_LOCAL_ADDR;
//	cmd_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
//	cmd_pack.type = PACK_TYPE_CMD;
//	cmd_pack.len = 6;
//	cmd_push_shelf_handle(&cmd_pack);
	
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(6);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = 0xee; //shape code
	local_pack->buf[1] = 0x66;
	local_pack->buf[2] = 0x88+test_shap_code;
	local_pack->buf[3] = 0x01; //dev id
	local_pack->buf[4] = 0x02; //track id
	local_pack->buf[5] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = 6;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_push_shelf_cache_pos_cmd(void)
{
//	local_pack_t cmd_pack = {0};
//	
//	cmd_pack.cmd = NET_CMD_PUSH_SHELF; //EE 66 88 01 02 01
//	cmd_pack.buf[0] = 0xee; //shape code
//	cmd_pack.buf[1] = 0x66;
//	cmd_pack.buf[2] = 0x88;
//	cmd_pack.buf[3] = 0x01; //dev id
//	cmd_pack.buf[4] = 0x02; //track id
//	cmd_pack.buf[5] = 0x02; //track pos id (1:add sample pos 2: cache pos)
//	cmd_pack.src_addr = CAN_LOCAL_ADDR;
//	cmd_pack.dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
//	cmd_pack.type = PACK_TYPE_CMD;
//	cmd_pack.len = 6;
//	cmd_push_shelf_handle(&cmd_pack);
	
	
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(6);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = 0xee; //shape code
	local_pack->buf[1] = 0x66;
	local_pack->buf[2] = 0x88+test_shap_code;
	local_pack->buf[3] = 0x01; //dev id
	local_pack->buf[4] = 0x02; //track id
	local_pack->buf[5] = 0x02; //track pos id (1:add sample pos 2: cache pos)
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = CAN_ADDR_SAMPLE_PLTAFROM;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = 6;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}

void dev_move_shelf_cmd(void)
{
//	local_pack_t cmd_pack = {0};
	
//	cmd_pack.cmd = NET_CMD_MOVE_SHELF; //EE 66 88 01 02 01
//	cmd_pack.buf[0] = 0x01; //shape code
//	cmd_pack.buf[1] = 0x02; ////track id
//	cmd_pack.buf[2] = 0x02; //track pos id (1:add sample pos 2: cache pos)
//	cmd_pack.buf[3] = 0x01; //tube pos
//	cmd_pack.dest_addr = CAN_ADDR_NORMAL_TRACK;
//	cmd_pack.type = PACK_TYPE_CMD;
//	cmd_pack.len = 4;
//	cmd_move_shelf_handle(&cmd_pack);
//	
//	cmd_pack.buf[0] = 0x01; //shape code
//	cmd_pack.buf[1] = 0x02; ////track id
//	cmd_pack.buf[2] = 0x02; //track pos id (1:add sample pos 2: cache pos)
//	cmd_pack.buf[3] = 0x0a; //tube pos
//	cmd_move_shelf_handle(&cmd_pack);
	
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(4);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = 0x01; //shape code
	local_pack->buf[1] = 0x02;
	local_pack->buf[2] = 0x02;
	local_pack->buf[3] = 0x0a; 
	local_pack->cmd = NET_CMD_MOVE_SHELF;
	local_pack->dest_addr = CAN_ADDR_NORMAL_TRACK;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = 4;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));
	
}

void dev_pop_shelf_cmd(void)
{
//	local_pack_t cmd_pack = {0};
//	
//	cmd_pack.cmd = NET_CMD_POP_SHELF; //EE 66 88 01 02 01
//	cmd_pack.buf[0] = 0xee; //shape code
//	cmd_pack.buf[1] = 0x66;
//	cmd_pack.buf[2] = 0x88;
//	cmd_pack.buf[3] = 0x01; //dev id
//	cmd_pack.buf[4] = 0x02; //track id
//	cmd_pack.buf[5] = 0x01; //track pos id (1:add sample pos 2: cache pos)
//	cmd_pack.src_addr = CAN_LOCAL_ADDR;
//	cmd_pack.dest_addr = CAN_ADDR_NORMAL_TRACK;
//	cmd_pack.type = PACK_TYPE_CMD;
//	cmd_pack.len = 6;
//	cmd_pop_shelf_handle(&cmd_pack);

	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(6);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = 0xee; //shape code
	local_pack->buf[1] = 0x66;
	local_pack->buf[2] = 0x88+test_shap_code;
	local_pack->buf[3] = 0x01; //dev id
	local_pack->buf[4] = 0x02; //track id
	local_pack->buf[5] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	local_pack->cmd = NET_CMD_POP_SHELF;
	local_pack->dest_addr = CAN_ADDR_NORMAL_TRACK;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = 6;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}






MSH_CMD_EXPORT(dev_reset_cmd, "dev_reset_cmd");
MSH_CMD_EXPORT(dev_check_bastet_cmd, "dev_check_bastet_cmd");
MSH_CMD_EXPORT(dev_push_cache_cmd, "dev_check_bastet_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cache_pos_cmd, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cmd, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_move_shelf_cmd, "dev_check_bastet_cmd");
MSH_CMD_EXPORT(dev_pop_shelf_cmd, "dev_check_bastet_cmd");



void dev_push_shelf_cache_pos_cmd1(void)
{
	test_shap_code = 0;
	dev_push_shelf_cache_pos_cmd();
}

void dev_push_shelf_cache_pos_cmd2(void)
{
	test_shap_code = 1;
	dev_push_shelf_cache_pos_cmd();
}

void dev_push_shelf_cache_pos_cmd3(void)
{
	test_shap_code = 2;
	dev_push_shelf_cache_pos_cmd();
}

void dev_push_shelf_cache_pos_cmd4(void)
{
	test_shap_code = 3;
	dev_push_shelf_cache_pos_cmd();
}

void dev_push_shelf_cache_pos_cmd5(void)
{
	test_shap_code = 4;
	dev_push_shelf_cache_pos_cmd();
}


void dev_push_shelf_cmd1(void)
{
	test_shap_code = 0;
	dev_push_shelf_cmd();
}

void dev_push_shelf_cmd2(void)
{
	test_shap_code = 1;
	dev_push_shelf_cmd();
}

void dev_push_shelf_cmd3(void)
{
	test_shap_code = 2;
	dev_push_shelf_cmd();
}

void dev_push_shelf_cmd4(void)
{
	test_shap_code = 3;
	dev_push_shelf_cmd();
}

void dev_push_shelf_cmd5(void)
{
	test_shap_code = 4;
	dev_push_shelf_cmd();
}


void dev_pop_shelf_cmd1(void)
{
	test_shap_code = 0;
	dev_pop_shelf_cmd();
}

void dev_pop_shelf_cmd2(void)
{
	test_shap_code = 1;
	dev_pop_shelf_cmd();
}

void dev_pop_shelf_cmd3(void)
{
	test_shap_code = 2;
	dev_pop_shelf_cmd();
}

void dev_pop_shelf_cmd4(void)
{
	test_shap_code = 3;
	dev_pop_shelf_cmd();
}

void dev_pop_shelf_cmd5(void)
{
	test_shap_code = 4;
	dev_pop_shelf_cmd();
}


MSH_CMD_EXPORT(dev_push_shelf_cache_pos_cmd1, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cache_pos_cmd2, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cache_pos_cmd3, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cache_pos_cmd4, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cache_pos_cmd5, "dev_push_shelf_cache_pos_cmd");


MSH_CMD_EXPORT(dev_push_shelf_cmd1, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cmd2, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cmd3, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cmd4, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_push_shelf_cmd5, "dev_push_shelf_cmd");


MSH_CMD_EXPORT(dev_pop_shelf_cmd1, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_pop_shelf_cmd2, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_pop_shelf_cmd3, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_pop_shelf_cmd4, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_pop_shelf_cmd5, "dev_push_shelf_cmd");


void scan_sample_test(void)
{
	sample_shelf_t smp = {0};
	smp.basket_pos = 2;
	smp.slot_pos = 1;
	smp.is_done = 0;
	smp.shape_code[0] = 0;
	smp.shape_code[1] = 1;
	smp.shape_code[2] = 2;
	flow_scan_sample_shelf(&smp);
	
}

MSH_CMD_EXPORT(scan_sample_test, "scan_sample_test");



void scan_single_smp(void)
{
	uint8_t len = flow_scan_sample(smp_shape_code_buf, 20);  
	if(len)
	{
		rt_kprintf("smp shelf size %d: %s\n", len, smp_shape_code_buf);
	}
}

MSH_CMD_EXPORT(scan_single_smp, "scan_single_smp");
