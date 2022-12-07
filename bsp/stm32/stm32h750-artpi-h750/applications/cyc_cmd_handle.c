#include "cyc_cmd_handle.h"


#define LOG_TAG "cyc cmd"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

static uint8_t tem_rsp_buf[128] = {0};

static uint8_t dev_get_smple_baseket_pos = 0;
static uint8_t dev_get_smple_baseket_slot = 0;
static uint8_t dev_pop_smple_baseket_pos = 0;
static uint8_t dev_pop_smple_baseket_slot = 0;


static volatile cache_region_t cache_area_table[CACHE_POS_NUM] = {0};
static uint8_t volatile start_basket_test_flag[BASKET_NUM] = {0};
static uint8_t volatile start_emer_store_test_flag[CACHE_NUM_EMER_STORE] = {0};
static uint16_t test_shap_code = 0;
void cache_pos_initail(void);

static uint8_t is_smp_exsit[10] = {0};
static uint8_t smp_shape_code_buf[256] = {0};

void dev_led_init(void);
smp_shelf_t globle_smp_shelf = {0};

static uint8_t basket_slot_info[CACHE_NUM_BASKET_SLOT] = {0};


void act_dev_status_reset(void)
{
	start_basket_test_flag[0] = 0;
	start_basket_test_flag[1] = 0;
	start_basket_test_flag[2] = 0;
	start_emer_store_test_flag[0] = 0;
	start_emer_store_test_flag[1] = 0;
	dev_led_init();
	cache_pos_initail();
	
	
	
}


uint8_t act_mincar_get_smp(uint8_t pos, uint8_t num)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};

	//1.get sample shelf from cache reg
	param[0] = pos;
	param[1] = num;
	 
	switch(pos)
	{
		case 1: //basket1 pos
		{
			//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
			break;
		}
		
		case 2: //basket2 pos
		{
			//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);    
			
			break;
		}
		
		case 3: //basket3 pos
		{
			//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);   
			
			break;
		}
		
		case 4: //basket1 pos
		{
			//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err =  local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);   
			
			break;
		}
		
		case 5: //basket1 pos
		{
			//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
			
			break;
		}
		
		case 6: //basket1 pos
		{
			//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);
			err = local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
		
			break;
		}
	
		default:
		{
			err = 1;
			break;
		}
			
	
	}
	
	return err;
	

}

uint8_t act_mincar_send_smp(uint8_t pos, uint8_t num)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};

	//1.get sample shelf from cache reg
	param[0] = pos;
	param[1] = num;
	 
	switch(pos)
	{
		case 1: //basket1 pos
		{
			//local_send_ctrl_frame(0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);    
			
			break;
		}
		
		case 2: //basket2 pos
		{
			//local_send_ctrl_frame(0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		case 3: //basket3 pos
		{
			//local_send_ctrl_frame(0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
			
			break;
		}
		
		case 4: //basket1 pos
		{
			//local_send_ctrl_frame(0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		case 5: //basket1 pos
		{
			//local_send_ctrl_frame(0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		case 6: //basket1 pos
		{
			//local_send_ctrl_frame(0x0901, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
			
			break;
		}
	
		default:
		{
			err = 1;
			break;
		}
		
	}
	
	return err;
	

}


uint8_t act_set_basket_led_status(uint8_t pos, uint8_t slot, uint8_t color)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};
	
	//1.get sample shelf from cache reg
	param[0] = pos;
	param[1] = slot;
	param[2] = color;

	//local_send_ctrl_frame(0x0601, CAN_ADDR_SAMPLE_STATUS, &param[0], 3);
	err = local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_SAMPLE_STATUS, CAN_WAIT_FLAG_RES);
	
	
	return err;
	
}


uint8_t act_set_emer_store_led_status(uint8_t slot, uint8_t color)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};
	
	//1.get sample shelf from cache reg
	param[0] = 1;
	param[1] = slot;
	param[2] = color;

	//local_send_ctrl_frame(0x0701, CAN_ADDR_SAMPLE_STATUS, &param[0], 3);
	err = local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_SAMPLE_STATUS, CAN_WAIT_FLAG_RES);
	


	return err;
	
}

uint8_t act_track_push_cache_pos(uint8_t track)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};
	
	switch(track)
	{
		case CAN_ADDR_EMER_TRACK: //emergency track
		{
			//cmd.push sample shelf to track cache pos [03: 01 06 01]
			param[0] = 1;
			//local_send_ctrl_frame(0x0701, CAN_ADDR_EMER_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		case CAN_ADDR_NORMAL_TRACK: //normal track
		{
			param[0] = 1;
			//local_send_ctrl_frame(0x0601, CAN_ADDR_EMER_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);  
			
			
			break;
		}
		
		default:
		{
			err = 1;
			break;
			
		}
	
	}
	
	return 0;

}


uint8_t act_track_push_addtion_pos(uint8_t track)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};
	
	switch(track)
	{
		case CAN_ADDR_EMER_TRACK: //emergency track
		{
			//cmd.push sample shelf to track cache pos [03: 01 06 01]
			param[0] = 2;
			//local_send_ctrl_frame(0x0701, CAN_ADDR_EMER_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		case CAN_ADDR_NORMAL_TRACK: //normal track
		{
			param[0] = 1;
			//local_send_ctrl_frame(0x0601, CAN_ADDR_NORMAL_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		default:
		{
			err = 1;
			break;
			
		}
	
	}
	
	return err;

}

uint8_t act_track_push_recyc_pos(uint8_t track)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};
	
	switch(track)
	{
		case CAN_ADDR_EMER_TRACK: //emergency track
		{
			//cmd.push sample shelf to track cache pos [03: 01 06 01]
			param[0] = 3;
			//local_send_ctrl_frame(0x0701, CAN_ADDR_EMER_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);  
			
			break;
		}
		
		case CAN_ADDR_NORMAL_TRACK: //normal track
		{
			param[0] = 2;
			//local_send_ctrl_frame(0x0601, CAN_ADDR_NORMAL_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES); 
			
			break;
		}
		
		default:
		{
			err = 1;
			break;
			
		}
	
	}
	
	return err;

}


uint8_t act_move_shelf(uint8_t track, uint8_t hole)
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0};
	
	switch(track)
	{
		case CAN_ADDR_EMER_TRACK:
		{
			param[0] = hole;
			//local_send_ctrl_frame(0x0801, CAN_ADDR_EMER_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES); 
			break;
		}
		
		case CAN_ADDR_NORMAL_TRACK:
		{
			param[0] = hole;
			//local_send_ctrl_frame(0x0901, CAN_ADDR_NORMAL_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0901, &param[0], CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES);  
			break;
		}
		
		default:
		{
			err = 1;
			break;
		}

	
	}

	return err;	
	

}


uint8_t act_track_pop_shelf(uint8_t track)  //cache_store
{
	uint8_t err = 0;
	uint8_t param[6] = {0};
	local_pack_t can_pack = {0}; 
	
	switch(track)
	{
		case CAN_ADDR_EMER_TRACK: //emergency track
		{
			//2. 1 7 1: 5 //change tack to emer track
			param[0] = 1;
			//local_send_ctrl_frame(0x0701, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);
			local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES); 
			
			//3. 1 7 4: 3
			param[0] = 4;
			//local_send_ctrl_frame(0x0701, CAN_ADDR_EMER_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);

			break;
		}
		
		case CAN_ADDR_NORMAL_TRACK: //normal track
		{
			//2. 1 7 2:5    //change tack to normal track
			param[0] = 2;
			//local_send_ctrl_frame(0x0701, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);
			local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
			
			//3. 1 6 3:4
			param[0] = 3;
			//local_send_ctrl_frame(0x0601, CAN_ADDR_NORMAL_TRACK, &param[0], 1);
			err = local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES);
			
			break;
		}
		
		default:
		{
			LOG_D("err track types %d!!!", track);
			err = 1;
			break;
			
		}
	
	}
	
	if(err)
	{
		return err;
	}
	
	//4. 1 7 3:5 //chage tack to recover pos
	param[0] = 3;
	//local_send_ctrl_frame(0x0701, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);
	local_send_ctrl_frame(0x0701, &param[0], CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
	
	#if 0 
	//7. 1 8 6:1  //mincar get sample from recover track
	param[0] = 6;
	local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);
	local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	#endif
	
	//5. 1 6 1:5
	param[0] = 1;
	//local_send_ctrl_frame(0x0601, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);
	local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);


	//6. 1 14 6 3:1
	param[0] = 6;
	param[1] = 3;
	//local_send_ctrl_frame(0x0e01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
	local_send_ctrl_frame(0x0e01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	
	#if 1
	//7. 1 8 6:1  //mincar get sample from recover track
	param[0] = 6;
	//local_send_ctrl_frame(0x0801, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);
	local_send_ctrl_frame(0x0801, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	#endif
	
	//8. 1 6 2:5
	param[0] = 2;
	//local_send_ctrl_frame(0x0601, CAN_ADDR_RECOVERY_TRACK, &param[0], 1);
	local_send_ctrl_frame(0x0601, &param[0], CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
	
	
	//9.1 1 16 1
	//9.2. 1 14 3 13:1
	param[0] = 1;
	//local_send_ctrl_frame(0x0e01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
	local_send_ctrl_frame(0x1001, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	
	param[0] = 3;
	param[1] = 13;
	//local_send_ctrl_frame(0x0e01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
	local_send_ctrl_frame(0x0e01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	
	//10. 1 17 1:1 
	param[0] = 1;
	//local_send_ctrl_frame(0x1101, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);
	err = local_send_ctrl_frame(0x1101, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	

	
	return err;

}






uint8_t act_scan_sample(uint8_t *buf, uint8_t maxlen)
{
	uint8_t recv_len = 0;

	scanner_dev_clean_stream();
	scanner_dev_ctrl(SCANNER_CMD_start, USER_SCANNER_TYPES, SCANNER_MODE_leuze_once);  //send start scan command
	rt_thread_delay(100);
	recv_len = scanner_dev_get_data(buf, maxlen, 50);
	

	return recv_len;
	
} 


void act_scan_continue(uint8_t *buf, uint8_t maxlen)
{
	uint8_t recv_len = 0;


	scanner_dev_ctrl(SCANNER_CMD_start, USER_SCANNER_TYPES, SCANNER_MODE_leuze_continue);  //send start scan command
	rt_thread_delay(100);
	scanner_dev_clean_stream();

} 

void act_scan_stop(void)
{
	scanner_dev_ctrl(SCANNER_CMD_stop, USER_SCANNER_TYPES, SCANNER_MODE_leuze_continue);  //send start scan command

} 





void act_get_basket_status(uint8_t basket_num)
{
	uint8_t res = 0;
	uint8_t param[6] = {0};
	uint16_t basket_status = 0;
	smp_shelf_t *sample_shelf = RT_NULL;
	uint8_t cache_pos = 0;
	local_pack_t rep_pack = {0};
	
	//local_send_ctrl_frame(0x2a04, CAN_ADDR_SAMPLE_STATUS, &basket_num, 1);
	param[0] = basket_num;
	res = local_send_ctrl_frame(0x2a04, &param[0], CAN_ADDR_SAMPLE_STATUS, CAN_WAIT_FLAG_DATA);
	if(res == RT_EOK)
	{
		basket_status = *((uint16_t*)&param[2]);  //cmd: D0 D1 D3 == basket D4D5 == status
	}
	  
	for(uint8_t i = 0; i < 13; i ++)
	{
		if(basket_status & (0x01<<i))
		{
			cache_pos = CACHE_POS_BASKET_BASE + ((basket_num -1)*CACHE_NUM_BASKET_SLOT) + i;
			cache_area_table[cache_pos].is_empty = 0; 
			cache_area_table[cache_pos].smp.basket_pos = basket_num;
			cache_area_table[cache_pos].smp.slot_pos = i+1;
			cache_area_table[cache_pos].smp.is_done = 0;
			act_set_basket_led_status(basket_num, i+1, LED_COLOR_Y);
			LOG_I("found sample, basket %d slot %d\n", basket_num, i+1);
			
			basket_slot_info[i] = 1;

		}
		else
		{
			#if 0
			rep_pack.cmd = 0x00000450;
			rep_pack.dest_addr = 0xFE;
			rep_pack.src_addr = LOCAL_NET_ADDR;
			rep_pack.serial = 0;
			rep_pack.type = PACK_TYPE_REPORT;	
			rep_pack.len = 7;
			
			rt_memset(tem_rsp_buf, 0, 30);
			tem_rsp_buf[0] = 0;
			tem_rsp_buf[1] = BASKET_TYPES_NORMAL;
			tem_rsp_buf[2] = basket_num;
			tem_rsp_buf[3] = i+1;
			
			//sample len and sample data 20 byte
			tem_rsp_buf[4] = 00;
			tem_rsp_buf[5] = 00;
			
			local_transmit_response(&rep_pack, tem_rsp_buf, 4, PACK_TYPE_REPORT);
			act_set_basket_led_status(basket_num, i+1, LED_COLOR_OFF);
			
			#endif
			basket_slot_info[i] = 0;
			
			LOG_I("report empty pos basket %d slot %d\n", basket_num, i+1);
			
		}
	}	
		
}	


/*****************************************************************************************************/
void cache_pos_initail(void)
{
	for(uint8_t i = 0; i < CACHE_POS_NUM; i++)
	{
		rt_memset((uint8_t*)&cache_area_table[i].smp.shape_code[0], 0, sizeof(cache_region_t));
		cache_area_table[i].is_empty = IS_POS_EMPTY;
		cache_area_table[i].smp.is_done = IS_SMP_NOT_DONE;
	}
	
}


int push_cache_pos(uint8_t src_pos, uint8_t cache_types, uint8_t done)
{
	//for(uint8_t i = CACHE_POS_BASKET_BASE; i < (CACHE_NUM_BASKET_SLOT * BASKET_NUM) + CACHE_POS_BASKET_BASE; i++)
	uint8_t i = 0;
	int pos = -1;
	uint8_t start_index = 0;
	uint8_t end_index = 0;
	
	if(cache_types == CACHE_TYPES_EMER)
	{
		start_index = CACHE_POS_EMER_STORE_BASE;
		end_index = CACHE_NUM_EMER_STORE + CACHE_POS_EMER_STORE_BASE;
	}
	else if(cache_types == CACHE_TYPES_BASKET)
	{
		start_index = CACHE_POS_BASKET_BASE;
		end_index = CACHE_NUM_BASKET_SLOT * BASKET_NUM + CACHE_POS_BASKET_BASE;
	}
	else if(cache_types == CACHE_TYPES_CACHE_STORE)
	{
		start_index = CACHE_POS_CACHE_STORE_BASE;
		end_index = CACHE_POS_CACHE_STORE_BASE + CACHE_NUM_CACHE_STORE;
	}
	
	for(i = start_index; i < end_index; i++) //CACHE_POS_NUM
	{
		if(cache_area_table[i].is_empty == IS_POS_EMPTY)
		{
			cache_area_table[i].is_empty = IS_POS_NOT_EMPTY;
			cache_area_table[src_pos].is_empty = IS_POS_EMPTY;
			rt_memcpy((uint8_t*)&cache_area_table[i].smp, (uint8_t*)&cache_area_table[src_pos].smp, sizeof(smp_shelf_t));
			rt_memset((uint8_t*)&cache_area_table[src_pos].smp, 0, sizeof(smp_shelf_t));
			cache_area_table[i].smp.is_done = done;
			pos = i - start_index;
			break;
		}
	
	}
	
	return pos;
	
}


int pop_cache_pos(uint8_t src_pos, uint8_t dest_pos)
{

	if(cache_area_table[dest_pos].is_empty == IS_POS_EMPTY)
	{
		cache_area_table[dest_pos].is_empty = IS_POS_NOT_EMPTY;
		
		cache_area_table[src_pos].is_empty = IS_POS_EMPTY;
		rt_memcpy((uint8_t*)&cache_area_table[dest_pos].smp, (uint8_t*)&cache_area_table[src_pos].smp, sizeof(smp_shelf_t));
		rt_memset((uint8_t*)&cache_area_table[src_pos].smp, 0, sizeof(smp_shelf_t));
		return dest_pos;
	}
		
	return -1;
	
}


int get_smp_cache_pos(smp_shelf_t *sample)
{
	for(uint8_t i = 0; i < CACHE_POS_NUM; i++)
	{
		if( cache_area_table[i].is_empty == IS_POS_NOT_EMPTY && 
			cache_area_table[i].smp.is_done == IS_SMP_NOT_DONE &&
			rt_memcmp((uint8_t*)&cache_area_table[i].smp.shape_code[0], &sample->shape_code[0], sample->len) == 0)
		{
			return i;	
		}
		
	}
	
	return -1;
	
}

int get_is_smp_in_place(smp_shelf_t *sample, uint8_t pos)
{
	if( cache_area_table[pos].is_empty == IS_POS_NOT_EMPTY && 
		cache_area_table[pos].smp.is_done == IS_SMP_NOT_DONE &&
		rt_memcmp((uint8_t*)&cache_area_table[pos].smp.shape_code[0], &sample->shape_code[0], sample->len) == 0)
	{
		return pos;	
	}
	
	
	return -1;
	
}


int get_is_exsit_smp_of_cache_pos(uint8_t pos)
{
	//011
	if( cache_area_table[pos].is_empty == IS_POS_NOT_EMPTY &&
		cache_area_table[pos].smp.basket_pos &&
		cache_area_table[pos].smp.slot_pos )
	{
		return pos;	
	}
	
	return -1;
	
}






int get_emer_store_sample(smp_shelf_t *dest_smp, uint8_t store_num)
{
	uint8_t slot_index = 0;

	
	if(store_num < 1 || store_num > CACHE_NUM_EMER_STORE)
	{
		return  -1;
	}
	
	slot_index = CACHE_POS_EMER_STORE_BASE + store_num-1;


	if(cache_area_table[slot_index].is_empty == IS_POS_NOT_EMPTY && cache_area_table[slot_index].smp.is_done == IS_SMP_NOT_DONE)
	{
		//cache_area_table[i].is_empty = 1;
		rt_memcpy(dest_smp, (uint8_t*)&cache_area_table[slot_index].smp, sizeof(smp_shelf_t));
		return slot_index;	
	}

	return -1;
	
}


int get_basket_sample(smp_shelf_t *dest_smp, uint8_t basket_num)
{
	uint8_t base_slot_index = 0;
	uint8_t end_slot_index = 0;
	
	if(basket_num < 1 || basket_num > BASKET_NUM)
	{
		return  -1;
	}
	
	base_slot_index = CACHE_POS_BASKET_BASE + ((basket_num-1) * CACHE_NUM_BASKET_SLOT);
	end_slot_index = base_slot_index + CACHE_NUM_BASKET_SLOT;
	
	for(uint8_t i = base_slot_index; i < end_slot_index; i++)
	{
		if(cache_area_table[i].is_empty == IS_POS_NOT_EMPTY && cache_area_table[i].smp.is_done == IS_SMP_NOT_DONE)
		{
			//cache_area_table[i].is_empty = 1;
			rt_memcpy(dest_smp, (uint8_t*)&cache_area_table[i].smp, sizeof(smp_shelf_t));
			return i;	
		}
		
	}
	
	return -1;
	
}






int get_is_exsit_the_same_smp(smp_shelf_t *dest_smp, uint8_t skip_pos)
{
	for(uint8_t i = 0; i < CACHE_POS_NUM; i++)
	{
		if(i == skip_pos)
		{
			continue;
		}
		
		if( rt_memcmp((uint8_t*)&cache_area_table[i].smp.shape_code[0], &dest_smp->shape_code[0], dest_smp->len) == 0 )
		{
			return i;	
		}
	}
		
	return 0;
	
}




#if 0  //USER_SCANNER_TYPES == SCANNER_TYPE_leuze_BCL95 
uint8_t report_sample_shelf_infomation(smp_shelf_t *smp, uint8_t *code, uint8_t *code_size, uint8_t num)
{
	uint8_t smp_num = 0;
	uint8_t pack_size = 0;
	uint8_t basket_type = 0;
	uint8_t *rep_buf = RT_NULL;
	local_pack_t rep_pack = {0};
	uint8_t tube_code_base = 0;
	uint8_t src_code_base = 0;
	rep_buf = rt_malloc(220);
	
	if(smp->basket_pos > BASKET_NUM)
	{
		basket_type = BASKET_TYPES_EMER;
	}
	else
	{
		basket_type = BASKET_TYPES_NORMAL;
	}
	
	//rt_memcpy(&rep_buf[4], &smp->shape_code[0], smp->len);
	
	rep_buf[0] = smp->len;
	rt_memcpy(&rep_buf[1], &smp->shape_code[1], smp->len);
	rep_buf[smp->len+1] = basket_type;
	rep_buf[smp->len+2] = smp->basket_pos;
	rep_buf[smp->len+3] = smp->slot_pos;
	
	tube_code_base = smp->len + 4;
	
	for(uint8_t i = 0; i < num; i++)
	{
		
		if(is_smp_exsit[i])
		{	
			if(code_size[i])
			{
				rt_memcpy(&rep_buf[tube_code_base+1], &code[src_code_base], code_size[i]-2);
				rep_buf[tube_code_base] = is_smp_exsit[i];   //is smple exsit
				rep_buf[tube_code_base+1] = code_size[i]-3;  //smple size
				
				src_code_base += code_size[i];
				tube_code_base += (code_size[i] + 1 -2);
			}
			else
			{
				rep_buf[tube_code_base] = is_smp_exsit[i]; //is smple exsit
				rep_buf[tube_code_base+1] = code_size[i];  //smple size
				src_code_base += code_size[i];
				tube_code_base += (code_size[i] + 2);
			}
			
			smp_num++;
		}
		else
		{
			
			rep_buf[tube_code_base] = is_smp_exsit[i]; //is smple exsit
			rep_buf[tube_code_base+1] = code_size[i];  //smple size
			src_code_base += code_size[i];
			tube_code_base += (code_size[i] + 2);
		}
		

		
	
	}
	
	
	LOG_D("smp shelf: %s\n", smp->shape_code);
	LOG_D("smp num %d\n", smp_num);
	pack_size = tube_code_base;
	LOG_HEX("report pack:", pack_size, rep_buf, pack_size);
	

	rep_pack.cmd = 0x00000450;
	rep_pack.dest_addr = 0xFE;
	rep_pack.src_addr = LOCAL_NET_ADDR;
	rep_pack.serial = 0;
	rep_pack.type = PACK_TYPE_REPORT;	
	rep_pack.len = pack_size; 

	
	if(smp->len)
	{
		local_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], rep_pack.len, PACK_TYPE_REPORT);
		udp_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], rep_pack.len, PACK_TYPE_REPORT);
		
	}
	else
	{
		rep_buf[0] = 0;
		rep_buf[1] = basket_type;
		rep_buf[2] = smp->basket_pos;
		rep_buf[3] = smp->slot_pos;
		
		local_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], 4, PACK_TYPE_REPORT);
		udp_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], 4, PACK_TYPE_REPORT);
	}
	
	
	rt_free(rep_buf);
	
	return pack_size;
	

}

#endif

#if 1  //USER_SCANNER_TYPES == SCANNER_TYPE_cognex 
uint8_t report_sample_shelf_infomation(smp_shelf_t *smp, uint8_t *code, uint8_t *code_size, uint8_t num)
{
	uint8_t smp_num = 0;
	uint8_t pack_size = 0;
	uint8_t basket_type = 0;
	uint8_t *rep_buf = RT_NULL;
	local_pack_t rep_pack = {0};
	uint8_t tube_code_base = 0;
	uint8_t src_code_base = 0;
	rep_buf = rt_malloc(220);
	
	if(smp->basket_pos > BASKET_NUM)
	{
		basket_type = BASKET_TYPES_EMER;
	}
	else
	{
		basket_type = BASKET_TYPES_NORMAL;
	}
	
	//rt_memcpy(&rep_buf[4], &smp->shape_code[0], smp->len);
	rep_buf[0] = smp->len;
	rt_memcpy(&rep_buf[1], &smp->shape_code[0], smp->len);
	rep_buf[smp->len+1] = basket_type;
	rep_buf[smp->len+2] = smp->basket_pos;
	rep_buf[smp->len+3] = smp->slot_pos;
	
	tube_code_base = smp->len + 4;
	
	for(uint8_t i = 0; i < num; i++)
	{
		
		if(is_smp_exsit[i])
		{	
			if(code_size[i])
			{
				rep_buf[tube_code_base] = is_smp_exsit[i];   //is smple exsit
				rep_buf[tube_code_base+1] = code_size[i]-1;  //smple size
				rt_memcpy(&rep_buf[tube_code_base+2], &code[src_code_base], code_size[i]-1);
				
				src_code_base += code_size[i];
				tube_code_base += (code_size[i]+1);
			}
			else
			{
				rep_buf[tube_code_base] = is_smp_exsit[i]; //is smple exsit
				rep_buf[tube_code_base+1] = code_size[i];  //smple size
				src_code_base += code_size[i];
				tube_code_base += (code_size[i] + 2);
			}
			
			smp_num++;
		}
		else
		{
			
			rep_buf[tube_code_base] = is_smp_exsit[i]; //is smple exsit
			rep_buf[tube_code_base+1] = code_size[i];  //smple size
			src_code_base += code_size[i];
			tube_code_base += (code_size[i] + 2);
		}
		

		
	
	}
	
	LOG_D("smp shelf: %s\n", smp->shape_code);
	LOG_D("smp num %d\n", smp_num);
	pack_size = tube_code_base;
	LOG_RAW("\n");
	LOG_HEX("report pack:", pack_size, rep_buf, pack_size);


	rep_pack.cmd = 0x00000450;
	rep_pack.dest_addr = 0xFE;
	rep_pack.src_addr = LOCAL_NET_ADDR;
	rep_pack.serial = 0;
	rep_pack.type = PACK_TYPE_REPORT;	
	rep_pack.len = pack_size; 

	
	if(smp->len)
	{
		local_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], rep_pack.len, PACK_TYPE_REPORT);
		udp_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], rep_pack.len, PACK_TYPE_REPORT);
		
	}
	else
	{
		rep_buf[0] = 0;
		rep_buf[1] = basket_type;
		rep_buf[2] = smp->basket_pos;
		rep_buf[3] = smp->slot_pos;
		
		local_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], 4, PACK_TYPE_REPORT);
		udp_transmit_response(&rep_pack, (uint8_t*)&rep_buf[0], 4, PACK_TYPE_REPORT);
	}
	
	
	rt_free(rep_buf);
	
	return pack_size;
	

}


#endif

void dev_start_conex_scanner(void);

void act_scan_sample_shelf_flows(smp_shelf_t *smp_shelf)
{
	uint8_t err = 0;
	uint8_t i = 0;
	uint8_t smp_size[10] = {0};
	uint8_t smp_index = 0;
	uint8_t sap_num = 0;
	uint8_t param[6] = {0};


	rt_memset(smp_shape_code_buf, 0, 256);
	//3.set motor position: get sample shelf 
	LOG_D("\n===============step3.get sample shelf===============\n");
	param[0] = smp_shelf->basket_pos;
	param[1] = smp_shelf->slot_pos;
	
	dev_get_smple_baseket_pos = smp_shelf->basket_pos;
	dev_get_smple_baseket_slot = smp_shelf->slot_pos;
	
	
	//local_send_ctrl_frame(0x0a01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2);
	local_send_ctrl_frame(0x0a01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	

	#if 1   //USER_SCANNER_TYPES SCANNER_TYPE_cognex
	//6.set smp 
	param[0] = 7;
	smp_shelf->len = 0;
	//local_send_ctrl_frame(0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);
	local_send_ctrl_frame(0x0f01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	
	rt_thread_delay(200);
	smp_size[0] = act_scan_sample(&smp_shelf->shape_code[0], SMP_SHAPE_CODE_MAX_SIZE);
	if(smp_size[0])   //2.scan sample shelf	shape code
	{
		LOG_D("smp shelf size %d: %s\n", smp_size[0], smp_shelf->shape_code[0]);
		//rt_memcpy(&smp_shelf->shape_code[0], &smp_shape_code_buf[1], smp_size[0]);
		smp_shelf->len = smp_size[0]-1;
	}
	else
	{
		err = 1;
		LOG_E("lose sample shelf shape code!!!\n");
	}

	#else   //USER_SCANNER_TYPES SCANNER_TYPE_leuze_BCL95
	
	//6.set smp 
	param[0] = 7;
	smp_shelf->len = 0;
	//local_send_ctrl_frame(0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1);
	local_send_ctrl_frame(0x0f01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	
	rt_thread_delay(200);
	smp_size[0] = act_scan_sample(&smp_shelf->shape_code[0], SMP_SHAPE_CODE_MAX_SIZE);
	if(smp_size[0])   //2.scan sample shelf	shape code
	{
		rt_kprintf("smp shelf size %d: %s\n", smp_size[0], smp_shelf->shape_code[0]);
		//rt_memcpy(&smp_shelf->shape_code[0], &smp_shape_code_buf[1], smp_size[0]);
		smp_shelf->len = smp_size[0] - 3;
	}
	else
	{
		err = 1;
		LOG_E("lose sample shelf shape code!!!\n");
	}
	
	#endif
	
	
	globle_smp_shelf.basket_pos = smp_shelf->basket_pos;
	globle_smp_shelf.slot_pos = smp_shelf->slot_pos;
	globle_smp_shelf.is_done = smp_shelf->is_done;
	globle_smp_shelf.len = smp_shelf->len;
	
	rt_memcpy(&globle_smp_shelf.shape_code[0], &smp_shelf->shape_code[0], globle_smp_shelf.len);

	if(get_is_exsit_the_same_smp(smp_shelf, CACHE_POS_MIN_CAR)) //if(get_is_e   _the_same_smp(smp_shelf, CACHE_POS_MIN_CAR))
	{
		err = 1;
		LOG_E("found the same sample, please take it away and release min car status!!!\n");
		act_set_basket_led_status(smp_shelf->basket_pos, smp_shelf->slot_pos, LED_COLOR_R);
	}

	smp_size[0] = 0;
	sap_num = 0;
	smp_index = 0;
	rt_memset(&is_smp_exsit[0], 0, 10);
	for(uint8_t i = 0; i < 10; i++)
	{	
		//4.set sample scan position: set sample tube scan pos
		LOG_D("4.1 set sample %d pos\n", i+1);
		param[0] = 0x06; 
		param[1] = i+1;
		//local_send_ctrl_frame(0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 2); 
		local_send_ctrl_frame(0x0f01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
		
		rt_thread_delay(200);
		
		//local_send_ctrl_frame(0x2904, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1); 
		param[0] = 0x06;
		local_send_ctrl_frame(0x2904, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_DATA);
		
		is_smp_exsit[i] = param[2];
		//is_smp_exsit[i] = 1;
		if(is_smp_exsit[i]) //smple exist
		{
			smp_size[i] = act_scan_sample(&smp_shape_code_buf[smp_index], 20); //2.scan sample
			smp_index += smp_size[i];
			sap_num++;
			
			#if 0
			dev_start_conex_scanner();
			#endif
		}
	
	}
	
	report_sample_shelf_infomation(smp_shelf, &smp_shape_code_buf[0], &smp_size[0], 10);
	

	//7.push all sample to car : call mini car pos
	param[0] = 8;
	//local_send_ctrl_frame(0x0f01, CAN_ADDR_SAMPLE_PLTAFROM, &param[0], 1); 
	local_send_ctrl_frame(0x0f01, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	
	if(err)
	{
		act_set_basket_led_status(smp_shelf->basket_pos, smp_shelf->slot_pos, LED_COLOR_R);
	}
		
		
}





static uint8_t basket_index = 0;
void req_basket_start_test_handle(void *args)
{
	uint8_t minicar_pos_opt = 0;
	uint8_t param[15] = {0};
	local_pack_t rsp_pack = {0};
	local_pack_t *req_pack = RT_NULL;
	
	req_pack = (local_pack_t*) args;
	basket_index = req_pack->buf[0] - 1;
	
	LOG_I("req: basket %d start test\n", basket_index+1);
	
	
	act_get_basket_status(basket_index+1);

	if(local_send_ctrl_frame(0x2904, param, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_DATA) == RT_EOK) 
	{
		minicar_pos_opt = param[2];
	}
	
	if(cache_area_table[CACHE_POS_MIN_CAR].is_empty == IS_POS_NOT_EMPTY) //min car busy
	{
		LOG_E("busy of min car, please release!\n", basket_index+1);
		return;
	}
		
	
	if(basket_index > BASKET_NUM)
	{
		return;
	}
	
	start_basket_test_flag[basket_index] = 1;
	
	rsp_pack.cmd = 0x0000650;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_NORMAL;
	param[1] = basket_index + 1;
	
	rt_memcpy(&param[2], &basket_slot_info[0], 13);
	local_transmit_response(&rsp_pack, param, 15, PACK_TYPE_REPORT);
	
	
//	rsp_pack.cmd = 0x0000650;
//	rsp_pack.src_addr = LOCAL_NET_ADDR;
//	rsp_pack.dest_addr = 0xFE;
//	rsp_pack.serial = 0;
//	param[0] = BASKET_TYPES_NORMAL;
//	param[1] = basket_index + 1;
//	local_transmit_response(&rsp_pack, param, 2, PACK_TYPE_REPORT);

	
}


void req_basket_pause_test_handle(void *args)
{
	local_pack_t rsp_pack = {0};
	uint8_t param[8] = {0};
	//uint16_t basket_index = 0;
	local_pack_t *req_pack = RT_NULL;
	
	req_pack = (local_pack_t*) args;
	basket_index = req_pack->buf[0] - 1;
	
	LOG_I("req: basket %d pause test, and reset  min car status\n", basket_index+1);
	
	if(basket_index > BASKET_NUM)
	{
		LOG_E("err basket num %d\n", basket_index+1);
		return;
	}
	
	start_basket_test_flag[basket_index] = 0;
	
	rsp_pack.cmd = 0x0000150;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_NORMAL;
	param[1] = basket_index + 1;
	local_transmit_response(&rsp_pack, param, 2, PACK_TYPE_REPORT);
	cache_area_table[CACHE_POS_MIN_CAR].is_empty = IS_POS_EMPTY;
	rt_memset((uint8_t*)&cache_area_table[CACHE_POS_MIN_CAR].smp, 0, sizeof(smp_shelf_t));  

}



void req_basket_update_handle(void *args)
{
	uint8_t basket_num = 0;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *req_pack = RT_NULL;
	req_pack = (local_pack_t*)args;
	basket_num = req_pack->buf[0];
	
	LOG_I("update basket %d \n", basket_num);
	//1.get basket status
 	act_get_basket_status(basket_num);
	
	#if 0
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf =  rt_malloc(4);
	
	rsp_pack->cmd = NET_CMD_MOVE_CACHE_REG;
	rsp_pack->len = 4;
	rsp_pack->buf[3] = 0;
	rsp_pack->type = PACK_TYPE_RESULT;
	local_send_response(rsp_pack);
	#endif
}

void req_basket_inser_sample_handle(void *args)
{
	uint8_t cache_pos = 0;
	local_pack_t *req_pack = RT_NULL;
	smp_shelf_t smple = {0};
	local_pack_t rsp_pack = {0};
	uint8_t param[3] = {0};
	
	req_pack = (local_pack_t*)args;	
	smple.basket_pos = req_pack->buf[0];
	smple.slot_pos = req_pack->buf[1];
	smple.is_done = 0;
	
	
	cache_pos = CACHE_POS_BASKET_BASE + ((smple.basket_pos -1)*CACHE_NUM_BASKET_SLOT) + (smple.slot_pos - 1);
	cache_area_table[cache_pos].is_empty = 0; 
	cache_area_table[cache_pos].smp.basket_pos = smple.basket_pos;
	cache_area_table[cache_pos].smp.slot_pos = smple.slot_pos;
	cache_area_table[cache_pos].smp.is_done = IS_SMP_NOT_DONE;
	
	LOG_I("req: basket %d solt: %d inser_sample_shelf, cache_pos %d\n", smple.basket_pos, smple.slot_pos, cache_pos);
	act_set_basket_led_status(smple.basket_pos,  smple.slot_pos, LED_COLOR_Y);
	
	rsp_pack.cmd = 0x0000250;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_NORMAL;
	param[1] = smple.basket_pos;
	param[2] = smple.slot_pos;
	
	if( dev_pop_smple_baseket_pos == smple.basket_pos  &&
	    dev_pop_smple_baseket_slot == smple.slot_pos)
	{
		LOG_I("mini car inser sample shelf!\n");
		cache_area_table[cache_pos].smp.is_done = IS_SMP_DONE;
		rt_thread_delay(30);
		act_set_basket_led_status(smple.basket_pos,  smple.slot_pos, LED_COLOR_G);
	
	}
	else if(start_basket_test_flag[smple.basket_pos-1])
	{
		LOG_I("user inser sample shelf!\n");
		local_transmit_response(&rsp_pack, param, 3, PACK_TYPE_REPORT);
	}

}


void req_basket_remove_sample_handle(void *args)  //
{
	uint8_t cache_pos = 0;
	local_pack_t *req_pack = RT_NULL;
	smp_shelf_t smple = {0};
	local_pack_t rsp_pack = {0};
	uint8_t param[3] = {0};

	req_pack = (local_pack_t*)args;	
	smple.basket_pos = req_pack->buf[0];
	smple.slot_pos = req_pack->buf[1];
	smple.is_done = 0;
	
	
	cache_pos = CACHE_POS_BASKET_BASE + ((smple.basket_pos -1)*CACHE_NUM_BASKET_SLOT) + (smple.slot_pos - 1);
	cache_area_table[cache_pos].is_empty = 1; 
	cache_area_table[cache_pos].smp.basket_pos = 0; 
	cache_area_table[cache_pos].smp.slot_pos = 0;
	cache_area_table[cache_pos].smp.is_done = 0;
	
	LOG_I("req: basket %d solt: %d remove_sample_shelf cache_pos %d\n", smple.basket_pos, smple.slot_pos, cache_pos);
	act_set_basket_led_status(smple.basket_pos,  smple.slot_pos, LED_COLOR_OFF);
	
	rsp_pack.cmd = 0x0000350;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_NORMAL;
	param[1] = smple.basket_pos;
	param[2] = smple.slot_pos;
	
	if( dev_get_smple_baseket_pos == smple.basket_pos && 
		dev_get_smple_baseket_slot == smple.slot_pos)
	{
		LOG_I("mini car take sample shelf!\n");
	}
	else if(start_basket_test_flag[smple.basket_pos-1])
	{
		LOG_I("user take sample shelf!\n");
		local_transmit_response(&rsp_pack, param, 3, PACK_TYPE_REPORT);  //dont need report
	}
	


}


void basket_sample_scan_handle(uint8_t basket_num)
{	
	int cache_store = -1;
	smp_shelf_t smp_shelf = {0};
	
	if(basket_num < 1 || basket_num > BASKET_NUM)
	{
		return;
	}
	
	if(start_basket_test_flag[basket_num-1] == 0)
	{
		return;
	}
	
	if(cache_area_table[CACHE_POS_MIN_CAR].is_empty == IS_POS_EMPTY) //min car idle 100 search_cache_pos
	{
		//cache_store = search_cache_basket(&smp_shelf, basket_num);
		cache_store = get_basket_sample(&smp_shelf, basket_num);
		if(cache_store == -1)
		{
			return;
		}
		
		act_scan_sample_shelf_flows(&smp_shelf);
		rt_memcpy((uint8_t*)&cache_area_table[cache_store].smp, &smp_shelf, sizeof(smp_shelf_t));
		pop_cache_pos(cache_store, CACHE_POS_MIN_CAR); //push_cache_sample(&smp_shelf, CACHE_POS_MIN_CAR);
		cache_area_table[CACHE_POS_MIN_CAR].smp.is_done = IS_SMP_NOT_DONE;
	}
	
	
}



void req_emer_start_test_handle(void *args)
{
	uint16_t emer_store_index = 0;
	local_pack_t *req_pack = RT_NULL;
	
	req_pack = (local_pack_t*) args;
	emer_store_index = req_pack->buf[0] - 1;
	
	LOG_I("req: emer store %d start_test\n", emer_store_index+1);
	
	if(cache_area_table[CACHE_POS_MIN_CAR].is_empty == IS_POS_NOT_EMPTY) 
	{
		LOG_E("busy of min car, please release!\n", basket_index+1);
		return;
	}
	
	if(emer_store_index > CACHE_NUM_EMER_STORE)
	{
		return;
	}
	
	start_emer_store_test_flag[emer_store_index] = 1;
	
}


void req_emer_pause_test_handle(void *args)
{
	uint8_t param[2] = {0};
	uint16_t emer_store_index = 0;
	local_pack_t *req_pack = RT_NULL;
	local_pack_t rsp_pack = {0};
	req_pack = (local_pack_t*) args;
	emer_store_index = req_pack->buf[0] - 1;
	
	LOG_I("req: emer store %d pause_test\n", emer_store_index+1);

	if(emer_store_index > CACHE_NUM_EMER_STORE)
	{
		return;
	}
	
	start_emer_store_test_flag[emer_store_index] = 0;
	
	rsp_pack.cmd = 0x0000150;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_EMER;
	param[1] = emer_store_index + 1;
	local_transmit_response(&rsp_pack, param, 2, PACK_TYPE_REPORT);


	req_pack = (local_pack_t*) args;
	
	

}


void req_emer_inser_sample_handle(void *args)
{
	uint8_t emer_store_pos = 0;
	uint8_t cache_pos = 0;
	local_pack_t *req_pack = RT_NULL;
	smp_shelf_t smple = {0};
	uint8_t param[3] = {0};
	local_pack_t rsp_pack = {0};

	req_pack = (local_pack_t*)args;	
	emer_store_pos = req_pack->buf[0];
	
	smple.basket_pos = 5;  //emer store
	smple.slot_pos = emer_store_pos;
	smple.is_done = 0;
	
	LOG_I("req: emer store %d inser_sample\n", emer_store_pos);
	act_set_emer_store_led_status(smple.slot_pos, LED_COLOR_Y);

	cache_pos = CACHE_POS_EMER_STORE_BASE + emer_store_pos -1;
	cache_area_table[cache_pos].is_empty = 0; 

	cache_area_table[cache_pos].smp.basket_pos = smple.basket_pos;
	cache_area_table[cache_pos].smp.slot_pos = smple.slot_pos;
	cache_area_table[cache_pos].smp.is_done = smple.is_done;
	
	rsp_pack.cmd = 0x0000250;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_EMER;
	param[1] = smple.basket_pos;
	param[2] = smple.slot_pos;
	local_transmit_response(&rsp_pack, param, 3, PACK_TYPE_REPORT);

}


void req_emer_remove_sample_handle(void *args)
{
	uint8_t emer_store_pos = 0;
	uint8_t cache_pos = 0;
	local_pack_t *req_pack = RT_NULL;
	smp_shelf_t smple = {0};
	uint8_t param[3] = {0};
	local_pack_t rsp_pack = {0};

	req_pack = (local_pack_t*)args;	
	emer_store_pos = req_pack->buf[0];
	smple.basket_pos = 0;
	smple.slot_pos = emer_store_pos;
	smple.is_done = 0;
	
	LOG_I("req: emer store %d remove_sample\n", emer_store_pos);
	act_set_emer_store_led_status(smple.slot_pos, LED_COLOR_OFF);
	
	cache_pos = CACHE_POS_EMER_STORE_BASE + emer_store_pos -1;
	cache_area_table[cache_pos].is_empty = 1; 

	cache_area_table[cache_pos].smp.basket_pos = 0;
	cache_area_table[cache_pos].smp.slot_pos = 0;
	cache_area_table[cache_pos].smp.is_done = 0;
	
	rsp_pack.cmd = 0x0000350;
	rsp_pack.src_addr = LOCAL_NET_ADDR;
	rsp_pack.dest_addr = 0xFE;
	rsp_pack.serial = 0;
	param[0] = BASKET_TYPES_EMER;
	param[1] = smple.basket_pos;
	param[2] = smple.slot_pos;
	local_transmit_response(&rsp_pack, param, 3, PACK_TYPE_REPORT);


}


void emer_store_sample_scan_handle(uint8_t store_num)
{	
	int cache_store = -1;
	int cache_store2 = -1;
	
	smp_shelf_t smp_shelf = {0};
	
	if(store_num < 1 || store_num > BASKET_NUM)
	{
		return;
	}
	
	if(start_emer_store_test_flag[store_num-1] == 0)
	{
		return;
	}
	
	if(cache_area_table[CACHE_POS_MIN_CAR].is_empty == IS_POS_EMPTY) //min car idle 100 search_cache_pos
	{
		if(get_is_exsit_smp_of_cache_pos(CACHE_POS_MIN_CAR) == CACHE_POS_MIN_CAR)  //if(search_cache_pos(CACHE_POS_MIN_CAR) == CACHE_POS_MIN_CAR) 
		{
			cache_store2 = push_cache_pos(CACHE_POS_MIN_CAR, CACHE_TYPES_EMER, 0); //100 //cache_store2 = push_cache_store(CACHE_POS_MIN_CAR, 0); //100
			if(cache_store2 == -1)
			{
				LOG_E("not empty cache store!!!");
				return;
			}
			
			act_mincar_send_smp(4, cache_store2+1);
			LOG_W("busy of car pos, has push to cache store %d \n", cache_store2+1);
		
		}
		
		
		cache_store = get_emer_store_sample(&smp_shelf, store_num);
		if(cache_store == -1)
		{
			return;
		}
		
		act_scan_sample_shelf_flows(&smp_shelf);
		rt_memcpy((uint8_t*)&cache_area_table[cache_store].smp, &smp_shelf, sizeof(smp_shelf_t));
		pop_cache_pos(cache_store, CACHE_POS_MIN_CAR); //push_cache_sample(&smp_shelf, CACHE_POS_MIN_CAR);
	}
	
	
}


void cmd_push_cache_store_handle(void *args)
{
	uint8_t err = 1;
	int smp_pos = -1;
	int cache_store = -1;
	smp_shelf_t des_smp = {0};
	local_pack_t *net_pack = (local_pack_t*)args;
	
	
	des_smp.len = net_pack->buf[0]; //sample shelf shape code size
	rt_memcpy(&des_smp.shape_code[0], &net_pack->buf[1], des_smp.len);  //sample shelf shape code
	rt_memcpy(tem_rsp_buf, &net_pack->buf[0], des_smp.len+4);  //get net pack buf data
	
	LOG_I("push sample in cache store \n");

	
	smp_pos = get_is_smp_in_place(&des_smp, CACHE_POS_MIN_CAR);
	if(smp_pos == CACHE_POS_MIN_CAR)
	{
		cache_store = push_cache_pos(smp_pos, CACHE_TYPES_CACHE_STORE, 0); //cache_store = push_cache_store(smp_pos, 0); //100
		if(cache_store == -1)
		{
			LOG_E("not empty cache store!!!\n");
			return;
		}

		act_mincar_send_smp(4, cache_store+1);
		LOG_D("push cache store %d done\n", cache_store+1);
		err = 0;
	}
	else
	{
		LOG_E("is empty of mini car!!!\n");
	}


	//local response
	if(local_get_communicate_err_cnt())
	{
		//err = 1;
	}
	
	local_transmit_response(net_pack, tem_rsp_buf, des_smp.len+4, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	
	
}




uint8_t push_tr_cache_pos(uint8_t track, uint8_t src_pos)
{
	uint8_t err = 1;
	int cache_store_id = -1;
	
	if(track == CAN_ADDR_EMER_TRACK)
	{
		if(cache_area_table[CACHE_POS_EMER_TR_CACHE].is_empty == IS_POS_NOT_EMPTY)
		{
			LOG_W("is busy of emer track cache pos\n");
			return err;
			
		}
		
		if(src_pos == CACHE_POS_MIN_CAR)
		{
			act_mincar_send_smp(0x06, 0x01);
			act_track_push_cache_pos(CAN_ADDR_EMER_TRACK);
			err = 0;
			
		}
		else if(src_pos >= CACHE_POS_CACHE_STORE_BASE && src_pos < (CACHE_POS_CACHE_STORE_BASE + CACHE_NUM_CACHE_STORE))
		{
			if(get_is_exsit_smp_of_cache_pos(CACHE_POS_MIN_CAR) == CACHE_POS_MIN_CAR)  //011, min car not empty
			{
				cache_store_id = push_cache_pos(CACHE_POS_MIN_CAR, CACHE_TYPES_CACHE_STORE, 0); 
				if(cache_store_id == -1)
				{
					LOG_E("not empty cache store!!!");
					return err;
				}
				
				act_mincar_send_smp(4, cache_store_id+1);
				LOG_W("busy of mini car, push smp to cache store %d at first\n", cache_store_id+1);
			}
			
			cache_store_id = (src_pos - CACHE_POS_CACHE_STORE_BASE);
			act_mincar_get_smp(4, cache_store_id+1);
			
			act_mincar_send_smp(0x06, 0x01);
			act_track_push_cache_pos(CAN_ADDR_EMER_TRACK);
			
			err = 0;
		
		}
		else
		{
			LOG_E("error track pos!\n");
		}

	}
	else if(track == CAN_ADDR_NORMAL_TRACK)
	{
		if(cache_area_table[CACHE_POS_NORMAL_TR_CACHE].is_empty == IS_POS_NOT_EMPTY)
		{
			LOG_W("is busy of normal track cache pos\n");
			return err;  
		}
		
		if(src_pos == CACHE_POS_MIN_CAR)
		{
			act_mincar_send_smp(0x06, 0x02);
			act_track_push_cache_pos(CAN_ADDR_NORMAL_TRACK);
			err = 0;
			
		}
		else if(src_pos >= CACHE_POS_CACHE_STORE_BASE && src_pos < (CACHE_POS_CACHE_STORE_BASE + CACHE_NUM_CACHE_STORE))
		{
			if(get_is_exsit_smp_of_cache_pos(CACHE_POS_MIN_CAR) == CACHE_POS_MIN_CAR)  //011, min car not empty
			{
				cache_store_id = push_cache_pos(CACHE_POS_MIN_CAR, CACHE_TYPES_CACHE_STORE, 0); 
				if(cache_store_id == -1)
				{
					LOG_E("not empty cache store!!!");
					return err;
				}
				
				act_mincar_send_smp(4, cache_store_id+1);
				LOG_W("busy of mini car, push smp to cache store %d at first\n", cache_store_id+1);
			}
			
			cache_store_id = (src_pos - CACHE_POS_CACHE_STORE_BASE);
			act_mincar_get_smp(4, cache_store_id+1);
			act_mincar_send_smp(0x06, 0x02);
			act_track_push_cache_pos(CAN_ADDR_NORMAL_TRACK);
			err = 0;
		
		}
		else
		{
			LOG_E("error track pos!\n");
		}
		
	}
	else
	{
		LOG_E("error track types!\n");
		return err;
	}
	
	return err;
	
}

uint8_t push_tr_add_smp_pos(uint8_t track, uint8_t src_pos)
{
	uint8_t err = 1;
	int smp_pos = -1;
	
	do
	{
		if(track == CAN_ADDR_EMER_TRACK) 
		{
			if(cache_area_table[CACHE_POS_EMER_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of emer track addtion pos\n");
				break;
			}
			
			if(src_pos == CACHE_POS_EMER_TR_CACHE)
			{
				act_track_push_addtion_pos(track);
				err = 0;
				break;
			
			}
			
			if(cache_area_table[CACHE_POS_EMER_TR_CACHE].is_empty == IS_POS_NOT_EMPTY )
			{
				LOG_W("is busy of emer track cache pos\n");
				break;
			}
			
			err = push_tr_cache_pos(track, src_pos);
			if(!err)
			{
				act_track_push_addtion_pos(track);
			}
			
			
			
		}
		else if(track == CAN_ADDR_NORMAL_TRACK)
		{
			if(cache_area_table[CACHE_POS_NORMAL_ADD_SMP].is_empty == 0)
			{
				LOG_E("is busy of normal track addtion pos\n");
				break;			
			}
			
			if(src_pos == CACHE_POS_NORMAL_TR_CACHE)
			{
				act_track_push_addtion_pos(track);
				err = 0;
				break;
			
			}
			
			err = push_tr_cache_pos(track, src_pos);
			if(!err)
			{
				act_track_push_addtion_pos(track);
			}

		}
		else
		{
			LOG_E("err track types!!!\n");
			err = 1;
		}
	
	} while(0);
	

	return err;

}


uint8_t push_tr_recyc_pos(uint8_t track, uint8_t src_pos)
{
	uint8_t err = 1;

	
	do
	{
		if(track == CAN_ADDR_EMER_TRACK)
		{
			if(cache_area_table[CACHE_POS_EMER_RECYC].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of emer track recyc pos\n");
				break;
			}
			
			if(src_pos == CACHE_POS_EMER_ADD_SMP)
			{
				act_track_push_recyc_pos(track);
				err = 0;
				break;
			}
			
			if(cache_area_table[CACHE_POS_EMER_ADD_SMP].is_empty == IS_POS_NOT_EMPTY )
			{
				LOG_W("is busy of emer track addtion pos\n");
				break;
			}
			
			if(src_pos == CACHE_POS_EMER_TR_CACHE)
			{
				act_track_push_addtion_pos(track);
				act_track_push_recyc_pos(track);
				err = 0;
				break;
			}
			
			if(cache_area_table[CACHE_POS_EMER_TR_CACHE].is_empty == IS_POS_NOT_EMPTY )
			{
				LOG_W("is busy of emer track cache pos\n");
				break;
			}
			
			err = push_tr_cache_pos(track, src_pos);
			if(!err)
			{
				act_track_push_addtion_pos(track);
				act_track_push_recyc_pos(track);
			}
			
			
	
		}
		else if(track == CAN_ADDR_NORMAL_TRACK)
		{
			if(cache_area_table[CACHE_POS_NORMAL_RECYC].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of emer track recyc pos\n");
				break;
			}
			
			if(src_pos == CACHE_POS_NORMAL_ADD_SMP)
			{
				act_track_push_recyc_pos(track);
				err = 0;
				break;
			}
			
			if(cache_area_table[CACHE_POS_NORMAL_ADD_SMP].is_empty == IS_POS_NOT_EMPTY )
			{
				LOG_W("is busy of emer track addtion pos\n");
				break;
			}
			
			if(src_pos == CACHE_POS_NORMAL_TR_CACHE)
			{
				act_track_push_addtion_pos(track);
				act_track_push_recyc_pos(track);
				err = 0;
				break;
			}
			
			if(cache_area_table[CACHE_POS_NORMAL_TR_CACHE].is_empty == IS_POS_NOT_EMPTY )
			{
				LOG_W("is busy of normal track cache pos\n");
				break;
			}
			
			err = push_tr_cache_pos(track, src_pos);
			if(!err)
			{
				act_track_push_addtion_pos(track);
				act_track_push_recyc_pos(track);
			}
			
			
		}
	
	}
	while(0);

	
	return err;

}

uint8_t pop_tr_pos(uint8_t track, uint8_t src_pos)
{
	int cache_pos = -1;
	uint32_t err = 1;
	
	if(get_is_exsit_smp_of_cache_pos(CACHE_POS_MIN_CAR) == CACHE_POS_MIN_CAR)
	{
		cache_pos = push_cache_pos(src_pos, CACHE_TYPES_CACHE_STORE, IS_SMP_DONE);
		if(cache_pos == -1)
		{
			LOG_E("no empty cache store of pop sample shelf!!!\n");
			return err;
		}
		act_mincar_send_smp(4, cache_pos + 1);
		
	}
	
	if(track == CAN_ADDR_EMER_TRACK)
	{
		if( src_pos == CACHE_POS_EMER_TR_CACHE && 
			cache_area_table[CACHE_POS_EMER_ADD_SMP].is_empty == IS_POS_EMPTY &&
		    cache_area_table[CACHE_POS_EMER_RECYC].is_empty == IS_POS_EMPTY )
		{
			act_track_push_addtion_pos(track);
			act_track_push_recyc_pos(track);
			act_track_pop_shelf(track);
			err = 0;
		}
		else if( src_pos == CACHE_POS_EMER_ADD_SMP && 
				 cache_area_table[CACHE_POS_EMER_RECYC].is_empty == IS_POS_EMPTY )
		{
			act_track_push_recyc_pos(track);
			act_track_pop_shelf(track);
			err = 0;
		}
		else if( src_pos == CACHE_POS_EMER_RECYC )
		{
			act_track_pop_shelf(track);
			err = 0;
		}
		else
		{
			if(cache_area_table[CACHE_POS_EMER_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of emer track addtion pos\n");
				err = 1;
			}
			
			if(cache_area_table[CACHE_POS_EMER_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of emer track recyc pos\n");
				err = 1;
			}
		}
	
	}
	else if(track == CAN_ADDR_NORMAL_TRACK)
	{
		if( src_pos == CACHE_POS_NORMAL_TR_CACHE && 
			cache_area_table[CACHE_POS_NORMAL_ADD_SMP].is_empty == IS_POS_EMPTY &&
		    cache_area_table[CACHE_POS_NORMAL_RECYC].is_empty == IS_POS_EMPTY )
		{
			act_track_push_addtion_pos(track);
			act_track_push_recyc_pos(track);
			act_track_pop_shelf(track);
			err = 0;
		}
		else if( src_pos == CACHE_POS_NORMAL_ADD_SMP && 
				 cache_area_table[CACHE_POS_NORMAL_RECYC].is_empty == IS_POS_EMPTY )
		{
			act_track_push_recyc_pos(track);
			act_track_pop_shelf(track);
			err = 0;
		}
		else if( src_pos == CACHE_POS_NORMAL_RECYC )
		{
			act_track_pop_shelf(track);
			err = 0;
		}
		else
		{
			if(cache_area_table[CACHE_POS_NORMAL_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of mormal track addtion pos\n");
				err = 1;
			}
			
			if(cache_area_table[CACHE_POS_NORMAL_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
			{
				LOG_W("is busy of mormal track recyc pos\n");
				err = 1;
			}
		}
	
	}
	else
	{
		LOG_E("err track types!!!\n");
	}
	
	return err;


}

void cmd_push_shelf_handle(void *args)
{
	uint8_t err = 1;
	uint8_t src_pos = 0;
	uint8_t track = 0;
	uint8_t des_tr_type = 0;
	uint8_t des_tr_pos = 0;
	uint8_t des_dev_id = 0;
	smp_shelf_t des_smp = {0};
	local_pack_t* net_pack = (local_pack_t*)args;
	
	des_smp.len = net_pack->buf[0]; //sample shelf shape code size
	rt_memcpy(&des_smp.shape_code[0], &net_pack->buf[1], des_smp.len);  //sample shelf shape code

	des_dev_id = net_pack->buf[des_smp.len+4];
	des_tr_type = net_pack->buf[des_smp.len+5];
	des_tr_pos = net_pack->buf[des_smp.len+6];
	rt_memcpy(tem_rsp_buf, &net_pack->buf[0], des_smp.len+7);  //get net pack buf data
	
	
	if(des_tr_type == DEST_TRACK_TYPES_EMER)
	{
		track = CAN_ADDR_EMER_TRACK;
	}
	else if(des_tr_type == DEST_TRACK_TYPES_NORMAL)
	{	
		track = CAN_ADDR_NORMAL_TRACK;
	}
	else if(des_tr_type == DEST_TRACK_TYPES_RECYC)
	{
		track = CAN_ADDR_RECOVERY_TRACK;
	}
	
	src_pos = get_smp_cache_pos(&des_smp);
	LOG_I("push sample in track %d pos %d \n", des_tr_type, des_tr_pos);

	if(des_tr_pos == DEST_TRACK_ADD_CACHE_POS) //push shelf to cache pos
	{
		err = push_tr_cache_pos(track, src_pos);
		if(!err)
		{
			//pop_cache_pos(src_pos, CACHE_POS_EMER_TR_CACHE);
			if(track == CAN_ADDR_EMER_TRACK)
			{
				pop_cache_pos(src_pos, CACHE_POS_EMER_TR_CACHE);
			}
			else if(track == CAN_ADDR_NORMAL_TRACK)
			{
				pop_cache_pos(src_pos, CACHE_POS_NORMAL_TR_CACHE);
			}
		}
	
	}
	else if(des_tr_pos == DEST_TRACK_ADD_POS)  //push shelf to add sample pos
	{
		err = push_tr_add_smp_pos(track, src_pos);
		if(!err)
		{
			if(track == CAN_ADDR_EMER_TRACK)
			{
				pop_cache_pos(src_pos, CACHE_POS_EMER_ADD_SMP);
			}
			else if(track == CAN_ADDR_NORMAL_TRACK)
			{
				pop_cache_pos(src_pos, CACHE_POS_NORMAL_ADD_SMP);
			}
		}

	}
	else if(des_tr_pos == DEST_TRACK_RECYC_POS)
	{
		err = push_tr_recyc_pos(track, src_pos);
		if(!err)
		{
			if(track == CAN_ADDR_EMER_TRACK)
			{
				pop_cache_pos(src_pos, CACHE_POS_EMER_RECYC);
			}
			else if(track == CAN_ADDR_NORMAL_TRACK)
			{
				pop_cache_pos(src_pos, CACHE_POS_NORMAL_RECYC);
			}
			
		}
	
	}
	else
	{
		LOG_E("error push track pos %d\n", des_tr_pos);
	}
	

	//err = local_get_communicate_err();
	if(local_get_communicate_err_cnt())
	{
		//err = 1;
	}
	
	
	//local response
	local_transmit_response(net_pack, tem_rsp_buf, des_smp.len+4, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	if(err)
	{
		LOG_E("push shelf fail\n");
	}
	else
	{
		LOG_I("push shelf sucessful err code %d!\n", err);
	}
	

}



void cmd_move_shelf_handle(void *args)
{
	uint8_t err = 0;
	uint8_t des_dev_id = 0;
	uint8_t des_tr_type = 0;
	uint8_t des_tr_pos = 0;
	uint8_t des_hole = 0;
	smp_shelf_t des_smp = {0};
	local_pack_t *net_pack = (local_pack_t*)args;
	
	
	des_smp.len = net_pack->buf[0]; //sample shelf shape code size
	rt_memcpy(&des_smp.shape_code[0], &net_pack->buf[1], des_smp.len);  //sample shelf shape code

	des_dev_id = net_pack->buf[des_smp.len+4];
	des_tr_type = net_pack->buf[des_smp.len+5];
	des_tr_pos = net_pack->buf[des_smp.len+6];
	des_hole = net_pack->buf[des_smp.len+7];
	rt_memcpy(tem_rsp_buf, &net_pack->buf[0], des_smp.len+8);  //get net pack buf data
	
	LOG_I("track %d move sample hole pos: \n", des_tr_type, des_hole);
	err = 1;
	
	if(des_tr_type == DEST_TRACK_TYPES_EMER)  //get track type: 0x01:emer 0x02:normal 0x03:recover
	{
		if(cache_area_table[CACHE_POS_EMER_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
		{
			act_move_shelf(CAN_ADDR_EMER_TRACK, des_hole);
			err = 0;
		}
		else
		{
			LOG_E("no found sample\n");
		}

	}
	else if(des_tr_type == DEST_TRACK_TYPES_NORMAL) //0x02:normal
	{
		if(cache_area_table[CACHE_POS_NORMAL_ADD_SMP].is_empty == IS_POS_NOT_EMPTY)
		{
			act_move_shelf(CAN_ADDR_NORMAL_TRACK, des_hole);
			err = 0;
		}
		else
		{
			LOG_E("no found sample\n");
		}

	}
	
	
	if(local_get_communicate_err_cnt())
	{
		//err = 1;
	}
	//local response
	local_transmit_response(net_pack, tem_rsp_buf, des_smp.len+4, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
		
}


void cmd_pop_shelf_handle(void *args)
{
	uint32_t err = 1;
	uint32_t err_type = 0;
	uint8_t track = 0;
	uint8_t src_pos = 0;
	
	uint8_t des_tr_type = 0;
	uint8_t des_tr_pos = 0;
	uint8_t des_dev_id = 0;
	
	uint8_t cmd_pop_pos = 0;
	uint8_t pop_pos = 0;
	uint8_t pop_slot = 0;

	int cache_pos = -1;
	int cache_pos2 = -1;
	smp_shelf_t des_smp = {0};
	local_pack_t *net_pack = (local_pack_t*)args;
	
	NET_GET_INDEX_LEV3(cmd_pop_pos, net_pack->cmd); //get mt id

	des_smp.len = net_pack->buf[0]; //sample shelf shape code size
	rt_memcpy(&des_smp.shape_code[0], &net_pack->buf[1], des_smp.len);  //sample shelf shape code
	
	des_dev_id = net_pack->buf[des_smp.len+4];
	des_tr_type = net_pack->buf[des_smp.len+5];
	des_tr_pos = net_pack->buf[des_smp.len+6];
	rt_memcpy(tem_rsp_buf, &net_pack->buf[0], des_smp.len+7);  //get net pack buf data
	

	LOG_I("track %d pop-out sample \n", track);
	src_pos = get_smp_cache_pos(&des_smp);

	do
	{	
		if(des_tr_type == DEST_TRACK_TYPES_EMER)
		{
			track = CAN_ADDR_EMER_TRACK;
		}
		else if(des_tr_type == DEST_TRACK_TYPES_NORMAL)
		{	
			track = CAN_ADDR_NORMAL_TRACK;
		}
		else if(des_tr_type == DEST_TRACK_TYPES_RECYC)
		{
			track = CAN_ADDR_RECOVERY_TRACK;
		}
		else if(src_pos == CACHE_POS_MIN_CAR)
		{
			//act_mincar_send_smp(pop_pos, pop_slot);
			//err = 0;
			LOG_I("pop shelf from mini car\n"); 
			
			//break;	
		}
		else
		{
			LOG_E("err track types!!!\n");
			break;
		}
		
		if(cmd_pop_pos == 0) //pop to cache store
		{
			cache_pos = push_cache_pos(src_pos, CACHE_TYPES_CACHE_STORE, IS_SMP_DONE); 
			pop_pos = 4; //cache store 
			pop_slot = cache_pos + 1;
			if(cache_pos == -1)
			{
				err_type = 2;
				LOG_E("not empty cache store!!!\n");
				break;
			}
			
			dev_pop_smple_baseket_pos = pop_pos;
			dev_pop_smple_baseket_slot = pop_slot;
		}
		else if(cmd_pop_pos == 1) //pop to basket
		{
			cache_pos = push_cache_pos(src_pos, CACHE_TYPES_BASKET, IS_SMP_DONE); 
			pop_pos =  (cache_pos / CACHE_NUM_BASKET_SLOT) + 1; 
			pop_slot = (cache_pos %  CACHE_NUM_BASKET_SLOT) + 1;
			
			if(cache_pos == -1)
			{
				err_type = 1;
				LOG_E("not empty basket slot!!!\n");
				break;
			}
			
			dev_pop_smple_baseket_pos = pop_pos;
			dev_pop_smple_baseket_slot = pop_slot;
		
		}
		else
		{
			LOG_E("err pop pos!!!\n");
			break;
		}
		
		if(src_pos == CACHE_POS_MIN_CAR)
		{
			LOG_I("pop from mini car ok\n"); 
			act_mincar_send_smp(pop_pos, pop_slot);
			err = 0;
			break;	
		}
		
	
		err = pop_tr_pos(track, src_pos);
		
		if(!err)
		{
			act_mincar_send_smp(pop_pos, pop_slot);
			
		}
		
	}
	while(0);
	
	
	if(cmd_pop_pos == 1)
	{
		act_set_basket_led_status(pop_pos, pop_slot, LED_COLOR_G);
	}
	
	if(local_get_communicate_err_cnt())
	{
		//err = 1;
	}
	
	if(err_type)
	{
		err = err_type;
	}
	else if(err)
	{
		err = 3;
	}

	local_transmit_response(net_pack, tem_rsp_buf, des_smp.len+4, PACK_TYPE_DATA);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	
	//report pop pos
	net_pack->cmd = 0x00000550;
	tem_rsp_buf[des_smp.len+2] = dev_pop_smple_baseket_pos;
	tem_rsp_buf[des_smp.len+3] = dev_pop_smple_baseket_slot;
	local_transmit_response(net_pack, tem_rsp_buf, des_smp.len+4, PACK_TYPE_REPORT);
	
}


void dev_update_basket_cmd1(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x01;

	req_basket_update_handle(&pack);
	//req_basket_start_test_handle(&pack);
}

void dev_update_basket_cmd2(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x02;
	
	req_basket_update_handle(&pack);
	//req_basket_start_test_handle(&pack);
}

void dev_update_basket_cmd3(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x03;
	
	req_basket_update_handle(&pack);
	//req_basket_start_test_handle(&pack);
}

void dev_inser_sample_cmd(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x02;
	pack.buf[1] = 0x05;
	
	req_basket_inser_sample_handle(&pack);
}

void dev_emer_store_inser_sample_cmd1(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x01;

	req_emer_inser_sample_handle(&pack);
	req_emer_start_test_handle(&pack);
}


void dev_emer_store_inser_sample_cmd2(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x02;

	req_emer_inser_sample_handle(&pack);
	req_emer_start_test_handle(&pack);
}



void dev_emer_store_move_sample_cmd(void)
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	pack.buf[0] = 0x01;

	req_emer_remove_sample_handle(&pack);
	
}






void cmd_dev_stop_handle(void *args)
{
	
	uint8_t param[8] = {0};
	uint32_t err = 0;
	local_pack_t *net_pack = (local_pack_t*)args;

	act_dev_status_reset();
	//local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_ACK);
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);
	udp_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_ACK);
	

}


extern void clen_net_response(void);
void cmd_dev_reset_handle(void *args)
{
	uint8_t buf[6] = {0};
	uint32_t err = 0;
	local_pack_t *net_pack = (local_pack_t*)args;
	
	clen_net_response();
		
//	//1.recover track
//	buf[0] = 1;
//	//send frame
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_NONE);
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_NONE);
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_NONE);
//	
//	//wait  response
//	local_wait_rsp_frame(0x0301, buf, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
//	local_wait_rsp_frame(0x0301, buf, CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);
//	local_wait_rsp_frame(0x0301, buf, CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES);
//	
//	//send frame
//	buf[0] = 2;
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_NONE);
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_NONE);
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_NONE);
//	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_NONE);
//	
//	//wait  response
//	local_wait_rsp_frame(0x0301, buf, CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
//	local_wait_rsp_frame(0x0301, buf, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
//	local_wait_rsp_frame(0x0301, buf, CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);
//	err = local_wait_rsp_frame(0x0301, buf, CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES);

	buf[0] = 1;
	//send frame
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES);
	
	//
	buf[0] = 2;
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_RECOVERY_TRACK, CAN_WAIT_FLAG_RES);
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_EMER_TRACK, CAN_WAIT_FLAG_RES);
	local_send_ctrl_frame(0x0301, buf, CAN_ADDR_NORMAL_TRACK, CAN_WAIT_FLAG_RES);
	
	act_dev_status_reset();
	
	
	if(local_get_communicate_err_cnt())
	{
		//err = 1;
	}
	local_transmit_response(net_pack, (uint8_t*)&err, 4, PACK_TYPE_RESULT);

}

void reset_mincar_cmd(void)
{
	uint8_t param[6] = {0};
	//	//local_send_ctrl_frame
	//2.sample table
	param[0] = 1;
	local_send_ctrl_frame(0x0301, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
	
	//2.sample table
	param[0] = 2;
	local_send_ctrl_frame(0x0301, &param[0], CAN_ADDR_SAMPLE_PLTAFROM, CAN_WAIT_FLAG_RES);  
	

}

void min_car_get_smp(void)
{
	act_mincar_get_smp(2, 2);
}




void dev_reset_cmd(void)
{
	//cmd_dev_reset_handle(RT_NULL);
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(4);
	dp = (uint32_t)local_pack;
	
	local_pack->cmd = NET_CMD_DEV_RESET;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = 4;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));
	
}



void scan_basket_sample_cmd(void)
{
	smp_shelf_t smp = {0};
	smp.basket_pos = 2;
	smp.slot_pos = 1;
	smp.is_done = 0;
	smp.shape_code[0] = 0;
	smp.shape_code[1] = 1;
	smp.shape_code[2] = 2;
	act_scan_sample_shelf_flows(&smp);
	

	
	act_mincar_send_smp(2, 1);
	
}

void scan_sample_emer_test(void)
{
	smp_shelf_t smp = {0};
	smp.basket_pos = 5;
	smp.slot_pos = 1;
	smp.is_done = 0;
	smp.shape_code[0] = 0;
	smp.shape_code[1] = 1;
	smp.shape_code[2] = 2;
	act_scan_sample_shelf_flows(&smp); //act_scan_sample_shelf_flows(&smp);
	
	act_mincar_send_smp(5, 1);
	
}




void dev_push_cache_store_cmd(void)
{

	//cmd_push_cache_store_handle(RT_NULL);
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+4);
	dp = (uint32_t)local_pack;
	
	local_pack->cmd = NET_CMD_MOVE_CACHE_REG;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	
	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[0], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0;
	local_pack->buf[globle_smp_shelf.len+2] = 0;
	local_pack->buf[globle_smp_shelf.len+3] = 0;
	
	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}






void dev_push_tr1_add_pos_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[0], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0x01; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = 0x01; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = 0x01; //basket sslot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x01; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_push_tr1_cache_pos_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[0], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0x01; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = 0x01; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = 0x01; //basket sslot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x01; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x02; //track pos id (1:add sample pos 2: cache pos)
	
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	rt_mq_send(network_command_queue, &dp, sizeof(void*));


}


void dev_push_tr1_recyc_pos_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[0], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0x01; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = 0x01; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = 0x01; //basket sslot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x01; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x03; //track pos id (1:add sample pos 2: cache pos 3:recycy pos)
	
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	rt_mq_send(network_command_queue, &dp, sizeof(void*));


}

void dev_tr1_pop_to_cache_store_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[0], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = BASKET_TYPES_NORMAL; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = globle_smp_shelf.basket_pos; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = globle_smp_shelf.slot_pos; //basket slot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x01; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos 3:recycy pos)
	

	
	local_pack->cmd = NET_CMD_POP_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;


	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_tr1_pop_to_basket_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[0], globle_smp_shelf.len);
	
	
	local_pack->buf[globle_smp_shelf.len+1] = BASKET_TYPES_NORMAL; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = globle_smp_shelf.basket_pos; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = globle_smp_shelf.slot_pos; //basket slot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x01; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	

	local_pack->cmd = NET_CMD_POP_SHELF|0x00010000;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_tr1_move_shelf_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+8);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = BASKET_TYPES_NORMAL; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = globle_smp_shelf.basket_pos; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = globle_smp_shelf.slot_pos; //basket slot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x01; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	local_pack->buf[globle_smp_shelf.len+7] = 0x09; //hoile pos 
	
	
	local_pack->cmd = NET_CMD_MOVE_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = globle_smp_shelf.len+8;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));
	
}




void dev_push_tr2_add_pos_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0x01; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = 0x01; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = 0x01; //basket sslot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x02; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_push_tr2_cache_pos_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0x01; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = 0x01; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = 0x01; //basket sslot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x02; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x02; //track pos id (1:add sample pos 2: cache pos)
	
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	rt_mq_send(network_command_queue, &dp, sizeof(void*));


}


void dev_push_tr2_recyc_pos_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = 0x01; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = 0x01; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = 0x01; //basket sslot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x02; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x03; //track pos id (1:add sample pos 2: cache pos 3:recycy pos)
	
	local_pack->cmd = NET_CMD_PUSH_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	rt_mq_send(network_command_queue, &dp, sizeof(void*));


}

void dev_tr2_pop_to_cache_store_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = BASKET_TYPES_NORMAL; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = globle_smp_shelf.basket_pos; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = globle_smp_shelf.slot_pos; //basket slot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x02; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos 3:recycy pos)
	

	
	local_pack->cmd = NET_CMD_POP_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;


	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_tr2_pop_to_basket_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+7);
	dp = (uint32_t)local_pack;
	
	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	
	local_pack->buf[globle_smp_shelf.len+1] = BASKET_TYPES_NORMAL; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = globle_smp_shelf.basket_pos; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = globle_smp_shelf.slot_pos; //basket slot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x02; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	

	local_pack->cmd = NET_CMD_POP_SHELF|0x00010000;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));

}


void dev_tr2_move_shelf_cmd(void)
{
	uint32_t dp = 0;
	local_pack_t *local_pack = rt_malloc(sizeof(local_pack_t));
	local_pack->buf = rt_malloc(globle_smp_shelf.len+8);
	dp = (uint32_t)local_pack;
	

	local_pack->buf[0] = globle_smp_shelf.len;
	rt_memcpy(&local_pack->buf[1], &globle_smp_shelf.shape_code[1], globle_smp_shelf.len);
	
	local_pack->buf[globle_smp_shelf.len+1] = BASKET_TYPES_NORMAL; //basket type
	local_pack->buf[globle_smp_shelf.len+2] = globle_smp_shelf.basket_pos; //basket num
	local_pack->buf[globle_smp_shelf.len+3] = globle_smp_shelf.slot_pos; //basket slot
	
	local_pack->buf[globle_smp_shelf.len+4] = 0x01; //dev id
	local_pack->buf[globle_smp_shelf.len+5] = 0x02; //track id
	local_pack->buf[globle_smp_shelf.len+6] = 0x01; //track pos id (1:add sample pos 2: cache pos)
	local_pack->buf[globle_smp_shelf.len+7] = 0x09; //hoile pos 
	
	
	local_pack->cmd = NET_CMD_MOVE_SHELF;
	local_pack->dest_addr = LOCAL_NET_ADDR;
	local_pack->type = PACK_TYPE_CMD;
	local_pack->len = globle_smp_shelf.len+8;

	rt_mq_send(network_command_queue, &dp, sizeof(void*));
	
}



void dev_init_cache_cmd(void)
{
	cache_pos_initail();
}

void start_emer_store_test_cmd1(void)   
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	
	
	pack.buf[0] = 1;
 	req_emer_start_test_handle(&pack);

}


void start_emer_store_test_cmd2(void)   
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	
	
	pack.buf[0] = 2;
 	req_emer_start_test_handle(&pack);

}

void start_baske_test_cmd1(void)   
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	
	
	pack.buf[0] = 1;
 	req_basket_start_test_handle(&pack);

}

void start_baske_test_cmd2(void)   
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	
	
	pack.buf[0] = 2;
 	req_basket_start_test_handle(&pack);

}

void start_baske_test_cmd3(void)   
{
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	pack.buf = buf;
	
	
	pack.buf[0] = 3;
 	req_basket_start_test_handle(&pack);

}

void dev_set_basket_led_cmd(void)
{
	act_set_basket_led_status(2, 1, 1);
	rt_thread_delay(1000);
	act_set_basket_led_status(2, 1, 2);
	rt_thread_delay(1000);
	act_set_basket_led_status(2, 1, 3);
	rt_thread_delay(1000);
	act_set_basket_led_status(2, 1, 4);
	rt_thread_delay(1000);
	
}

void dev_set_emer_store_led_cmd(void)
{
	act_set_emer_store_led_status(1, 1);
	rt_thread_delay(1000);
	act_set_emer_store_led_status(1, 2);
	rt_thread_delay(1000);
	act_set_emer_store_led_status(1, 3);
	rt_thread_delay(1000);
	act_set_emer_store_led_status(1, 4);
	rt_thread_delay(1000);
}

void dev_led_init(void)
{
	uint8_t pos = 0;
	uint8_t solt = 0;
	
	act_set_emer_store_led_status(1, LED_COLOR_OFF);
	act_set_emer_store_led_status(2, LED_COLOR_OFF);
	
	act_get_basket_status(1);
	act_get_basket_status(2);
	act_get_basket_status(3);
	
//	for(pos = 0; pos < 3; pos++)
//	{
//		for(solt = 0; solt < 13; solt++)
//		{
//			act_set_basket_led_status(pos+1, solt+1, LED_COLOR_OFF);
//		}
//	}
	
}



//extern void dev_scaner_read_data(void);
void open_scanner_cmd(void)
{
	uint8_t len = 0;   
	//scanner_dev_clean_stream();
	
	act_scan_continue(RT_NULL, 0);  //start scan

	while(1)
	{
		scanner_dev_clean_stream();
		//dev_scaner_read_data();
		
	}
}

void dev_start_conex_scanner(void)
{
	uint8_t recv_size = 0;
	rt_memset(smp_shape_code_buf, 0, sizeof(smp_shape_code_buf));
	scanner_dev_clean_stream();
	scanner_dev_ctrl(SCANNER_CMD_start, SCANNER_TYPE_cognex, 0);  //send start scan command
	rt_thread_delay(100);
	recv_size = scanner_dev_get_data(&smp_shape_code_buf[0], 20, 100);
	
	LOG_I("size %d: %s\n", recv_size, smp_shape_code_buf);
	
//	for(uint8_t i = 0; i < recv_size; i++)
//	{
//		rt_kprintf("%02x ", smp_shape_code_buf[i]);
//	
//	}
	

}


void scaner_get_date_cmd(void)
{
	uint8_t len = 0;
	while(1)
	{
		len = scanner_dev_get_data(&smp_shape_code_buf[0], 20, 50);
		if(len)
		{
			LOG_I("size: %d %s\n", len, smp_shape_code_buf);
		}
		
		rt_thread_delay(100);
	
	}
	


}



MSH_CMD_EXPORT(dev_init_cache_cmd, "dev_init_cache_cmd");
MSH_CMD_EXPORT(dev_led_init, "dev_led_init");
MSH_CMD_EXPORT(dev_update_basket_cmd1, "dev_check_basket_cmd");
MSH_CMD_EXPORT(dev_update_basket_cmd2, "dev_check_basket_cmd");
MSH_CMD_EXPORT(dev_update_basket_cmd3, "dev_check_basket_cmd");
MSH_CMD_EXPORT(start_emer_store_test_cmd1, "start_emer_store_test_cmd");
MSH_CMD_EXPORT(start_emer_store_test_cmd2, "start_emer_store_test_cmd");
MSH_CMD_EXPORT(start_baske_test_cmd1, "start_baske_test_cmd1");
MSH_CMD_EXPORT(start_baske_test_cmd2, "start_baske_test_cmd1");
MSH_CMD_EXPORT(start_baske_test_cmd3, "start_baske_test_cmd1");
MSH_CMD_EXPORT(dev_inser_sample_cmd, "dev_inser_sample_cmd");
MSH_CMD_EXPORT(dev_emer_store_inser_sample_cmd1, "dev_emer_store_inser_sample_cmd");
MSH_CMD_EXPORT(dev_emer_store_inser_sample_cmd2, "dev_emer_store_inser_sample_cmd");
MSH_CMD_EXPORT(dev_set_emer_store_led_cmd, "dev_set_emer_store_led_cmd");
MSH_CMD_EXPORT(dev_set_basket_led_cmd, "dev_set_basket_led_cmd");
MSH_CMD_EXPORT(scan_sample_emer_test, "scan_sample_emer_test");


MSH_CMD_EXPORT(dev_reset_cmd, "dev_reset_cmd");
MSH_CMD_EXPORT(dev_push_cache_store_cmd, "dev_push_cache_cmd");

MSH_CMD_EXPORT(dev_push_tr1_cache_pos_cmd, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_tr1_add_pos_cmd, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_push_tr1_recyc_pos_cmd, "dev_push_shelf_recyc_pos_cmd");
MSH_CMD_EXPORT(dev_tr1_move_shelf_cmd, "dev_move_shelf_cmd");
MSH_CMD_EXPORT(dev_tr1_pop_to_cache_store_cmd, "dev_push_shelf_cmd"); 
MSH_CMD_EXPORT(dev_tr1_pop_to_basket_cmd, "dev_tr1_pop_to_basket_cmd"); 


MSH_CMD_EXPORT(dev_push_tr2_cache_pos_cmd, "dev_push_shelf_cache_pos_cmd");
MSH_CMD_EXPORT(dev_push_tr2_add_pos_cmd, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_push_tr2_recyc_pos_cmd, "dev_push_shelf_recyc_pos_cmd");
MSH_CMD_EXPORT(dev_tr2_move_shelf_cmd, "dev_move_shelf_cmd");
MSH_CMD_EXPORT(dev_tr2_pop_to_cache_store_cmd, "dev_push_shelf_cmd");
MSH_CMD_EXPORT(dev_tr2_pop_to_basket_cmd, "dev_tr2_pop_to_basket_cmd");


MSH_CMD_EXPORT(scan_basket_sample_cmd, "scan_basket_sample_cmd");
MSH_CMD_EXPORT(open_scanner_cmd, "open_scanner_cmd");
MSH_CMD_EXPORT(reset_mincar_cmd, "reset_mincar_cmd");
MSH_CMD_EXPORT(dev_start_conex_scanner, "dev_start_scanner");
MSH_CMD_EXPORT(scaner_get_date_cmd, "scaner_get_date_cmd");

