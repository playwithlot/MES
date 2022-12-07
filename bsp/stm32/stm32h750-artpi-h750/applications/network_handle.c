#include "network_handle.h"

#define LOG_TAG "network"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

//tcp resource
uint16_t network_pack_serial = 0;
static int now_connected_socket = -1;
static struct rt_thread tcp_recv_handle;
static struct rt_thread tcp_send_handle;
static uint8_t network_recv_buf[TCP_PACK_MAX_LEN];
static uint8_t network_frame[TCP_PACK_MAX_LEN];
static uint8_t network_send_buf[TCP_PACK_MAX_LEN];


//internal api
static void tcp_recv_thread(void *args);
static void tcp_send_thread(void *args);
void network_recv_pack_handle(uint8_t *buf);
static void tcp_send_network_pack(network_pack_t *pack);
void network_recv_pack_handle2(uint8_t *buf, uint16_t len);

//void network_get_frame(uint8_t *rbuf, uint16_t rbuf_len, uint8_t *frame, uint16_t frame_len);
uint8_t network_get_frame(uint8_t *rbuf, uint16_t rbuf_len, uint8_t *frame, uint16_t *frame_len);
void network_clean_stream(void);

rt_mutex_t net_rsp_mux = RT_NULL; 

uint16_t cal_network_pack_crc(network_pack_t *pack);
uint16_t cal_network_pack_crc(network_pack_t *pack)
{
	uint8_t *buf = RT_NULL;
	uint8_t header_size = 0;
	uint16_t pack_size = 0;
	uint16_t crc = 0;
	
	pack_size = pack->len + NET_PACK_HEAD_LEN - sizeof(pack->tail);
	header_size = NET_PACK_HEAD_LEN - sizeof(pack->head);
	
	buf = rt_malloc(pack_size);
	rt_memcpy(buf, &pack->len, header_size);
	rt_memcpy(&buf[header_size], pack->buf, pack->len);
	crc = CRC16_modbus(buf, pack_size);
	rt_free(buf);
	
	return crc;
	
}

void create_tcp_recv_thread(void)
{
		rt_err_t err;
	
    err = rt_thread_init(&tcp_recv_handle, "tcp_recv", 
                         tcp_recv_thread, 
                         RT_NULL,
                         (void *)TCP_RECV_THREAD_STACK_BASE,
                         TCP_RECV_THREAD_STACK_SIZE,
                         TCP_RECV_THREAD_PRIORITY, 
						 TCP_RECV_THREAD_TIME_SLINCE);
						 
    if (err == RT_EOK)
    {
		LOG_D("tcp_recv_thread done!\n");
        rt_thread_startup(&tcp_recv_handle);
    }
		
}


void create_tcp_send_thread(void)
{
		rt_err_t err;
	
    err = rt_thread_init(&tcp_send_handle, "tcp_send", 
                         tcp_send_thread, 
                         RT_NULL,
                         (void *)TCP_SEND_THREAD_STACK_BASE,
                         TCP_SEND_THREAD_STACK_SIZE,
                         TCP_SEND_THREAD_PRIORITY, 
						 TCP_SEND_THREAD_TIME_SLINCE);
						 
    if (err == RT_EOK)
    {
		LOG_D("tcp_send_thread done!\n");
        rt_thread_startup(&tcp_send_handle);
    }
		
}

static uint16_t network_frame_base = 0;
static void network_clean_stream(void)
{
	network_frame_base = 0;
}

uint8_t network_get_frame(uint8_t *rbuf, uint16_t rbuf_len, uint8_t *frame, uint16_t *frame_len)
{
	uint8_t res = 0;
	
	uint32_t *head = 0;
	uint32_t *tail = 0;
	uint16_t *buf_len = 0;
	uint16_t i = 0;
	
	for(i = network_frame_base; i < rbuf_len; i++)
	{
		head = (uint32_t*)&rbuf[i];
		if(*head == NET_PACK_HEAD)
		{
			buf_len = (uint16_t*)&rbuf[i+4];
			tail = (uint32_t*)&rbuf[17+(*buf_len) + network_frame_base];
			
			if(*tail == NET_PACK_END)
			{
				*frame_len = *buf_len + sizeof(network_pack_t) - 4;
				//LOG_E("head: %08x buf len: %d frame len: %d tail: %08x\n", *head, *buf_len, *frame_len, *tail);
				network_frame_base = (i+*frame_len);
				res = 1;
				
				rt_memcpy(&frame[0], &rbuf[i], network_frame_base);
				break;
			
			}
		}
	}
	
	return res;

}



static void tcp_recv_thread(void *args)
{
	socklen_t sin_size;
	int server_sock, bytes_received, reuse = 1;
	struct sockaddr_in server_addr, client_addr;
	uint8_t cnt = 0;
	int on;
	int new_connected_socket = -1;
	uint16_t frame_len = 0;
	uint16_t frame_num = 0;
	//network_pack_t network_cmd_pack;
	
	
	
	while(1)
	{
		//crate a sock 
		if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			LOG_D("Socket error\n");
			rt_thread_delay(Msec(10));
			continue;
		}
		
		//set tcp server param
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(8500); //服务端工作的端口 TCP_LOCAL_SERVER_PORT
		server_addr.sin_addr.s_addr = INADDR_ANY;
		rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
		setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
		setsockopt(server_sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
	
		//bing to the tcp server port 
		if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
		{
				LOG_D("Unable to bind\n");
				closesocket(server_sock);
				rt_thread_delay(Msec(200));
				continue;
		}
		
		//listening port 
		if (listen(server_sock, 1) == -1)
		{
				LOG_D("Listen error\n");
				closesocket(server_sock);
				rt_thread_delay(Msec(10));
				continue;
		}
		
		LOG_D("TCP server: Waiting for client connect...\n");
		sin_size = sizeof(struct sockaddr_in); 

		
		//accept tcp client connect 
		cnt = 0;
		while (1) //等待连接
		{
			new_connected_socket = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
			now_connected_socket = new_connected_socket;
			if(new_connected_socket >= 0)
			{
				closesocket(server_sock); //close tcp sever socket any not accept connect any more
				LOG_D("is tcp client connected: %d\n", new_connected_socket);
				break;
			}
			else
			{
				cnt++;
				
				if (cnt > 10)
				{
					 break;
				}
				
				LOG_D("accept connection failed! errno = %d\n", errno);
				rt_thread_delay(Msec(10));
				
				continue;
			}

		}

		

		LOG_D("local tcp server got a connection from (%s , %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		//set tcp application level param: ack no delay
		on = 1;
		setsockopt(new_connected_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int));
		struct timeval timeout = {180, 0};
		setsockopt(new_connected_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
		
		//recv tcp client data
		while(1)
		{
			 bytes_received = recv(new_connected_socket, &network_recv_buf[0], TCP_PACK_MAX_LEN, 0);
			 network_clean_stream();
			 if(bytes_received == 0) //tcp client disconnected
			 {
					
					break;
			 }
			 else if(bytes_received < 0) //recv data form tcp client
			 {
					LOG_E("connect error: %d\n", errno);
					break;
			 }
			 
			 #if 0
			 LOG_RAW("net pack: ");
			
			 for(uint16_t i = 0; i < bytes_received; i++)
			 {
				LOG_RAW("%02x ", network_recv_buf[i]);
			 }
			 
			 LOG_RAW("\n\n");
			 #endif
			 
			 LOG_RAW("\n");
			 LOG_HEX("net pack", bytes_received, network_recv_buf, bytes_received);
			 
			 while(1)
			 {
				if(network_get_frame(network_recv_buf, bytes_received, network_frame, &frame_len))
				{
					frame_num++;
					LOG_I("network recv %d frame: ", frame_num);
					
					#if 0
					for(uint8_t i = 0; i < frame_len; i++)
					{
						LOG_RAW("%02x ", network_frame[i]);
					}
					LOG_RAW("\n\n");
					
					#endif
					
					LOG_RAW("\n");
					LOG_HEX("net pack", frame_len, network_frame, frame_len);
					
					//network_recv_pack_handle(network_frame);
					network_recv_pack_handle2(network_frame, frame_len);
				}
				else
				{
					break;
				}
				
			 }
			 
			if(bytes_received != network_frame_base)
			{
				LOG_E("recv an illegal cmd frame! %d == %d\n", bytes_received, network_frame_base);
			}

			rt_thread_delay(1);  //give up time slice

		}
		
		LOG_E("netwrok disconnet error: %d\n", errno); 
		closesocket(new_connected_socket);
		new_connected_socket = -1;
		now_connected_socket = new_connected_socket;
		
	
	}
	
}








#define NET_RESPONSE_FRAME_MAX (100)
static uint8_t net_response_index = 0;
static network_rsp_t net_response[NET_RESPONSE_FRAME_MAX] = {0};

void set_net_response(local_pack_t *pack)
{
	rt_mutex_take(net_rsp_mux, RT_WAITING_FOREVER);
	net_response_index++;
	net_response_index %= NET_RESPONSE_FRAME_MAX;
	
	net_response[net_response_index].cmd = pack->cmd;
	net_response[net_response_index].serial = pack->serial;
	net_response[net_response_index].src_addr = pack->src_addr;
	net_response[net_response_index].dest_addr = pack->dest_addr;
	net_response[net_response_index].type = pack->type;
	

	LOG_I("set wait ack info: cmd: %04x serail:%04x src:%02x des:%02x types:%02x\n", 
	pack->cmd, pack->serial, pack->src_addr, pack->dest_addr, net_response[net_response_index].type);
	
	rt_mutex_release(net_rsp_mux);

}


void clen_net_response(void)
{
	rt_mutex_take(net_rsp_mux, RT_WAITING_FOREVER);
	for(uint8_t i = 0; i < NET_RESPONSE_FRAME_MAX; i++)
	{
		rt_memset(&net_response[i], 0, sizeof(network_rsp_t)); 
	}
	
	rt_mutex_release(net_rsp_mux);
}


uint8_t get_net_response(network_pack_t *pack, uint8_t type, rt_tick_t timeout)
{
	uint8_t net_rsp_index = 0;
	rt_err_t err = 1;    //犯了低级错误
	rt_tick_t current_tick = 0;
	
	LOG_I("get wait ack info: cmd: %04x serail:%04x src:%02x des:%02x types:%02x\n", 
	pack->cmd, pack->serial, pack->src_addr, pack->dest_addr, type);
	
	while(1)
	{
		rt_thread_delay(1);
		rt_mutex_take(net_rsp_mux, RT_WAITING_FOREVER);
		net_rsp_index++;
		net_rsp_index %= NET_RESPONSE_FRAME_MAX;
		
		if( net_response[net_rsp_index].cmd == pack->cmd && 
			net_response[net_rsp_index].serial == pack->serial && 
			//net_response[net_rsp_index].src_addr == pack->dest_addr && 
			//net_response[net_rsp_index].dest_addr == pack->src_addr && 
			net_response[net_rsp_index].type == type)
		{
			rt_memset(&net_response[net_rsp_index], 0, sizeof(network_rsp_t));
			err = RT_EOK;
		}
		
		rt_mutex_release(net_rsp_mux);
		
		if(err == RT_EOK)
		{
			LOG_I("wait ack frame sucessful: cmd: %04x serail:%04x src:%02x des:%02x types:%02x\n", 
			pack->cmd, pack->serial, pack->src_addr, pack->dest_addr, type);
			break;
		}
		
		current_tick++;
		if(current_tick > timeout)
		{
			LOG_E("wait ack frame fail: cmd: %04x serail:%04x src:%02x des:%02x types:%02x\n", 
			pack->cmd, pack->serial, pack->src_addr, pack->dest_addr, type);
			
			for(uint8_t i = 0; i < NET_RESPONSE_FRAME_MAX; i++)
			{
				LOG_I("rsp tab: cmd: %04x serail:%04x src:%02x des:%02x types:%02x\n", 
				net_response[i].cmd, net_response[i].serial, net_response[i].src_addr, net_response[i].dest_addr, net_response[i].type);
			}
			err = 1;
			break;
		}
	}
	
	
	
	return err;
	
}


void network_recv_pack_handle(uint8_t *buf)
{
		uint32_t err = 0;
		uint8_t timestamp[3] = {0};
		uint16_t ms = 0;
		uint16_t crc = 0;
		uint32_t dp = 0;
		network_pack_t pack = {0};
		local_pack_t *local_pack = RT_NULL;

		//network_cmd_pack = rt_malloc(sizeof(network_pack_t));
		rt_memcpy(&pack, buf, NET_PACK_HEAD_LEN);

		if(pack.head == NET_PACK_HEAD && pack.len < TCP_PACK_MAX_LEN) //check net cmd header data and len field
		{
			//network_cmd_pack.buf = rt_malloc(network_cmd_pack.len);
			//rt_memcpy(network_cmd_pack.buf, &network_recv_buf[NET_DEFAULT_HEAD_LEN], network_cmd_pack->len);
			pack.buf = &buf[NET_PACK_HEAD_LEN];
		}

		rt_memcpy(&pack.crc, &buf[NET_PACK_HEAD_LEN + pack.len], 6);  //get crc and tail


		//check pack tail  
		
		if(pack.tail == NET_PACK_END)
		{
			#if 0 
			get_timestamp(&timestamp[0], &timestamp[1], &timestamp[2], &ms);
			LOG_RAW("%02d:%02d:%02d:%03d %08x %04x %04x %02x %02x %02x %08x ", &timestamp[0], &timestamp[1], &timestamp[2], ms, pack.head, pack.len, 
																pack.serial, pack.type,
																pack.src_addr, pack.dest_addr, pack.cmd);
			
			for(uint8_t i = 0; i < pack.len; i++)
			{
				LOG_RAW("%02x ", pack.buf[i]);
			}
			LOG_RAW("%04x %08x \n", pack.crc, pack.tail);
			#endif
			
			
			//check crc
			crc = cal_network_pack_crc(&pack); //CRC16_modbus(&network_recv_buf[4], network_cmd_pack.len + NET_PACK_HEAD_LEN - sizeof(network_cmd_pack.tail));
			//crc = CRC16_modbus(&buf[4], pack.len + 11);
			if(pack.crc == crc)
			{
				//set_net_response(&pack);
				local_pack = rt_malloc(sizeof(local_pack_t));
				if(local_pack)
				{
					if(local_pack->type != PACK_TYPE_ACK)
					{
						local_transmit_response(local_pack, (uint8_t*)&err, 4, PACK_TYPE_ACK);
					}
					
					local_pack->buf = rt_malloc(pack.len);
					rt_memcpy(local_pack, &pack.len, NET_PACK_HEAD_LEN - 4);
					rt_memcpy(local_pack->buf, pack.buf, pack.len);
					dp = (uint32_t)local_pack;
					rt_mq_send(network_command_queue, &dp, sizeof(void*));
				}
				else
				{
					if(local_pack->buf)
					{
						rt_free(local_pack->buf);
					}
					
					if(local_pack)
					{
						rt_free(local_pack);
					}
				}
				
				#if 0 
				LOG_RAW("raw: ");
				for(uint8_t i = 0; i < sizeof(network_pack_t) + local_pack->len - 4; i++)
				{
					LOG_RAW("%02x ", buf[i]);
				}
				LOG_RAW("\n\n"); 
				#endif

			}
			else
			{
				LOG_E("net pack crc err, cal(h):%x, now(h):%x\n", crc, pack.crc);
			}
			
		}
		else
		{
			LOG_E("recv an illegal cmd frame!\n");
		}
		 
	
}





void network_recv_pack_handle2(uint8_t *buf, uint16_t len)
{
		uint32_t err = 0;
		uint8_t timestamp[3] = {0};
		uint16_t ms = 0;
		uint16_t crc = 0;
		uint32_t dp = 0;
		network_pack_t pack = {0};
		local_pack_t *local_pack = RT_NULL;

		//network_cmd_pack = rt_malloc(sizeof(network_pack_t));
		rt_memcpy(&pack, buf, NET_PACK_HEAD_LEN);
		
		if(pack.head == NET_PACK_HEAD && pack.len < TCP_PACK_MAX_LEN) //check net cmd header data and len field
		{
			//network_cmd_pack.buf = rt_malloc(network_cmd_pack.len);
			//rt_memcpy(network_cmd_pack.buf, &network_recv_buf[NET_DEFAULT_HEAD_LEN], network_cmd_pack->len);
			pack.buf = &buf[NET_PACK_HEAD_LEN];
		}

		rt_memcpy(&pack.crc, &buf[NET_PACK_HEAD_LEN + pack.len], 6);  //get crc and tail


		//check pack tail  
		
		if(pack.tail == NET_PACK_END)
		{
			#if 0 
			get_timestamp(&timestamp[0], &timestamp[1], &timestamp[2], &ms);
			LOG_RAW("%02d:%02d:%02d:%03d %08x %04x %04x %02x %02x %02x %08x ", &timestamp[0], &timestamp[1], &timestamp[2], ms, pack.head, pack.len, 
																pack.serial, pack.type,
																pack.src_addr, pack.dest_addr, pack.cmd);
			
			for(uint8_t i = 0; i < pack.len; i++)
			{
				LOG_RAW("%02x ", pack.buf[i]);
			}
			LOG_RAW("%04x %08x \n", pack.crc, pack.tail);
			#endif

			//check crc
			crc = cal_network_pack_crc(&pack); //CRC16_modbus(&network_recv_buf[4], network_cmd_pack.len + NET_PACK_HEAD_LEN - sizeof(network_cmd_pack.tail));
			//crc = CRC16_modbus(&buf[4], pack.len + 11);
			if(pack.crc == crc)
			{
				
				local_pack = rt_malloc(sizeof(local_pack_t));
				if(local_pack)
				{
					if(local_pack->type == PACK_TYPE_ACK)
					{
						LOG_I("recv ack pack: cmd: %04x serail:%04x src:%02x des:%02x types:%02x\n", 
						local_pack->cmd, local_pack->serial, local_pack->src_addr, local_pack->dest_addr, local_pack->type); 
						set_net_response(local_pack);
						
					}
					
					
					if( (local_pack->type != PACK_TYPE_ACK) && (local_pack->type != PACK_TYPE_HEARTBEAT) )
					{
						local_transmit_response(local_pack, (uint8_t*)&err, 4, PACK_TYPE_ACK);
					}
					

					
					local_pack->buf = rt_malloc(pack.len);
					rt_memcpy(local_pack, &pack.len, NET_PACK_HEAD_LEN - 4);
					rt_memcpy(local_pack->buf, pack.buf, pack.len);
					dp = (uint32_t)local_pack;
					rt_mq_send(network_command_queue, &dp, sizeof(void*));
				}
				else
				{
					if(local_pack->buf)
					{
						rt_free(local_pack->buf);
					}
					
					if(local_pack)
					{
						rt_free(local_pack);
					}
				}
				
				#if 0 
				LOG_RAW("raw: ");
				for(uint8_t i = 0; i < sizeof(network_pack_t) + local_pack->len - 4; i++)
				{
					LOG_RAW("%02x ", buf[i]);
				}
				LOG_RAW("\n\n"); 
				#endif

			}
			else
			{
				LOG_E("net pack crc err, cal(h):%x, now(h):%x\n", crc, pack.crc);
			}
			
		}
		else
		{
			LOG_E("recv an illegal cmd frame!\n");
		}
		 
	
}


static void tcp_send_thread(void *args)
{
	
	uint32_t dp = 0;
	local_pack_t *local_pack = RT_NULL;
	network_pack_t network_pack = {0};

	
	
	while(1)
	{
		rt_mq_recv(local_rsp_queue, &dp, sizeof(void*), RT_WAITING_FOREVER);
		local_pack = (local_pack_t*)(dp);
		
		//set the network pack
		network_pack.len = local_pack->len;
		network_pack.serial = local_pack->serial;
		network_pack.type = local_pack->type;
		network_pack.src_addr = local_pack->src_addr;
		network_pack.dest_addr = local_pack->dest_addr;
		network_pack.head = NET_PACK_HEAD;
		network_pack.tail = NET_PACK_END;
		network_pack.cmd = local_pack->cmd;
		network_pack.buf = local_pack->buf;
		network_pack.crc = cal_network_pack_crc(&network_pack);
		
		
		tcp_send_network_pack(&network_pack);
		
		
		if(local_pack->buf)
		{
			rt_free(local_pack->buf);
			local_pack->buf = RT_NULL;
		}
		
		if(local_pack)
		{
			rt_free(local_pack);
			local_pack = RT_NULL;
		}
		
		
		
		rt_thread_delay(1);
	}
	
}



static void tcp_send_network_pack(network_pack_t *pack)
{
	uint8_t re_send_cnt = 0;
	uint16_t pack_size = 0;
	pack_size = pack->len + NET_PACK_HEAD_LEN + 6;
	
	if(now_connected_socket >= 0)
	{
		rt_memcpy(network_send_buf, pack, NET_PACK_HEAD_LEN);
		rt_memcpy(&network_send_buf[NET_PACK_HEAD_LEN], pack->buf, pack->len);
		rt_memcpy(&network_send_buf[NET_PACK_HEAD_LEN + pack->len], &pack->crc, 6); 
		
		#if 0
		LOG_RAW("rsp pack: ");
		for(uint8_t i = 0; i < pack_size; i++)
		{
			LOG_RAW("%02x ", network_send_buf[i]);
		}
		LOG_RAW("\n");
		LOG_RAW("\n");
		#endif
		
		LOG_RAW("\n");
		LOG_HEX("rsp pack", pack_size, network_send_buf, pack_size);
		
		if(pack->type == PACK_TYPE_DATA)   //if date frame wait for ack 
		{
			re_send_cnt = 0;
			while(re_send_cnt < 3)
			{
				re_send_cnt++;
				send(now_connected_socket, network_send_buf, pack_size, 0);
				if(get_net_response(pack, PACK_TYPE_ACK, NETWORK_WAIT_ACK_TIME) == RT_EOK)
				{	
					LOG_I("get data frame %d ack\n", pack->serial);
					break;
				}
			}
			
		}
		else if(pack->type == PACK_TYPE_RESULT)
		{
			re_send_cnt = 0; 
			while(re_send_cnt < 3)
			{
				re_send_cnt++;
				send(now_connected_socket, network_send_buf, pack_size, 0);
				if(get_net_response(pack, PACK_TYPE_ACK, NETWORK_WAIT_ACK_TIME) == RT_EOK)
				{	
					LOG_I("get result frame %d ack\n", pack->serial);
					break;
				}
			}
						
		}
		else if(pack->type == PACK_TYPE_REPORT)
		{
			re_send_cnt = 0;
			while(re_send_cnt < 3)
			{
				re_send_cnt++;
				send(now_connected_socket, network_send_buf, pack_size, 0);
				if(get_net_response(pack, PACK_TYPE_ACK, NETWORK_WAIT_ACK_TIME) == RT_EOK)
				{	
					LOG_I("get report frame %d ack\n", pack->serial);
					break;
				}
			}
		
		}
		else if(pack->type == PACK_TYPE_ACK)
		{
			send(now_connected_socket, network_send_buf, pack_size, 0);
		}
		else if(pack->type == PACK_TYPE_HEARTBEAT)
		{
			send(now_connected_socket, network_send_buf, pack_size, 0);
		}
		else 
		{
			send(now_connected_socket, network_send_buf, pack_size, 0);
		}
			
		
	}

}









