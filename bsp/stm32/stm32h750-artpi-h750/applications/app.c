#include "app.h"
		
//#define NET_CMD_RD_VERSION               (0x00000101)
// a5 a1 1a 5a 00 00 0a 00 06 fe fe 00 00 00 00 22 4e 5a 1a a1 a5

#define LOG_TAG "app"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>
		
static void msg_queue_clean(rt_mq_t mq)
{
	uint32_t dp = 0;
	while(rt_mq_recv(mq, &dp, sizeof(void*), 0) == RT_EOK);
}	


extern void dev_led_init(void);
extern void act_dev_status_reset(void);
void app_initailze(void)
{
	network_command_queue = rt_mq_create("network_command_queue", sizeof(void*), 20, RT_IPC_FLAG_FIFO);
	if(network_command_queue != RT_NULL)
	{
		msg_queue_clean(network_command_queue);
		LOG_D("create network_command_queue sucessful!\n");
	
	}
	
	local_rsp_queue = rt_mq_create("local_rsp_queue", sizeof(void*), 20, RT_IPC_FLAG_FIFO);
	if(local_rsp_queue != RT_NULL)
	{
		msg_queue_clean(local_rsp_queue);
		LOG_D("create local_rsp_queue sucessful!\n");
	
	}
	

	can_report_queue = rt_mq_create("can_report_queue", sizeof(void*), 20, RT_IPC_FLAG_FIFO);
	if(can_report_queue != RT_NULL)
	{
		msg_queue_clean(can_report_queue);
		LOG_D("create can_report_queue sucessful!\n");
	
	}
	
	can_response_queue = rt_mq_create("can_response_queue", sizeof(void*), 30, RT_IPC_FLAG_FIFO);
	if(can_response_queue != RT_NULL)
	{
		msg_queue_clean(can_response_queue);
		LOG_D("create can_response_queue sucessful!\n");
	
	}
	
	can_rsp_mux = rt_mutex_create("can rsp mutex",RT_IPC_FLAG_PRIO); 
	if(can_rsp_mux != RT_NULL)
	{
		LOG_D("can_rsp_mux sucessful!\n");
	}
	
	net_rsp_mux = rt_mutex_create("net rsp mutex",RT_IPC_FLAG_PRIO); 
	if(net_rsp_mux != RT_NULL)
	{
		LOG_D("net_rsp_mux sucessful!\n");
	}
	
	multicast_rsp_mux = rt_mutex_create("multicastrsp mutex",RT_IPC_FLAG_PRIO); 
	if(multicast_rsp_mux != RT_NULL)
	{
		LOG_D("multicast_rsp_mux sucessful!\n");
	}
	

	act_dev_status_reset();
	can_device_init();
	scanner_dev_init(USER_SCANNER_TYPES);  

	create_monitor_thread();
	create_tcp_recv_thread();
	create_tcp_send_thread();
	create_can_recv_thread();
	create_local_handle_thread();
	create_multicast_handle_thread();
	

}






void get_timestamp(uint8_t *h, uint8_t *min, uint8_t *sec, uint16_t *ms)
{
	rt_tick_t t = rt_tick_get();
	
	uint32_t hms[4] = {0};
	
	hms[3] = t % 1000;
	hms[2] = t / 1000;
	hms[1] = hms[02] / 60; 
	hms[0] = hms[01] / 60; 
	
	*h = hms[0];
	*min = hms[1] % 60;
	*sec = hms[2] % 60;
	*ms = hms[3];

}


void get_timestamp_cmd(void)
{
	uint8_t h; 
	uint8_t min;
	uint8_t sec;
	uint16_t ms;
	
	get_timestamp(&h, &min, &sec, &ms);
	
	LOG_D("%02d:%02d:%02d:%03d", h, min, sec, ms);

}

MSH_CMD_EXPORT(get_timestamp_cmd, "get_timestamp_cmd");













