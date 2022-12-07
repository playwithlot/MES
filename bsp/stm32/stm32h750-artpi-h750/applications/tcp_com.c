#include "tcp_com.h"


static int now_connected_socket = -1;
static struct rt_thread tcp_recv_handle;
static struct rt_thread tcp_send_handle;
static void tcp_recv_thread(void *args);
static void tcp_send_thread(void *args);
static uint8_t network_recv_buf[TCP_PACK_MAX_LEN];
static void network_recv_pack_handle(network_pack_t *pack);
static uint16_t cal_network_pack_crc(network_pack_t *pack);
static void tcp_send_network_pack(network_pack_t *pack);

static void network_cmd_pack_handle(network_pack_t *pack);
static void network_ack_pack_handle(network_pack_t *pack);

//export
void create_cmd_frame_pack(uint8_t can_addr, uint32_t can_cmd, uint8_t *praram, uint8_t len);
rt_mq_t can_handle_queue = RT_NULL;
rt_mq_t network_handle_queue = RT_NULL;



static uint16_t cal_network_pack_crc(network_pack_t *pack)
{
	uint8_t *buf = RT_NULL;
	uint8_t header_size = 0;
	uint16_t pack_size = 0;
	uint16_t crc = 0;
	
	pack_size = pack->len + NET_PACK_HEAD_LEN - sizeof(pack->tail);
	header_size = NET_PACK_HEAD_LEN - sizeof(pack->head);
	
	buf = rt_malloc(pack_size);
	rt_memcpy(buf, &pack->len, header_size);
	rt_memcpy(&buf[header_size], &pack->buf, pack->len);
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
        rt_thread_startup(&tcp_recv_handle);
    }
		
}




static void tcp_recv_thread(void *args)
{
	socklen_t sin_size;
	int server_sock, bytes_received, reuse = 1;
	struct sockaddr_in server_addr, client_addr;
	uint8_t cnt = 0;

	uint16_t crc;
	int on;
	int new_connected_socket = -1;
	network_pack_t network_cmd_pack;
	
	
	
	while(1)
	{
		//crate a sock 
		if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			rt_kprintf("Socket error\n");
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
				rt_kprintf("Unable to bind\n");
				closesocket(server_sock);
				rt_thread_delay(Msec(200));
				continue;
		}
		
		//listening port 
		if (listen(server_sock, 1) == -1)
		{
				rt_kprintf("Listen error\n");
				closesocket(server_sock);
				rt_thread_delay(Msec(10));
				continue;
		}
		
		rt_kprintf("TCP server: Waiting for client connect...\n");
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
				rt_kprintf("is tcp client connected: %d\n", new_connected_socket);
				break;
			}
			else
			{
				cnt++;
				
				if (cnt > 10)
				{
					 break;
				}
				
				rt_kprintf("accept connection failed! errno = %d\n", errno);
				rt_thread_delay(Msec(10));
				
				continue;
			}

		}

		

		rt_kprintf("local tcp server got a connection from (%s , %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		//set tcp application level param: ack no delay
		on = 1;
		setsockopt(new_connected_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(int));
		struct timeval timeout = {65, 0};
		setsockopt(new_connected_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
		
		//recv tcp client data
		while(1)
		{
			 bytes_received = recv(new_connected_socket, &network_recv_buf[0], TCP_PACK_MAX_LEN, 0);
			 if(bytes_received == 0) //tcp client disconnected
			 {
					break;
			 }
			 else if(bytes_received < 0) //recv data form tcp client
			 {
					rt_kprintf("connect error: %d\n", errno);
					break;
			 }
			 
			 //network_cmd_pack = rt_malloc(sizeof(network_pack_t));
			 rt_memcpy(&network_cmd_pack, network_recv_buf, NET_PACK_HEAD_LEN);

			 if(network_cmd_pack.head == NET_PACK_HEAD && network_cmd_pack.len < TCP_PACK_MAX_LEN) //check net cmd header data and len field
			 {
					//network_cmd_pack.buf = rt_malloc(network_cmd_pack.len);
				  //rt_memcpy(network_cmd_pack.buf, &network_recv_buf[NET_DEFAULT_HEAD_LEN], network_cmd_pack->len);
					network_cmd_pack.buf = &network_recv_buf[NET_PACK_HEAD_LEN];
			 }
			 
			 rt_memcpy(&network_cmd_pack.crc, &network_recv_buf[NET_PACK_HEAD_LEN + network_cmd_pack.len], 6);  //get crc and tail
			 
			 
			 //check pack tail  
			 if(network_cmd_pack.tail == NET_PACK_END)
			 {
					rt_kprintf("%08x %04x %04x %02x %02x %02x %08x ", network_cmd_pack.head, network_cmd_pack.len, 
																													 network_cmd_pack.serial, network_cmd_pack.type,
																													 network_cmd_pack.src_addr, network_cmd_pack.dest_addr, network_cmd_pack.cmd);
				 
					for(uint8_t i = 0; i < network_cmd_pack.len; i++)
					{
						rt_kprintf("%02x ", network_cmd_pack.buf[i]);
					}
					rt_kprintf("%04x %08x \n", network_cmd_pack.crc, network_cmd_pack.tail);
					
					//check crc
					crc = cal_network_pack_crc(&network_cmd_pack); //CRC16_modbus(&network_recv_buf[4], network_cmd_pack.len + NET_PACK_HEAD_LEN - sizeof(network_cmd_pack.tail));
					if(network_cmd_pack.crc == crc)
					{
						//rt_mq_send(can_handle_queue, cmd_pack, sizeof(network_pack_t));
						network_recv_pack_handle(&network_cmd_pack);
					}
					else
					{
						rt_kprintf("net pack crc err, cal(h):%x, now(h):%x\n", crc, network_cmd_pack.crc);
					}
					
			 }
			 else
			 {
				 rt_kprintf("tcp: recv an illegal cmd frame!\n");
			 }


			 
			 rt_thread_delay(1);  //give up time slice
			 //network_pack_t
			 
	
			
		}
		
		closesocket(new_connected_socket);
		new_connected_socket = -1;
		now_connected_socket = new_connected_socket;
		
	
	}
	
}


static void tcp_send_thread(void *args)
{
	uint32_t dp = 0;
	network_pack_t *network_pack = RT_NULL;
	
	while(1)
	{
		rt_mq_recv(network_handle_queue, &dp, sizeof(network_pack_t), RT_WAITING_FOREVER);
		network_pack = (network_pack_t*)(dp);
		tcp_send_network_pack(network_pack);
		
		if(network_pack->buf)
		{
			rt_free(network_pack->buf);
		}
		
		rt_free(network_pack);
		rt_thread_delay(10);
	}
	
}

static void tcp_send_network_pack(network_pack_t *pack)
{
	uint8_t *buf = RT_NULL;
	uint16_t pack_size = 0;

	pack_size = pack->len + NET_PACK_HEAD_LEN + 6;
	
	if(now_connected_socket >= 0)
	{
		buf = rt_malloc(pack_size);
		rt_memcpy(buf, &pack, NET_PACK_HEAD_LEN);
		rt_memcpy(&buf[NET_PACK_HEAD_LEN], &pack->buf, pack->len);
		rt_memcpy(&buf[NET_PACK_HEAD_LEN + pack->len], &pack->crc, 6);
		send(now_connected_socket, pack, sizeof(network_pack_t), 0);
		
		if(buf)
		{
			rt_free(buf);
		}
		
	}

}

static void network_recv_pack_handle(network_pack_t *pack)
{
	switch(pack->type) //referen cmd list
	{
		case NET_PACK_TYPE_CMD:
			rt_kprintf("recv upper machine cmd frame!\n");
			network_cmd_pack_handle(pack); 
		break;
		
		case NET_PACK_TYPE_ACK:  //handle the upper machine ack frame here
			rt_kprintf("recv upper machine ack frame!\n");
			network_ack_pack_handle(pack); 
		break;
		
		//the following frame will not recv forever
		case NET_PACK_TYPE_RESULT:
		case NET_PACK_TYPE_DATA:
		case NET_PACK_TYPE_REPORT:
		case NET_PACK_TYPE_HEARTBEAT:
		break;
			
		default:
			
		break;
	}

	
}






void create_network_frame(uint8_t src_addr, uint8_t dest_addr, uint16_t serial, uint8_t frame_type, uint16_t cmd, uint8_t *buf, uint16_t len)
{
	uint32_t dp = 0;
	network_pack_t *network_pack; 
	network_pack->buf = rt_malloc(len);
	
	if(network_pack != RT_NULL && network_pack->buf != RT_NULL)
	{	
		network_pack->head = NET_PACK_HEAD;
		network_pack->len = len;
		network_pack->serial = serial;
		network_pack->type = frame_type;
		network_pack->src_addr = src_addr;
		network_pack->dest_addr = dest_addr;
		network_pack->cmd = cmd;
		rt_memcpy(network_pack, buf, len);
		network_pack->crc = cal_network_pack_crc(network_pack); 
		network_pack->tail = NET_PACK_END;
		dp = (uint32_t)network_pack;
		rt_mq_send(network_handle_queue, &dp, sizeof(void*));  //network to midmachine

	}
	else
	{
		if(network_pack)
		{
			rt_free(network_pack);
		}
		
		if(network_pack->buf)
		{
			rt_free(network_pack->buf);
		}
		
	}

}


//network to can
static void network_cmd_pack_handle(network_pack_t *pack)
{
	switch(pack->cmd)
	{
		case NET_CMD_temp_get_version:
			
		break;
		
		case NET_CMD_temp_get_mt_para:
			
		break;
		
		case NET_CMD_temp_set_position_para:
			
		break;
		
		case NET_CMD_temp_get_position_para:
			
		break;
		
		default:
			create_can_frame(pack->src_addr, pack->dest_addr, pack->serial, pack->type, pack->cmd, pack->buf, pack->len);
		break;
	
	}


}



//network rsp
static void network_ack_pack_handle(network_pack_t *pack)
{
	create_network_frame(pack->dest_addr, pack->src_addr, pack->serial, pack->type, pack->cmd, RT_NULL, 0);
	

}




