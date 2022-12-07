#include "dev_can.h"


#define LOG_TAG "can"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

static uint8_t can_pack_serial = 0;
static rt_device_t can_dev = RT_NULL;
static struct rt_semaphore can_rx_sem;
static rt_err_t can_rx_callback(rt_device_t dev, rt_size_t size);
rt_mq_t can_report_queue = RT_NULL;
rt_mq_t can_response_queue = RT_NULL;
rt_mutex_t can_rsp_mux = RT_NULL; 

void can_device_init(void)
{

    can_dev = rt_device_find("fdcan1");
	rt_sem_init(&can_rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
	
    if (can_dev == RT_NULL)
    {
        LOG_D("not find can1");
    }
		
    if (RT_EOK != rt_device_open(can_dev, RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX))
    {
        LOG_D("opend can1 failed.");
    }
		
    //register can recivece callback
    rt_device_set_rx_indicate(can_dev, can_rx_callback);
    rt_device_control(can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN500kBaud);
    rt_device_control(can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_NORMAL);
}




static rt_err_t can_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&can_rx_sem);
    return RT_EOK;
}


rt_err_t can_dev_get_msg(rt_can_msg_t msg)
{
	//struct rt_can_msg can_rxmsg = {0};
	rt_err_t err = RT_EOK;
	err = rt_sem_take(&can_rx_sem, RT_WAITING_NO);
	
	if(err == RT_EOK) //none block wait for semaphore, take semaphore
	{
		msg->hdr = -1;
		rt_device_read(can_dev, 0, msg, sizeof(struct rt_can_msg));  //read can device recived data
	}
	
	return err;
	
}





size_t can_device_send_ack(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t dest_addr, uint16_t serial, uint8_t type, uint8_t len)
{
	rt_size_t size = 0;
	uint32_t msg_id = 0;
	struct rt_can_msg msg = {0};
		
	if(can_dev)
	{
		SET_CAN_PACK_saddr(msg_id, src_addr); //set can frame info to id field
		SET_CAN_PACK_daddr(msg_id, dest_addr);
		SET_CAN_PACK_serial(msg_id, serial);
		SET_CAN_PACK_TYPE(msg_id, type);
		
		msg.ide = RT_CAN_EXTID; //extends frame
		msg.rtr = RT_CAN_DTR;   //data framre
		msg.len = len+2;	    //data len
		msg.id = msg_id;

		rt_memset(&msg.data[0], 0, 8);
		rt_memcpy(&msg.data[0], &cmd, 2);
		rt_memcpy(&msg.data[2], buf, len);
		
		size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
	
	}
	
	return size;

}





size_t can_device_send_frame(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t dest_addr, uint8_t serial, uint8_t type, uint8_t len)
{
	rt_size_t size = 0;
	uint32_t msg_id = 0;
	struct rt_can_msg msg = {0};
	uint16_t ms = 0;
	uint8_t times[4] = {0};
	
	if(can_dev)
	{
		
		SET_CAN_PACK_saddr(msg_id, src_addr); //set can frame info to id field
		SET_CAN_PACK_daddr(msg_id, dest_addr);
		SET_CAN_PACK_serial(msg_id, serial); //pack->serial;
		SET_CAN_PACK_TYPE(msg_id, type);
		
		msg.ide = RT_CAN_EXTID; //extends frame
		msg.rtr = RT_CAN_DTR;   //data framre
		msg.len = len+2;	//data len
		msg.id = msg_id;
		
		
		rt_memset(&msg.data[0], 0, 8);
		rt_memcpy(&msg.data[0], &cmd, 2);
		rt_memcpy(&msg.data[2], buf, len);
		
		size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
	
	}
	
	get_timestamp(&times[0], &times[1], &times[2], &ms);
	LOG_D("%02d:%02d:%02d:%03d [%02x >> %02x] %02x %02x %02x %02x %02x %02x %02x %02x\n", times[0], times[1], times[2], times[3], src_addr, dest_addr,
	msg.data[0], msg.data[1], msg.data[2], msg.data[3], msg.data[4], msg.data[5], msg.data[6], msg.data[7]);

	return size;

}


//rt_err_t can_device_get_rsp_frame(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t serial, uint8_t type, rt_tick_t timeout)
//{
//	uint8_t *pcmd = RT_NULL;
//	uint32_t dp = 0;
//	rt_err_t err = 1;
//	uint8_t times[3] = {0};
//	uint16_t ms = {0};
//	local_pack_t *pack = RT_NULL;
//	rt_tick_t tick = 0;
//	
//	while(1)
//	{
//		rt_thread_delay(1);
//		err = rt_mq_recv(can_response_queue, &dp, sizeof(void*), 1);
//		if(err == RT_EOK) 
//		{
//			pack = (local_pack_t*)dp;
//			
//			if( pack->type == type && 
//				pack->cmd == cmd && 
//			    pack->src_addr == src_addr &&
//			    pack->serial == serial)
//			{
//				rt_memcpy(buf, pack->buf, 6);
//				pcmd = (uint8_t*)&pack->cmd;
//				get_timestamp(&times[0], &times[1], &times[2], &ms);

//				LOG_D("%02d:%02d:%02d:%03d [%02x << %02x] %02x %02x %02x %02x %02x %02x %02x %02x\n",  
//				times[0], times[1], times[2], ms, pack->dest_addr, pack->src_addr, pcmd[0], pcmd[1], 
//				pack->buf[0], pack->buf[1], pack->buf[2], pack->buf[3], pack->buf[4], pack->buf[5]);
//				
//				if(type == PACK_TYPE_RESULT)
//				{
//					err = pack->buf[1];
//				}
//				else
//				{
//					err = RT_EOK;
//				}
//				
//				
//				if(pack->buf)
//				{
//					rt_free(pack->buf);
//					pack->buf = RT_NULL;
//				}
//				
//				if(pack)
//				{
//					rt_free(pack);
//					pack = RT_NULL;
//				}
//				
//				break;
//				//can_device_send_ack(pack->cmd, pack->buf, pack->dest_addr, pack->src_addr, pack->serial, PACK_TYPE_ACK, 6);
//				
//			}
//			else
//			{
//				rt_mq_send(can_response_queue, &dp, sizeof(void*));

//			}
//				
//		}

//		tick++;
//		if(tick > timeout)
//		{
//			err = 1;
//			break;
//		}
//		
//		
//	
//	}
//	
//	return err;

//}


//static struct rt_thread can_recv_handle;
//void can_recv_thread(void *args)
//{
//	uint32_t dp = 0;
//	struct rt_can_msg msg = {0};
//	local_pack_t *pack = RT_NULL;
//	
//	while(1)
//	{
//		if(can_dev_get_msg(&msg) == RT_EOK)
//		{

//			switch(GET_CAN_PACK_TYPE(msg.id))
//			{
//				#if 1
//				case PACK_TYPE_ACK:
//				{
//					pack = rt_malloc(sizeof(local_pack_t));
//					pack->buf = rt_malloc(6);
//					rt_memset(pack->buf, 0, 6);
//				
//					pack->len = msg.len;
//					pack->serial = GET_CAN_PACK_serial(msg.id);
//					pack->type = GET_CAN_PACK_TYPE(msg.id);
//					pack->src_addr = GET_CAN_PACK_saddr(msg.id);
//					pack->dest_addr = CAN_LOCAL_ADDR;
//					pack->cmd = *((uint16_t*)&msg.data[0]);
//					rt_memcpy(pack->buf, &msg.data[2], pack->len);
//					
//					dp = (uint32_t)pack;
//					rt_mq_send(can_response_queue, &dp, sizeof(void*));
//					break;
//				}
//				#endif
//				
//				case PACK_TYPE_RESULT:
//				case PACK_TYPE_DATA:
//				{
//					pack = rt_malloc(sizeof(local_pack_t));
//					pack->buf = rt_malloc(6);
//					rt_memset(pack->buf, 0, 6);
//				
//					pack->len = msg.len;
//					pack->serial = GET_CAN_PACK_serial(msg.id);
//					pack->type = GET_CAN_PACK_TYPE(msg.id);
//					pack->src_addr = GET_CAN_PACK_saddr(msg.id);
//					pack->dest_addr = CAN_LOCAL_ADDR;
//					pack->cmd = *((uint16_t*)&msg.data[0]);
//					rt_memcpy(pack->buf, &msg.data[2], pack->len);
//					
//					dp = (uint32_t)pack;
//					can_device_send_ack(pack->cmd, pack->buf, pack->dest_addr, pack->src_addr, pack->serial, PACK_TYPE_ACK, 6);
//					rt_mq_send(can_response_queue, &dp, sizeof(void*));

//					break;
//				}
//				
//				
//				case PACK_TYPE_REPORT:
//				{
//					pack = rt_malloc(sizeof(local_pack_t));
//					pack->buf = rt_malloc(6);
//					rt_memset(pack->buf, 0, 6);
//				
//					pack->len = msg.len;
//					pack->serial = GET_CAN_PACK_serial(msg.id);
//					pack->type = GET_CAN_PACK_TYPE(msg.id);
//					pack->src_addr = GET_CAN_PACK_saddr(msg.id);
//					pack->dest_addr = CAN_LOCAL_ADDR;
//					pack->cmd = *((uint16_t*)&msg.data[0]);
//					rt_memcpy(pack->buf, &msg.data[2], pack->len);
//					
//					dp = (uint32_t)pack;
//					can_device_send_ack(pack->cmd, pack->buf, pack->dest_addr, pack->src_addr, pack->serial, PACK_TYPE_ACK, 6);
//					rt_mq_send(can_report_queue, &dp, sizeof(void*));	
//					
//					break;
//				}
//					
//			}
//			
//			
//		}
//		
//		rt_thread_delay(1);
//	}
//	
//}


#define CAN_RESPONSE_FRAME_MAX (100)
static uint8_t can_response_index = 0;
static can_rsp_t can_response[CAN_RESPONSE_FRAME_MAX] = {0};

void set_can_response(local_pack_t *pack)
{
	rt_mutex_take(can_rsp_mux, RT_WAITING_FOREVER);
	can_response_index++;
	can_response_index %= CAN_RESPONSE_FRAME_MAX;
	
	can_response[can_response_index].cmd = pack->cmd;
	can_response[can_response_index].serial = pack->serial;
	can_response[can_response_index].src_addr = pack->src_addr;
	can_response[can_response_index].dest_addr = pack->dest_addr;
	can_response[can_response_index].type = pack->type;
	rt_memcpy(&can_response[can_response_index].buf[0], &pack->buf[0], pack->len);
	
	rt_mutex_release(can_rsp_mux);

}


uint8_t get_can_response(uint8_t *buf, uint16_t cmd, uint16_t serial, uint8_t src_addr, uint8_t type, rt_tick_t timeout)
{
	rt_err_t err = 1;
	uint8_t index = 0;
	
	rt_tick_t current_tick = 0;
	while(1)
	{
		rt_thread_delay(1);
		rt_mutex_take(can_rsp_mux, RT_WAITING_FOREVER);
		index++;
		index %= CAN_RESPONSE_FRAME_MAX;
		
		if( can_response[index].cmd == cmd && 
		can_response[index].serial == serial && 
		can_response[index].src_addr == src_addr && 
		can_response[index].type == type)
		{
			rt_memcpy(buf, &can_response[index].buf[0], 6);
			err = RT_EOK;
		}
		
		rt_mutex_release(can_rsp_mux);
		
		if(err == RT_EOK)
		{
			break;
		}
		
		current_tick++;
		
		if(current_tick > timeout)
		{
			err = 1;
			break;
		}
	}
	
	
	return err;
	
}





rt_err_t can_device_get_rsp_frame(uint16_t cmd, uint8_t *buf, uint8_t src_addr, uint8_t serial, uint8_t type, rt_tick_t timeout)
{
	uint8_t *pcmd = RT_NULL;
	uint32_t dp = 0;
	rt_err_t err = 1;
	//uint8_t times[3] = {0};
	//uint16_t ms = {0};
	uint8_t rsp_buf[6] = {0};
	
	rt_tick_t tick = 0;
	
	err = get_can_response(buf, cmd, serial, src_addr, type, timeout);

	pcmd = (uint8_t*)&cmd;
	if(err == RT_EOK)
	{
		LOG_D("s:%d: t:%d [%02x << %02x] %02x %02x %02x %02x %02x %02x %02x %02x\n",  
				serial, type, CAN_LOCAL_ADDR, src_addr, pcmd[0], pcmd[1], 
				rsp_buf[0], rsp_buf[1], rsp_buf[2], rsp_buf[3], rsp_buf[4], rsp_buf[5]);
	}
	
	return err;


}




static struct rt_thread can_recv_handle;
void can_recv_thread(void *args)
{
	uint32_t dp = 0;
	struct rt_can_msg msg = {0};
	uint8_t buf[6] = {0};
	local_pack_t pack = {0};
	local_pack_t *rep_pack = RT_NULL;
	pack.buf = buf;
	
	while(1)
	{
		if(can_dev_get_msg(&msg) == RT_EOK)
		{

			switch(GET_CAN_PACK_TYPE(msg.id))
			{
				#if 1
				case PACK_TYPE_ACK:
				{
					rt_memset(&pack, 0, sizeof(local_pack_t));
					pack.len = msg.len;
					pack.serial = GET_CAN_PACK_serial(msg.id);
					pack.type = GET_CAN_PACK_TYPE(msg.id);
					pack.src_addr = GET_CAN_PACK_saddr(msg.id);
					pack.dest_addr = CAN_LOCAL_ADDR;
					pack.cmd = *((uint16_t*)&msg.data[0]);
					rt_memcpy(pack.buf, &msg.data[2], pack.len);
					set_can_response(&pack);

					break;
				}
				#endif
				
				case PACK_TYPE_RESULT:
				case PACK_TYPE_DATA:
				{
					rt_memset(&pack, 0, sizeof(local_pack_t));
					pack.len = msg.len;
					pack.serial = GET_CAN_PACK_serial(msg.id);
					pack.type = GET_CAN_PACK_TYPE(msg.id);
					pack.src_addr = GET_CAN_PACK_saddr(msg.id);
					pack.dest_addr = CAN_LOCAL_ADDR;
					pack.cmd = *((uint16_t*)&msg.data[0]);
					rt_memcpy(pack.buf, &msg.data[2], pack.len);
					set_can_response(&pack);

					break;
				}
				
				
				case PACK_TYPE_REPORT:
				{
					rep_pack = rt_malloc(sizeof(local_pack_t));
					rep_pack->buf = rt_malloc(6);
					rt_memset(rep_pack->buf, 0, 6);
				
					rep_pack->len = msg.len;
					rep_pack->serial = GET_CAN_PACK_serial(msg.id);
					rep_pack->type = GET_CAN_PACK_TYPE(msg.id);
					rep_pack->src_addr = GET_CAN_PACK_saddr(msg.id);
					rep_pack->dest_addr = CAN_LOCAL_ADDR;
					rep_pack->cmd = *((uint16_t*)&msg.data[0]);
					rt_memcpy(rep_pack->buf, &msg.data[2], rep_pack->len);
					
					dp = (uint32_t)rep_pack;
					can_device_send_ack(rep_pack->cmd, rep_pack->buf, rep_pack->dest_addr, rep_pack->src_addr, rep_pack->serial, PACK_TYPE_ACK, 6);
					rt_mq_send(can_report_queue, &dp, sizeof(void*));	
					
					break;
				}
					
			}
			
			
		}
		
		rt_thread_delay(1);
	}
	
}




void create_can_recv_thread(void)
{
	rt_err_t err;

	err = rt_thread_init(&can_recv_handle, "can_recv", 
											 can_recv_thread, 
											 RT_NULL,
											 (void *)CAN_RECV_THREAD_STACK_BASE,
											 CAN_RECV_THREAD_STACK_SIZE,
											 CAN_RECV_THREAD_PRIORITY, 
											 CAN_RECV_THREAD_TIME_SLINCE);
	if (err == RT_EOK)
	{
		rt_kprintf("can_recv_thread done!\n");
		rt_thread_startup(&can_recv_handle);
	}


}


